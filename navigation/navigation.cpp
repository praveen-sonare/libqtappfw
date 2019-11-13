/*
 * Copyright (C) 2019 Konsulko Group
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

#include "message.h"
#include "messageengine.h"
#include "navigation.h"
#include "navigationmessage.h"
#include "responsemessage.h"

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
    NavigationMessage *nmsg = new NavigationMessage();
    QJsonObject parameter, point;
    QJsonArray points;
    point.insert("latitude", lat);
    point.insert("longitude", lon);
    points.append(point);
    parameter.insert("points", points);
    nmsg->createRequest("broadcast_waypoints", parameter);
    m_mloop->sendMessage(nmsg);
    delete nmsg;
}

void Navigation::broadcastPosition(double lat, double lon, double drc, double dst)
{
    NavigationMessage *nmsg = new NavigationMessage();
    QJsonObject parameter;

    parameter.insert("position", "car");
    parameter.insert("latitude", lat);
    parameter.insert("longitude", lon);
    parameter.insert("direction", drc);
    parameter.insert("distance", dst);

    nmsg->createRequest("broadcast_position", parameter);
    m_mloop->sendMessage(nmsg);
    delete nmsg;
}

void Navigation::broadcastRouteInfo(double lat, double lon, double route_lat, double route_lon)
{
    NavigationMessage *nmsg = new NavigationMessage();
    QJsonObject parameter;

    parameter.insert("position", "route");
    parameter.insert("latitude", lat);
    parameter.insert("longitude", lon);
    parameter.insert("route_latitude", route_lat);
    parameter.insert("route_longitude", route_lon);

    nmsg->createRequest("broadcast_position", parameter);
    m_mloop->sendMessage(nmsg);
    delete nmsg;
}

void Navigation::broadcastStatus(QString state)
{
    NavigationMessage *nmsg = new NavigationMessage();
    QJsonObject parameter;
    parameter.insert("state", state);
    nmsg->createRequest("broadcast_status", parameter);
    m_mloop->sendMessage(nmsg);
    delete nmsg;
}

void Navigation::onConnected()
{
    QStringListIterator eventIterator(events);
    NavigationMessage *nmsg;

    while (eventIterator.hasNext()) {
        nmsg = new NavigationMessage();
        QJsonObject parameter;
        parameter.insert("value", eventIterator.next());
        nmsg->createRequest("subscribe", parameter);
        m_mloop->sendMessage(nmsg);
        delete nmsg;
    }
}

void Navigation::onDisconnected()
{
    QStringListIterator eventIterator(events);
    NavigationMessage *nmsg;

    while (eventIterator.hasNext()) {
        nmsg = new NavigationMessage();
        QJsonObject parameter;
        parameter.insert("value", eventIterator.next());
        nmsg->createRequest("unsubscribe", parameter);
        m_mloop->sendMessage(nmsg);
        delete nmsg;
    }
}

void Navigation::onMessageReceived(MessageType type, Message *msg)
{
    if (type == NavigationEventMessage) {
        NavigationMessage *tmsg = qobject_cast<NavigationMessage*>(msg);

        if (tmsg->isPositionEvent()) {
            emit positionEvent(tmsg->eventData().toVariantMap());
        }
        if (tmsg->isStatusEvent()) {
            emit statusEvent(tmsg->eventData().toVariantMap());
        }
        if (tmsg->isWaypointsEvent()) {
            emit waypointsEvent(tmsg->eventData().toVariantMap());
        }
    }

    msg->deleteLater();
}
