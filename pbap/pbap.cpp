/*
 * Copyright (C) 2018 Konsulko Group
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <QMetaEnum>
#include <QtQml/QQmlEngine>

#include <vcard/vcard.h>

#include "message.h"
#include "messageengine.h"
#include "pbap.h"
#include "pbapmessage.h"
#include "responsemessage.h"

PhoneNumber::PhoneNumber(QString number, QString type)
{
    m_number = number;
    m_type = stringToEnum(type);
}

PhoneNumber::~PhoneNumber()
{
}

int PhoneNumber::stringToEnum(QString key)
{
    const QMetaObject* metaObject = PhoneNumber::metaObject();
    int enumIndex = metaObject->indexOfEnumerator("PhoneNumberType");
    QMetaEnum mEnum = metaObject->enumerator(enumIndex);

    int value = mEnum.keyToValue(key.toUtf8().data());
    return (value < 0) ? 0 : value;
}

Contact::Contact(QString name, QList<PhoneNumber *>numbers)
{
    m_name = name;
    m_numbers = numbers;
}

Contact::~Contact()
{
}

RecentCall::RecentCall(QString name, QString number, QString datetime, QString type)
{
    m_name = name;
    m_number = number;
    m_datetime = datetime;
    m_type = stringToEnum(type);
}

RecentCall::~RecentCall()
{
}

int RecentCall::stringToEnum(QString key)
{
    const QMetaObject* metaObject = RecentCall::metaObject();
    int enumIndex = metaObject->indexOfEnumerator("RecentCallType");
    QMetaEnum mEnum = metaObject->enumerator(enumIndex);

    int value = mEnum.keyToValue(key.toUtf8().data());
    return (value < 0) ? 0 : value;
}

Pbap::Pbap (QUrl &url, QQmlContext *context, QObject * parent) :
    QObject(parent),
    m_mloop(nullptr)
{
    m_mloop = new MessageEngine(url);
    m_context = context;
    m_context->setContextProperty("ContactsModel", QVariant::fromValue(m_contacts));
    qmlRegisterUncreatableType<PhoneNumber>("PhoneNumber", 1, 0, "PhoneNumber", "Enum");
    m_context->setContextProperty("RecentCallModel", QVariant::fromValue(m_calls));
    qmlRegisterUncreatableType<RecentCall>("RecentCall", 1, 0, "RecentCall", "Enum");

    QObject::connect(m_mloop, &MessageEngine::connected, this, &Pbap::onConnected);
    QObject::connect(m_mloop, &MessageEngine::disconnected, this, &Pbap::onDisconnected);
    QObject::connect(m_mloop, &MessageEngine::messageReceived, this, &Pbap::onMessageReceived);
}

Pbap::~Pbap()
{
    delete m_mloop;
}

void Pbap::refreshContacts(int max_entries)
{
    PbapMessage *tmsg = new PbapMessage();
    QJsonObject parameter;

    if (max_entries >= 0)
        parameter.insert("max_entries", max_entries);

    tmsg->createRequest("contacts", parameter);
    m_mloop->sendMessage(tmsg);
    tmsg->deleteLater();
}

void Pbap::refreshCalls(int max_entries)
{
    PbapMessage *tmsg = new PbapMessage();
    QJsonObject parameter;

    parameter.insert("list", "cch");
    if (max_entries >= 0)
        parameter.insert("max_entries", max_entries);

    tmsg->createRequest("history", parameter);
    m_mloop->sendMessage(tmsg);
    tmsg->deleteLater();
}

void Pbap::search(QString number)
{
    PbapMessage *tmsg = new PbapMessage();
    QJsonObject parameter;

    if (!number.isEmpty())
        parameter.insert("number", number);
    parameter.insert("max_entries", 1);

    tmsg->createRequest("search", parameter);
    m_mloop->sendMessage(tmsg);
    tmsg->deleteLater();
}

bool compareContactPtr(QObject *a, QObject *b)
{
    Contact *contactA = qobject_cast<Contact *>(a);
    Contact *contactB = qobject_cast<Contact *>(b);

    return (*contactA < *contactB);
}

void Pbap::updateContacts(QString vcards)
{
    QString name, number, type;

    m_contacts.clear();

    QList<vCard> contacts_vcards = vCard::fromByteArray(vcards.toUtf8());

    for (auto vcard : contacts_vcards) {
        vCardProperty name_prop = vcard.property(VC_FORMATTED_NAME);
        QStringList values = name_prop.values();
        name = values.at(vCardProperty::DefaultValue);
        if (name.isEmpty() || name.startsWith('#'))
            continue;
        /*
         * libvcard has no member function to return a list of named
         * properties, so we iterate over all properties and parse
         * each identified VC_TELEPHONE property in the vCard.
         */
        QList<PhoneNumber *> numbers;
        vCardPropertyList properties = vcard.properties();
        for (auto property : properties) {
            if (property.isValid() && (property.name() == VC_TELEPHONE)) {
                QStringList values = property.values();
                number = values.at(0);
                vCardParamList params = property.params();
                // The first parameter is always the phone number type
                type = params.at(0).value();
                numbers.append(new PhoneNumber(number, type));
            }
        }
        m_contacts.append(new Contact(name, numbers));
    }

    std::sort(m_contacts.begin(), m_contacts.end(), compareContactPtr);

    // Refresh model
    m_context->setContextProperty("ContactsModel", QVariant::fromValue(m_contacts));

    refreshCalls(100);
}

