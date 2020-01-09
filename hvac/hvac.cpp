/*
 * Copyright (C) 2020 Konsulko Group
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
#include <QMimeDatabase>
#include <QtQml/QQmlEngine>

#include "message.h"
#include "messageengine.h"
#include "hvac.h"
#include "hvacmessage.h"
#include "responsemessage.h"

// TODO: don't duplicate defaults from HVAC service here
HVAC::HVAC (QUrl &url, QObject * parent) :
    QObject(parent),
    m_mloop(nullptr),
    m_fanspeed(0),
    m_temp_left_zone(21),
    m_temp_right_zone(21)
{
    m_mloop = new MessageEngine(url);
    QObject::connect(m_mloop, &MessageEngine::messageReceived, this, &HVAC::onMessageReceived);
}

HVAC::~HVAC()
{
    delete m_mloop;
}

void HVAC::control(QString verb, QString field, int value)
{
    HVACMessage *tmsg = new HVACMessage();
    QJsonObject parameter;

    parameter.insert(field, value);
    tmsg->createRequest(verb, parameter);
    m_mloop->sendMessage(tmsg);
    delete tmsg;
}

void HVAC::set_fanspeed(int speed)
{
    control("set", "FanSpeed", speed);
    emit fanSpeedChanged(speed);
}

void HVAC::set_temp_left_zone(int temp)
{
    control("set", "LeftTemperature", temp);
    control("set", "LeftLed", temp);
    emit leftTemperatureChanged(temp);
}

void HVAC::set_temp_right_zone(int temp)
{

    control("set", "RightTemperature", temp);
    control("set", "RightLed", temp);
    emit rightTemperatureChanged(temp);
}

void HVAC::onMessageReceived(MessageType type, Message *msg)
{
    if (msg->isEvent() && type == HVACEventMessage) {
        HVACMessage *tmsg = qobject_cast<HVACMessage*>(msg);

        if (tmsg->isLanguageEvent()) {
            // TODO: cannot be currently tested with identity service
            QVariantMap data = tmsg->eventData().toVariantMap();
            emit languageChanged(data.value("language").toString());
        }
    }
}
