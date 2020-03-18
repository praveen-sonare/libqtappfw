/*
 * Copyright (C) 2019, 2020 Konsulko Group
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

#include "callmessage.h"
#include "eventmessage.h"
#include "messagefactory.h"
#include "messageengine.h"
#include "navigation.h"


Navigation::Navigation (QUrl &url, QObject * parent) :
    QObject(parent),
    m_mloop(nullptr)
{
    m_mloop = new MessageEngine(url);
    QObject::connect(m_mloop, &MessageEngine::connected, this, &Navigation::onConnected);
    QObject::connect(m_mloop, &MessageEngine::disconnected, this, &Navigation::onDisconnected);
    QObject::connect(m_mloop, &MessageEngine::messageReceived, this, &Navigation::onMessageReceived);
}

Navigation::~Navigation()
{
    delete m_mloop;
}

void Navigation::sendWaypoint(double lat, double lon)
{
    std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
    if (!msg)
        return;

    CallMessage* nmsg = static_cast<CallMessage*>(msg.get());
    QJsonObject parameter, point;
    QJsonArray points;
    point.insert("latitude", lat);
    point.insert("longitude", lon);
    points.append(point);
    parameter.insert("points", points);
    nmsg->createRequest("navigation", "broadcast_waypoints", parameter);
    m_mloop->sendMessage(std::move(msg));
}

void Navigation::broadcastPosition(double lat, double lon, double drc, double dst)
{
    std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
    if (!msg)
        return;

    CallMessage* nmsg = static_cast<CallMessage*>(msg.get());
    QJsonObject parameter;

    parameter.insert("position", "car");
    parameter.insert("latitude", lat);
    parameter.insert("longitude", lon);
    parameter.insert("direction", drc);
    parameter.insert("distance", dst);

    nmsg->createRequest("navigation", "broadcast_position", parameter);
    m_mloop->sendMessage(std::move(msg));
}

void Navigation::broadcastRouteInfo(double lat, double lon, double route_lat, double route_lon)
{
    std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
    if (!msg)
        return;

    CallMessage* nmsg = static_cast<CallMessage*>(msg.get());
    QJsonObject parameter;

    parameter.insert("position", "route");
    parameter.insert("latitude", lat);
    parameter.insert("longitude", lon);
    parameter.insert("route_latitude", route_lat);
    parameter.insert("route_longitude", route_lon);

    nmsg->createRequest("navigation", "broadcast_position", parameter);
    m_mloop->sendMessage(std::move(msg));
}

void Navigation::broadcastStatus(QString state)
{
    std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
    if (!msg)
        return;

    CallMessage* nmsg = static_cast<CallMessage*>(msg.get());
    QJsonObject parameter;
    parameter.insert("state", state);
    nmsg->createRequest("navigation", "broadcast_status", parameter);
    m_mloop->sendMessage(std::move(msg));
}

void Navigation::onConnected()
{
    QStringListIterator eventIterator(events);

    while (eventIterator.hasNext()) {
        std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
        if (!msg)
            return;

        CallMessage* nmsg = static_cast<CallMessage*>(msg.get());
        QJsonObject parameter;
        parameter.insert("value", eventIterator.next());
        nmsg->createRequest("navigation", "subscribe", parameter);
        m_mloop->sendMessage(std::move(msg));
    }
}

void Navigation::onDisconnected()
{
    QStringListIterator eventIterator(events);

    while (eventIterator.hasNext()) {
        std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
        if (!msg)
            return;

        CallMessage* nmsg = static_cast<CallMessage*>(msg.get());
        QJsonObject parameter;
        parameter.insert("value", eventIterator.next());
        nmsg->createRequest("navigation", "unsubscribe", parameter);
        m_mloop->sendMessage(std::move(msg));
    }
}

void Navigation::onMessageReceived(std::shared_ptr<Message> msg)
{
    if (!msg)
        return;

    if (msg->isEvent()) {
        std::shared_ptr<EventMessage> emsg = std::static_pointer_cast<EventMessage>(msg);
        if (emsg->eventApi() != "navigation")
            return;

        if (emsg->eventName() == "position") {
            emit positionEvent(emsg->eventData().toVariantMap());
        }
        else if (emsg->eventName() == "status") {
            emit statusEvent(emsg->eventData().toVariantMap());
        }
        else if (emsg->eventName() == "waypoints") {
            emit waypointsEvent(emsg->eventData().toVariantMap());
        }
    }
}
