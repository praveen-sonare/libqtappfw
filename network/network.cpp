/*
 * Copyright (C) 2018-2020 Konsulko Group
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

#include <QDebug>
#include <QtQml/QQmlEngine>

#include "callmessage.h"
#include "eventmessage.h"
#include "responsemessage.h"
#include "messagefactory.h"
#include "messageengine.h"
#include "networkadapter.h"
#include "network.h"


Network::Network (QUrl &url, QQmlContext *context, QObject * parent) :
    QObject(parent),
    m_mloop(nullptr)
{
    m_mloop = new MessageEngine(url);
    m_adapters.append(new WifiAdapter(this, context, parent));

    QObject::connect(m_mloop, &MessageEngine::connected, this, &Network::onConnected);
    QObject::connect(m_mloop, &MessageEngine::disconnected, this, &Network::onDisconnected);
    QObject::connect(m_mloop, &MessageEngine::messageReceived, this, &Network::onMessageReceived);

    m_adapters.append(new WiredAdapter(this, context, parent));
}

Network::~Network()
{
    delete m_mloop;
    while (!m_adapters.isEmpty())
        m_adapters.takeLast();
}

void Network::connect(QString service)
{
    std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
    if (!msg)
        return;

    CallMessage* nmsg = static_cast<CallMessage*>(msg.get());
    QJsonObject parameter;

    parameter.insert("service", service);

    nmsg->createRequest("network-manager", "connect_service", parameter);
    m_mloop->sendMessage(std::move(msg));
}

void Network::disconnect(QString service)
{
    std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
    if (!msg)
        return;

    CallMessage* nmsg = static_cast<CallMessage*>(msg.get());
    QJsonObject parameter;

    parameter.insert("service", service);

    nmsg->createRequest("network-manager", "disconnect_service", parameter);
    m_mloop->sendMessage(std::move(msg));
}

void Network::remove(QString service)
{
    std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
    if (!msg)
        return;

    CallMessage* nmsg = static_cast<CallMessage*>(msg.get());
    QJsonObject parameter;

    parameter.insert("service", service);

    nmsg->createRequest("network-manager", "remove_service", parameter);
    m_mloop->sendMessage(std::move(msg));
}

void Network::power(bool on, QString type)
{
    if (on)
        enableTechnology(type);
    else
        disableTechnology(type);
}

void Network::input(int id, QString passphrase)
{
    std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
    if (!msg)
        return;

    CallMessage* nmsg = static_cast<CallMessage*>(msg.get());
    QJsonObject parameter, fields;

    parameter.insert("id", id);
    fields.insert("passphrase", passphrase);
    parameter.insert("fields", fields);

    nmsg->createRequest("network-manager", "agent_response", parameter);
    m_mloop->sendMessage(std::move(msg));
}

void Network::configureAddress(QString service, QVariantList paramlist)
{
    std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
    if (!msg)
        return;

    CallMessage* nmsg = static_cast<CallMessage*>(msg.get());
    QJsonObject parameter, type, properties;
    QJsonArray  values = QJsonArray::fromVariantList(paramlist);

    if (values.isEmpty() || values.count() < 4) {
        qWarning("Invalid addressing params");
        return;
    }

    properties.insert("method", values[0]);
    properties.insert("address", values[1]);
    properties.insert("netmask", values[2]);
    properties.insert("gateway", values[3]);
    type.insert("ipv4.configuration", properties);
    parameter.insert("properties", type);
    parameter.insert("service", service);

    nmsg->createRequest("network-manager", "set_property", parameter);
    m_mloop->sendMessage(std::move(msg));
}

void Network::configureNameServer(QString service, QVariantList paramlist)
{
    std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
    if (!msg)
        return;

    CallMessage* nmsg = static_cast<CallMessage*>(msg.get());
    QJsonObject parameter, properties;
    QJsonArray  values = QJsonArray::fromVariantList(paramlist);

    if (values.isEmpty() || values.count() < 2) {
        qWarning("Invalid nameserver params");
        return;
    }

    QStringList nslist = values[1].toString().split(' ');
    QJsonArray nameservers = QJsonArray::fromStringList(nslist);

    properties.insert("nameservers.configuration", nameservers);
    parameter.insert("properties", properties);
    parameter.insert("service", service);

    nmsg->createRequest("network-manager", "set_property", parameter);
    m_mloop->sendMessage(std::move(msg));
}

void Network::getServices()
{
    std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
    if (!msg)
        return;

    CallMessage* nmsg = static_cast<CallMessage*>(msg.get());
    QJsonObject parameter;

    nmsg->createRequest("network-manager", "services", parameter);
    m_mloop->sendMessage(std::move(msg));
}

AdapterIf* Network::findAdapter(QString type)
{
    QList<AdapterIf*>::iterator iter;
    for (iter = m_adapters.begin(); iter != m_adapters.end(); ++iter)
        if  ((*iter)->getType() == type)
            return (*iter);
    return nullptr;
}

void Network::updateServiceProperties(QJsonObject data)
{
    QString service = data.value("service").toString();
    QJsonObject properties = data.value("properties").toObject();
    QList<AdapterIf*>::iterator iter;
    for (iter = m_adapters.begin(); iter != m_adapters.end(); ++iter)
        (*iter)->updateProperties(service, properties);

}

bool Network::addService(QJsonObject service)
{
    QString id = service.value("service").toString();
    QJsonObject properties = service.value("properties").toObject();
    QList<AdapterIf*>::iterator iter;
    for (iter = m_adapters.begin(); iter != m_adapters.end(); ++iter)
        if ((*iter)->addService(id, properties))
            return true;

    return false;
}

void Network::removeService(QJsonObject service)
{
    QString id = service.value("service").toString();
    QList<AdapterIf*>::iterator iter;
    for (iter = m_adapters.begin(); iter != m_adapters.end(); ++iter)
        (*iter)->removeService(id);
}

void Network::addServices(QJsonArray services)
{
    for (auto service : services)
        addService(service.toObject());
}

void Network::scanServices(QString type)
{
    std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
    if (!msg)
        return;

    CallMessage* nmsg = static_cast<CallMessage*>(msg.get());
    QJsonObject parameter;

    parameter.insert("technology", type);
    nmsg->createRequest("network-manager", "scan_services", parameter);
    m_mloop->sendMessage(std::move(msg));
}

void Network::disableTechnology(QString type)
{
    std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
    if (!msg)
        return;

    CallMessage* nmsg = static_cast<CallMessage*>(msg.get());
    QJsonObject parameter;

    parameter.insert("technology", type);
    nmsg->createRequest("network-manager", "disable_technology", parameter);
    m_mloop->sendMessage(std::move(msg));
}

void Network::enableTechnology(QString type)
{
    std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
    if (!msg)
        return;

    CallMessage* nmsg = static_cast<CallMessage*>(msg.get());
    QJsonObject parameter;

    parameter.insert("technology", type);
    nmsg->createRequest("network-manager", "enable_technology", parameter);
    m_mloop->sendMessage(std::move(msg));
}

void Network::parseTechnologies(QJsonArray technologies)
{
    for (auto value : technologies) {
        QJsonObject technology = value.toObject();
        QJsonObject properties = technology.value("properties").toObject();
        QString type = properties.value("type").toString();

        AdapterIf* adapter = findAdapter(type);
        if (adapter)
            adapter->updateStatus(properties);
    }
}

void Network::getTechnologies()
{
    std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
    if (!msg)
        return;

    CallMessage* nmsg = static_cast<CallMessage*>(msg.get());
    QJsonObject parameter;

    nmsg->createRequest("network-manager", "technologies", parameter);
    m_mloop->sendMessage(std::move(msg));
}

void Network::processEvent(std::shared_ptr<Message> msg)
{
    std::shared_ptr<EventMessage> emsg = std::static_pointer_cast<EventMessage>(msg);
    QString ename = emsg->eventName();
    QString eapi = emsg->eventApi();
    QJsonObject data = emsg->eventData();

    if (eapi != "network-manager")
        return;

    if (ename == "agent") {
        QJsonObject fields = data.value("fields").toObject();
        QJsonObject passphrase = fields.value("passphrase").toObject();
        QString type = passphrase.value("type").toString();
        QString reqmt = passphrase.value("requirement").toString();
        if (((type == "psk") || (type == "wep")) && (reqmt == "mandatory")) {
            int id = data.value("id").toInt();
            emit inputRequest(id);
        }
    } else if (ename == "services") {
        QJsonArray services = data.value("values").toArray();
        for (auto value : services) {
            QJsonObject service = value.toObject();
            QString action = service.value("action").toString();
            if (action == "changed") {
                addService(service);
            } else if (action == "removed") {
                 removeService(service);
            }
        }
    } else if (ename == "service_properties") {
        updateServiceProperties(data);
    } else if (ename == "technology_properties") {
        QJsonObject properties = data.value("properties").toObject();
        QString type = data.value("technology").toString();
        AdapterIf* adapter = findAdapter(type);
        if (adapter)
            adapter->updateStatus(properties);
    }
}

void Network::processReply(std::shared_ptr<Message> msg)
{
    std::shared_ptr<ResponseMessage> rmsg = std::static_pointer_cast<ResponseMessage>(msg);
    QString verb = rmsg->requestVerb();
    QJsonObject data = rmsg->replyData();

    if (verb == "services") {
        addServices(data.value("values").toArray());
    } else if (verb == "technologies") {
        parseTechnologies(data.value("values").toArray());
    } else if (verb == "connect_service") {
        if (rmsg->replyStatus() == "failed" && rmsg->replyInfo().contains("invalid-key")) {
            emit invalidPassphrase(rmsg->requestParameters()["service"].toString());
        }
    }
}

void Network::onMessageReceived(std::shared_ptr<Message> msg)
{
    if (!msg)
        return;

    if (msg->isEvent())
        processEvent(msg);
    else if (msg->isReply())
        processReply(msg);
}

void Network::onConnected()
{
    QStringListIterator eventIterator(events);

    while (eventIterator.hasNext()) {
        std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
        if (!msg)
            return;

        CallMessage* nmsg = static_cast<CallMessage*>(msg.get());
        QJsonObject parameter;
        parameter.insert("value", eventIterator.next());
        nmsg->createRequest("network-manager", "subscribe", parameter);
        m_mloop->sendMessage(std::move(msg));
    }

    getTechnologies();
}

void Network::onDisconnected()
{
    QStringListIterator eventIterator(events);

    while (eventIterator.hasNext()) {
        std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
        if (!msg)
            return;
        CallMessage *nmsg = static_cast<CallMessage*>(msg.get());
        QJsonObject parameter;
        parameter.insert("value", eventIterator.next());
        nmsg->createRequest("network-manager", "unsubscribe", parameter);
        m_mloop->sendMessage(std::move(msg));
    }

    getTechnologies();
}