#define VC_DATETIME "X-IRMC-CALL-DATETIME"

void Pbap::updateCalls(QString vcards)
{
    QString name, number, datetime, type;

    m_calls.clear();

    QList<vCard> history_vcards = vCard::fromByteArray(vcards.toUtf8());

    for (auto vcard : history_vcards) {
        vCardProperty number_prop = vcard.property(VC_TELEPHONE);
        if (number_prop.isValid()) {
            QStringList values = number_prop.values();
            number = values.at(0);
        }
        vCardProperty name_prop = vcard.property(VC_FORMATTED_NAME);
        QStringList values = name_prop.values();
        name = values.at(0);
        // For calls with an empty name, fetch the name from contacts
        if (name.isEmpty()) {
            bool found = false;
            for (auto contact_obj : m_contacts) {
                Contact *contact = qobject_cast<Contact *>(contact_obj);
                QList<PhoneNumber *> numbers = contact->numbers();
                for (auto phone_number : numbers) {
                    if (number.endsWith(phone_number->number())) {
                        name = contact->name();
                        found = true;
                        break;
                    }
                }
                if (found == true)
                    break;
            }
            if (!found)
                name = number;
        }
        vCardProperty datetime_prop = vcard.property(VC_DATETIME);
        if (datetime_prop.isValid()) {
            vCardParamList params = datetime_prop.params();
            QStringList values = datetime_prop.values();
            type = params.at(0).value();
            datetime = values.at(0);
            // Convert the PBAP date/time to ISO 8601 format
            datetime.insert(4, '-');
            datetime.insert(7, '-');
            datetime.insert(13, ':');
            datetime.insert(16, ':');
        }
        m_calls.append(new RecentCall(name, number, datetime, type));
    }

    // Refresh model
    m_context->setContextProperty("RecentCallModel", QVariant::fromValue(m_calls));
}

void Pbap::sendSearchResults(QJsonArray results)
{
    QString name;

    if (results.empty())
        name = "Not Found";
    else
        name = results.at(0).toObject().value("name").toString();

    emit searchResults(name);
}

void Pbap::onConnected()
{
    QStringListIterator eventIterator(events);
    PbapMessage *tmsg;

    while (eventIterator.hasNext()) {
        tmsg = new PbapMessage();
        QJsonObject parameter;
        parameter.insert("value", eventIterator.next());
        tmsg->createRequest("subscribe", parameter);
        m_mloop->sendMessage(tmsg);
        tmsg->deleteLater();
    }
}

void Pbap::onDisconnected()
{
    QStringListIterator eventIterator(events);
    PbapMessage *tmsg;

    while (eventIterator.hasNext()) {
        tmsg = new PbapMessage();
        QJsonObject parameter;
        parameter.insert("value", eventIterator.next());
        tmsg->createRequest("unsubscribe", parameter);
        m_mloop->sendMessage(tmsg);
        tmsg->deleteLater();
    }
}

void Pbap::onMessageReceived(MessageType type, Message *msg)
{
    if (msg->isEvent() && type == PbapEventMessage) {
        PbapMessage *tmsg = qobject_cast<PbapMessage*>(msg);

        if (tmsg->isStatusEvent()) {
            emit statusChanged(tmsg->connected());
            if (tmsg->connected() == true) {
                refreshContacts(-1);
            }
        }
    } else if (msg->isReply() && type == ResponseRequestMessage) {
        ResponseMessage *tmsg = qobject_cast<ResponseMessage*>(msg);

        if (tmsg->requestVerb() == "contacts") {
            updateContacts(tmsg->replyData().value("vcards").toString());
        } else if (tmsg->requestVerb() == "history") {
            updateCalls(tmsg->replyData().value("vcards").toString());
        } else if (tmsg->requestVerb() == "search") {
            sendSearchResults(tmsg->replyData().value("results").toArray());
        }
    }

    msg->deleteLater();
}
