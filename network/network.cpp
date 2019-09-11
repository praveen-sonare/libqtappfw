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

#include <QtQml/QQmlEngine>

#include "message.h"
#include "messageengine.h"
#include "network.h"
#include "networkmessage.h"
#include "responsemessage.h"
#include "networkadapter.h"

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
    NetworkMessage *nmsg = new NetworkMessage();
    QJsonObject parameter;

    parameter.insert("service", service);

    nmsg->createRequest("connect_service", parameter);
    m_mloop->sendMessage(nmsg);
    delete nmsg;
}

void Network::disconnect(QString service)
{
    NetworkMessage *nmsg = new NetworkMessage();
    QJsonObject parameter;

    parameter.insert("service", service);

    nmsg->createRequest("disconnect_service", parameter);
    m_mloop->sendMessage(nmsg);
    delete nmsg;
}

void Network::remove(QString service)
{
    NetworkMessage *nmsg = new NetworkMessage();
    QJsonObject parameter;

    parameter.insert("service", service);

    nmsg->createRequest("remove_service", parameter);
    m_mloop->sendMessage(nmsg);
    delete nmsg;
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
    NetworkMessage *nmsg = new NetworkMessage();
    QJsonObject parameter, fields;

    parameter.insert("id", id);
    fields.insert("passphrase", passphrase);
    parameter.insert("fields", fields);

    nmsg->createRequest("agent_response", parameter);
    m_mloop->sendMessage(nmsg);
    delete nmsg;
}

void Network::configureAddress(QString service, QVariantList paramlist)
{
    NetworkMessage *nmsg = new NetworkMessage();
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

    nmsg->createRequest("set_property", parameter);
    m_mloop->sendMessage(nmsg);
    delete nmsg;
}

void Network::configureNameServer(QString service, QVariantList paramlist)
{
    NetworkMessage *nmsg = new NetworkMessage();
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

    nmsg->createRequest("set_property", parameter);
    m_mloop->sendMessage(nmsg);
    delete nmsg;
}

void Network::getServices()
{
    NetworkMessage *nmsg = new NetworkMessage();
    QJsonObject parameter;

    nmsg->createRequest("services", parameter);
    m_mloop->sendMessage(nmsg);
    delete nmsg;
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
    NetworkMessage *nmsg = new NetworkMessage();
    QJsonObject parameter;

    parameter.insert("technology", type);
    nmsg->createRequest("scan_services", parameter);
    m_mloop->sendMessage(nmsg);

    delete nmsg;
}

void Network::disableTechnology(QString type)
{
    NetworkMessage *nmsg = new NetworkMessage();
    QJsonObject parameter;

    parameter.insert("technology", type);
    nmsg->createRequest("disable_technology", parameter);
    m_mloop->sendMessage(nmsg);

    delete nmsg;
}

void Network::enableTechnology(QString type)
{
    NetworkMessage *nmsg = new NetworkMessage();
    QJsonObject parameter;

    parameter.insert("technology", type);
    nmsg->createRequest("enable_technology", parameter);
    m_mloop->sendMessage(nmsg);

    delete nmsg;
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
    NetworkMessage *nmsg = new NetworkMessage();
    QJsonObject parameter;

    nmsg->createRequest("technologies", parameter);
    m_mloop->sendMessage(nmsg);
    delete nmsg;
}

void Network::processEvent(NetworkMessage *nmsg)
{
    if (nmsg->eventName() == "agent") {
        QJsonObject agent = nmsg->eventData();
        QJsonObject fields = agent.value("fields").toObject();
        QJsonObject passphrase = fields.value("passphrase").toObject();
        QString type = passphrase.value("type").toString();
        QString reqmt = passphrase.value("requirement").toString();
        if (((type == "psk") || (type == "wep")) && (reqmt == "mandatory")) {
            int id = agent.value("id").toInt();
            emit inputRequest(id);
        }
    } else if (nmsg->eventName() == "services") {
        QJsonArray services = nmsg->eventData().value("values").toArray();
        for (auto value : services) {
            QJsonObject service = value.toObject();
            QString action = service.value("action").toString();
            if (action == "changed") {
                addService(service);
            } else if (action == "removed") {
                 removeService(service);
            }
        }
    } else if (nmsg->eventName() == "service_properties") {
        updateServiceProperties(nmsg->eventData());
    } else if (nmsg->eventName() == "technology_properties") {
        QJsonObject technology = nmsg->eventData();
        QJsonObject properties = technology.value("properties").toObject();
        QString type = technology.value("technology").toString();
        AdapterIf* adapter = findAdapter(type);
        if (adapter)
            adapter->updateStatus(properties);
    }
}

void Network::processReply(ResponseMessage *rmsg)
{
    if (rmsg->requestVerb() == "services") {
        addServices(rmsg->replyData().value("values").toArray());
    } else if (rmsg->requestVerb() == "technologies") {
        parseTechnologies(rmsg->replyData().value("values").toArray());
    } else if (rmsg->requestVerb() == "connect_service") {
        if (rmsg->replyStatus() == "failed" && rmsg->replyInfo().contains("invalid-key")) {
            emit invalidPassphrase(rmsg->requestData()["parameter"].toMap()["service"].toString());
        }
    }
}

void Network::onMessageReceived(MessageType type, Message *msg)
{
    if (msg->isEvent() && (type == NetworkEventMessage)) {
        processEvent(qobject_cast<NetworkMessage*>(msg));
    } else if (msg->isReply() && (type == ResponseRequestMessage)) {
        processReply(qobject_cast<ResponseMessage*>(msg));
    }

    msg->deleteLater();
}

void Network::onConnected()
{
    QStringListIterator eventIterator(events);
    NetworkMessage *nmsg;

    while (eventIterator.hasNext()) {
        nmsg = new NetworkMessage();
        QJsonObject parameter;
        parameter.insert("value", eventIterator.next());
        nmsg->createRequest("subscribe", parameter);
        m_mloop->sendMessage(nmsg);
        delete nmsg;
    }

    getTechnologies();
}

void Network::onDisconnected()
{
    QStringListIterator eventIterator(events);
    NetworkMessage *nmsg;

    while (eventIterator.hasNext()) {
        nmsg = new NetworkMessage();
        QJsonObject parameter;
        parameter.insert("value", eventIterator.next());
        nmsg->createRequest("unsubscribe", parameter);
        m_mloop->sendMessage(nmsg);
        delete nmsg;
    }

    getTechnologies();
}
