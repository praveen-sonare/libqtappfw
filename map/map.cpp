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
#include "map.h"
#include "mapmessage.h"
#include "responsemessage.h"

Map::Map (QUrl &url, QObject * parent) :
    QObject(parent),
    m_mloop(nullptr)
{
    m_mloop = new MessageEngine(url);
    QObject::connect(m_mloop, &MessageEngine::connected, this, &Map::onConnected);
    QObject::connect(m_mloop, &MessageEngine::disconnected, this, &Map::onDisconnected);
    QObject::connect(m_mloop, &MessageEngine::messageReceived, this, &Map::onMessageReceived);
}

Map::~Map()
{
    delete m_mloop;
}

void Map::compose(QString recipient, QString message)
{
    MapMessage *tmsg = new MapMessage();
    QJsonObject parameter;
    parameter.insert("recipient", recipient);
    parameter.insert("message", message);
    tmsg->createRequest("compose", parameter);
    m_mloop->sendMessage(tmsg);
    delete tmsg;
}

void Map::onConnected()
{
    MapMessage *tmsg = new MapMessage();
    QJsonObject parameter;
    parameter.insert("value", "notification");
    tmsg->createRequest("subscribe", parameter);
    m_mloop->sendMessage(tmsg);
    delete tmsg;
}

void Map::onDisconnected()
{
    MapMessage *tmsg = new MapMessage();
    QJsonObject parameter;
    parameter.insert("value", "notification");
    tmsg->createRequest("unsubscribe", parameter);
    m_mloop->sendMessage(tmsg);
    delete tmsg;
}

void Map::onMessageReceived(MessageType type, Message *msg)
{
    if (type == MapEventMessage) {
        MapMessage *tmsg = qobject_cast<MapMessage*>(msg);

        if (tmsg->isNotificationEvent()) {
            emit notificationEvent(tmsg->eventData().toVariantMap());
        }
    }

    msg->deleteLater();
}
