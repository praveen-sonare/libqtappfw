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

#include "callmessage.h"
#include "eventmessage.h"
#include "responsemessage.h"
#include "messagefactory.h"
#include "messageengine.h"
#include "messageenginefactory.h"
#include "radio.h"


Radio::Radio (QUrl &url, QQmlContext *context, QObject * parent) :
    QObject(parent),
    m_band(1),
    m_frequency(0),
    m_minFrequency(0),
    m_maxFrequency(0),
    m_playing(false),
    m_scanning(false)
{
    m_mloop = MessageEngineFactory::getInstance().getMessageEngine(url);
    m_context = context;

    QObject::connect(m_mloop.get(), &MessageEngine::connected, this, &Radio::onConnected);
    QObject::connect(m_mloop.get(), &MessageEngine::disconnected, this, &Radio::onDisconnected);
    QObject::connect(m_mloop.get(), &MessageEngine::messageReceived, this, &Radio::onMessageReceived);
}

Radio::~Radio()
{
}

void Radio::setBand(int band)
{
    std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
    if (!msg)
        return;

    CallMessage* rmsg = static_cast<CallMessage*>(msg.get());
    QJsonObject parameter;

    parameter.insert("band", band ? "FM": "AM");
    rmsg->createRequest("radio", "band", parameter);
    m_mloop->sendMessage(std::move(msg));
}

void Radio::setFrequency(int frequency)
{
    std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
    if (!msg)
        return;

    CallMessage* rmsg = static_cast<CallMessage*>(msg.get());
    QJsonObject parameter;

    if (m_scanning)
        scanStop();

    if (frequency == m_frequency)
        return;

    parameter.insert("value", QString::number(frequency));
    rmsg->createRequest("radio", "frequency", parameter);
    m_mloop->sendMessage(std::move(msg));

    // To improve UI responsiveness, signal the change here immediately
    // This fixes visual glitchiness in the slider caused by the frequency
    // update event taking long enough that the QML engine gets a chance
    // to update the slider with the current value before the event with
    // the new value comes.
    m_frequency = frequency;
    emit frequencyChanged(m_frequency);
}

// control related methods

void Radio::start()
{
    std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
    if (!msg)
        return;

    CallMessage* rmsg = static_cast<CallMessage*>(msg.get());
    QJsonObject parameter;

    rmsg->createRequest("radio", "start", parameter);
    m_mloop->sendMessage(std::move(msg));
}

void Radio::stop()
{
    std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
    if (!msg)
        return;

    CallMessage* rmsg = static_cast<CallMessage*>(msg.get());
    QJsonObject parameter;
    rmsg->createRequest("radio", "stop", parameter);
    m_mloop->sendMessage(std::move(msg));
}

void Radio::scanForward()
{
    if (m_scanning)
        return;

    std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
    CallMessage* rmsg = static_cast<CallMessage*>(msg.get());
    QJsonObject parameter;
    parameter.insert("direction", "forward");
    rmsg->createRequest("radio", "scan_start", parameter);
    m_mloop->sendMessage(std::move(msg));

    m_scanning = true;
    emit scanningChanged(m_scanning);
}

void Radio::scanBackward()
{
    if (m_scanning)
        return;

    std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
    if (!msg)
        return;

    CallMessage* rmsg = static_cast<CallMessage*>(msg.get());
    QJsonObject parameter;
    parameter.insert("direction", "backward");
    rmsg->createRequest("radio", "scan_start", parameter);
    m_mloop->sendMessage(std::move(msg));

    m_scanning = true;
    emit scanningChanged(m_scanning);
}

void Radio::scanStop()
{
    std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
    if (!msg)
        return;

    CallMessage* rmsg = static_cast<CallMessage*>(msg.get());
    QJsonObject parameter;
    rmsg->createRequest("radio", "scan_stop", parameter);
    m_mloop->sendMessage(std::move(msg));

    m_scanning = false;
    emit scanningChanged(m_scanning);
}

void Radio::updateFrequencyBandParameters()
{
    std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
    if (!msg)
        return;

    CallMessage* rmsg = static_cast<CallMessage*>(msg.get());
    QJsonObject parameter;
    parameter.insert("band", m_band ? "FM" : "AM");
    rmsg->createRequest("radio", "frequency_range", parameter);
    m_mloop->sendMessage(std::move(msg));

    std::unique_ptr<Message> msg2 = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
    if (!msg2)
        return;

    rmsg = static_cast<CallMessage*>(msg2.get());
    rmsg->createRequest("radio", "frequency_step", parameter);
    m_mloop->sendMessage(std::move(msg2));
}

void Radio::onConnected()
{
    QStringListIterator eventIterator(events);

    while (eventIterator.hasNext()) {
        std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
        if (!msg)
            return;

        CallMessage* rmsg = static_cast<CallMessage*>(msg.get());
        QJsonObject parameter;
        parameter.insert("value", eventIterator.next());
        rmsg->createRequest("radio", "subscribe", parameter);
        m_mloop->sendMessage(std::move(msg));
    }

    // Trigger initial update of frequency band parameters (min/max/step)
    updateFrequencyBandParameters();
}

void Radio::onDisconnected()
{
    QStringListIterator eventIterator(events);

    while (eventIterator.hasNext()) {
        std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
        if (!msg)
            return;

        CallMessage* rmsg = static_cast<CallMessage*>(msg.get());
        QJsonObject parameter;
        parameter.insert("value", eventIterator.next());
        rmsg->createRequest("radio", "unsubscribe", parameter);
        m_mloop->sendMessage(std::move(msg));
    }
}

void Radio::onMessageReceived(std::shared_ptr<Message> msg)
{
    if (!msg)
        return;

    if (msg->isEvent()) {
        std::shared_ptr<EventMessage> emsg = std::static_pointer_cast<EventMessage>(msg);
        QString ename = emsg->eventName();
        QString eapi = emsg->eventApi();
        QJsonObject data = emsg->eventData();
        if (ename == "frequency") {
            unsigned int frequency = data.value("value").toInt();
            m_frequency = frequency;
            if (!m_scanning && (m_frequency != frequency)) {
                emit frequencyChanged(m_frequency);
            }
        } else if (ename == "station_found") {
            m_scanning = false;
            emit scanningChanged(m_scanning);

            m_frequency = data.value("value").toInt();
            emit frequencyChanged(m_frequency);
        } else if (ename == "status") {
            if (data.value("value") == QString("playing")) {
                m_playing = true;
                emit playingChanged(m_playing);
            } else if (data.value("value") == QString("stopped")) {
                m_playing = false;
                emit playingChanged(m_playing);
            }
        }
    } else if (msg->isReply()) {
        std::shared_ptr<ResponseMessage> rmsg = std::static_pointer_cast<ResponseMessage>(msg);
        QString api = rmsg->requestApi();
        if (api != "radio")
            return;

        QString verb = rmsg->requestVerb();
        QJsonObject data = rmsg->replyData();
        if (verb == "frequency_range") {
            m_minFrequency = data.value("min").toInt();
            emit minFrequencyChanged(m_minFrequency);
            m_maxFrequency = data.value("max").toInt();
            emit maxFrequencyChanged(m_maxFrequency);

            // Handle start up
            if (!m_frequency) {
                m_frequency = m_minFrequency;
                emit frequencyChanged(m_frequency);
            }
        } else if (verb == "frequency_step") {
            m_frequencyStep = data.value("step").toInt();
            emit frequencyStepChanged(m_frequencyStep);
        }
    }
}
