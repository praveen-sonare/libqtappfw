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

#include <QDebug>
#include <QMetaEnum>
#include <QMimeDatabase>
#include <QtQml/QQmlEngine>

#include "callmessage.h"
#include "eventmessage.h"
#include "messagefactory.h"
#include "messageengine.h"
#include "messageenginefactory.h"
#include "hvac.h"


// TODO: don't duplicate defaults from HVAC service here
HVAC::HVAC (QUrl &url, QObject * parent) :
    QObject(parent),
    m_fanspeed(0),
    m_temp_left_zone(21),
    m_temp_right_zone(21)
{
    m_mloop = MessageEngineFactory::getInstance().getMessageEngine(url);
    QObject::connect(m_mloop.get(), &MessageEngine::messageReceived, this, &HVAC::onMessageReceived);
}

HVAC::~HVAC()
{
}

void HVAC::control(QString verb, QString field, int value)
{
    std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
    if (!msg)
        return;

    CallMessage* hmsg = static_cast<CallMessage*>(msg.get());
    QJsonObject parameter;

    parameter.insert(field, value);
    hmsg->createRequest("hvac", verb, parameter);
    m_mloop->sendMessage(std::move(msg));
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

void HVAC::onMessageReceived(std::shared_ptr<Message> msg)
{
    if (!msg)
        return;

    if (msg->isEvent()) {
        std::shared_ptr<EventMessage> emsg = std::static_pointer_cast<EventMessage>(msg);
        if (emsg->eventApi() != "hvac")
            return;

        if (emsg->eventName() == "language") {
            // TODO: cannot be currently tested with identity service
            QVariantMap data = emsg->eventData().toVariantMap();
            emit languageChanged(data.value("language").toString());
        }
    }
}
