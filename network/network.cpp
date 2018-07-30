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
#include "network.h"
#include "networkmessage.h"
#include "responsemessage.h"
#include "wifinetworkmodel.h"

Network::Network (QUrl &url, QQmlContext *context, QObject * parent) :
    QObject(parent),
    m_mloop(nullptr),
    m_wifi(nullptr),
    m_wifiConnected(false),
    m_wifiEnabled(false),
    m_wifiStrength(0)
{
    m_mloop = new MessageEngine(url);
    m_wifi = new WifiNetworkModel();

    context->setContextProperty("WifiNetworkModel", m_wifi);

    QObject::connect(m_mloop, &MessageEngine::connected, this, &Network::onConnected);
    QObject::connect(m_mloop, &MessageEngine::disconnected, this, &Network::onDisconnected);
    QObject::connect(m_mloop, &MessageEngine::messageReceived, this, &Network::onMessageReceived);
    QObject::connect(m_wifi, &WifiNetworkModel::strengthChanged, this, &Network::updateWifiStrength);
}

Network::~Network()
{
    delete m_mloop;
    delete m_wifi;
}

void Network::connect(QString service)
{
    NetworkMessage *nmsg = new NetworkMessage();
    QJsonObject parameter;

    parameter.insert("service", service);

    nmsg->createRequest("connect_service", parameter);
    m_mloop->sendMessage(nmsg);
    nmsg->deleteLater();
}

void Network::disconnect(QString service)
{
    NetworkMessage *nmsg = new NetworkMessage();
    QJsonObject parameter;

    parameter.insert("service", service);

    nmsg->createRequest("disconnect_service", parameter);
    m_mloop->sendMessage(nmsg);
    nmsg->deleteLater();
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
    nmsg->deleteLater();
}

void Network::power(bool on)
{
    if (on)
        enableTechnology("wifi");
    else
        disableTechnology("wifi");
}

void Network::enableTechnology(QString type)
{
    NetworkMessage *nmsg = new NetworkMessage();
    QJsonObject parameter;

    parameter.insert("technology", type);
    nmsg->createRequest("enable_technology", parameter);
    m_mloop->sendMessage(nmsg);

    nmsg->deleteLater();
}

void Network::disableTechnology(QString type)
{
    NetworkMessage *nmsg = new NetworkMessage();
    QJsonObject parameter;

    parameter.insert("technology", type);
    nmsg->createRequest("disable_technology", parameter);
    m_mloop->sendMessage(nmsg);

    nmsg->deleteLater();
}

void Network::scanServices(QString type)
{
    NetworkMessage *nmsg = new NetworkMessage();
    QJsonObject parameter;

    parameter.insert("technology", type);
    nmsg->createRequest("scan_services", parameter);
    m_mloop->sendMessage(nmsg);

    nmsg->deleteLater();
}

bool Network::addService(QJsonObject service)
{
        // Ignore services that are already added
        QString id = service.value("service").toString();
        if (m_wifi->getNetwork(id))
            return false;

        QJsonObject properties = service.value("properties").toObject();

        // Ignore hidden SSIDs
        QString ssid = properties.value("name").toString();
        if (ssid == "")
            return false;

        // Ignore technologies other than WiFi
        QString type = properties.value("type").toString();
        if (type != "wifi")
            return false;

        // Initially support only IPv4 and the first security method found
        QString address = properties.value("ipv4").toObject().value("address").toString();
        QString security = properties.value("security").toArray().at(0).toString();
        QString state = properties.value("state").toString();
        int strength = properties.value("strength").toInt();

        WifiNetwork *network = new WifiNetwork(address, security, id, ssid, state, strength);
        m_wifi->addNetwork(network);

        if ((state == "ready") || (state == "online"))
            updateWifiStrength(strength);

        return true;
}

void Network::removeService(QJsonObject service)
{
    QString id = service.value("service").toString();
    WifiNetwork *network = m_wifi->getNetwork(id);

    if (network)
        m_wifi->removeNetwork(network);
}

void Network::addServices(QJsonArray services)
{
    for (auto service : services)
        addService(service.toObject());
}

void Network::getServices()
{
    NetworkMessage *nmsg = new NetworkMessage();
    QJsonObject parameter;

    nmsg->createRequest("services", parameter);
    m_mloop->sendMessage(nmsg);
    nmsg->deleteLater();
}

void Network::updateWifiStatus(QJsonObject properties)
{
    if (properties.contains("connected")) {
        m_wifiConnected = properties.value("connected").toBool();
        emit wifiConnectedChanged(m_wifiConnected);
    }

    if (properties.contains("powered")) {
        m_wifiEnabled = properties.value("powered").toBool();
        emit wifiEnabledChanged(m_wifiEnabled);
        if (m_wifiEnabled)
            getServices();
    }
}

void Network::updateWifiStrength(int strength)
{
    m_wifiStrength = strength;
    emit wifiStrengthChanged(m_wifiStrength);
}

void Network::parseTechnologies(QJsonArray technologies)
{
    for (auto value : technologies) {
        QJsonObject technology = value.toObject();
        QJsonObject properties = technology.value("properties").toObject();
        QString type = properties.value("type").toString();
        if (type == "wifi") {
            updateWifiStatus(properties);
            if (m_wifiEnabled)
                getServices();
            break;
        }
    }
}

void Network::getTechnologies()
{
    NetworkMessage *nmsg = new NetworkMessage();
    QJsonObject parameter;

    nmsg->createRequest("technologies", parameter);
    m_mloop->sendMessage(nmsg);
    nmsg->deleteLater();
}

void Network::updateServiceProperties(QJsonObject data)
{
    QString service = data.value("service").toString();
    QJsonObject properties = data.value("properties").toObject();
    m_wifi->updateProperties(service, properties);
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
            QString id = service.value("service").toString();
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
        if (technology.value("technology").toString() == "wifi") {
            QJsonObject properties = technology.value("properties").toObject();
            updateWifiStatus(properties);
        }
    }
}

void Network::processReply(ResponseMessage *rmsg)
{
    if (rmsg->requestVerb() == "services") {
        addServices(rmsg->replyData().value("values").toArray());
    } else if (rmsg->requestVerb() == "technologies") {
        parseTechnologies(rmsg->replyData().value("values").toArray());
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
        nmsg->deleteLater();
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
        nmsg->deleteLater();
    }

    m_wifi->removeAllNetworks();
}
