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

#include "message.h"
#include "messageengine.h"
#include "radio.h"
#include "radiomessage.h"
#include "responsemessage.h"

Radio::Radio (QUrl &url, QQmlContext *context, QObject * parent) :
    QObject(parent),
    m_mloop(nullptr),
    m_band(1),
    m_frequency(0),
    m_minFrequency(0),
    m_maxFrequency(0),
    m_playing(false),
    m_scanning(false)
{
    m_mloop = new MessageEngine(url);
    m_context = context;

    QObject::connect(m_mloop, &MessageEngine::connected, this, &Radio::onConnected);
    QObject::connect(m_mloop, &MessageEngine::disconnected, this, &Radio::onDisconnected);
    QObject::connect(m_mloop, &MessageEngine::messageReceived, this, &Radio::onMessageReceived);
}

Radio::~Radio()
{
    delete m_mloop;
}

void Radio::setBand(int band)
{
    RadioMessage *rmsg = new RadioMessage();
    QJsonObject parameter;

    parameter.insert("band", band ? "FM": "AM");
    rmsg->createRequest("band", parameter);
    m_mloop->sendMessage(rmsg);
    delete rmsg;
}

void Radio::setFrequency(int frequency)
{
    RadioMessage *rmsg = new RadioMessage();
    QJsonObject parameter;

    if (m_scanning)
        scanStop();

    if (frequency == m_frequency)
        return;

    parameter.insert("value", QString::number(frequency));
    rmsg->createRequest("frequency", parameter);
    m_mloop->sendMessage(rmsg);
    delete rmsg;

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
    RadioMessage *rmsg = new RadioMessage();
    QJsonObject parameter;

    rmsg->createRequest("start", parameter);
    m_mloop->sendMessage(rmsg);
    delete rmsg;
}

void Radio::stop()
{
    RadioMessage *rmsg = new RadioMessage();

    QJsonObject parameter;
    rmsg->createRequest("stop", parameter);
    m_mloop->sendMessage(rmsg);
    delete rmsg;
}

void Radio::scanForward()
{
    if (m_scanning)
        return;

    RadioMessage *rmsg = new RadioMessage();
    QJsonObject parameter;
    parameter.insert("direction", "forward");
    rmsg->createRequest("scan_start", parameter);
    m_mloop->sendMessage(rmsg);

    m_scanning = true;
    emit scanningChanged(m_scanning);

    delete rmsg;
}

void Radio::scanBackward()
{
    if (m_scanning)
        return;

    RadioMessage *rmsg = new RadioMessage();
    QJsonObject parameter;
    parameter.insert("direction", "backward");
    rmsg->createRequest("scan_start", parameter);
    m_mloop->sendMessage(rmsg);

    m_scanning = true;
    emit scanningChanged(m_scanning);

    delete rmsg;
}

void Radio::scanStop()
{
    RadioMessage *rmsg = new RadioMessage();

    QJsonObject parameter;
    rmsg->createRequest("scan_stop", parameter);
    m_mloop->sendMessage(rmsg);

    m_scanning = false;
    emit scanningChanged(m_scanning);

    delete rmsg;
}

void Radio::updateFrequencyBandParameters()
{
    RadioMessage *rmsg = new RadioMessage();

    QJsonObject parameter;
    parameter.insert("band", m_band ? "FM" : "AM");
    rmsg->createRequest("frequency_range", parameter);
    m_mloop->sendMessage(rmsg);
    delete rmsg;

    rmsg = new RadioMessage();
    rmsg->createRequest("frequency_step", parameter);
    m_mloop->sendMessage(rmsg);
    delete rmsg;
}

void Radio::onConnected()
{
    QStringListIterator eventIterator(events);
    RadioMessage *rmsg;

    while (eventIterator.hasNext()) {
        rmsg = new RadioMessage();
        QJsonObject parameter;
        parameter.insert("value", eventIterator.next());
        rmsg->createRequest("subscribe", parameter);
        m_mloop->sendMessage(rmsg);
        delete rmsg;
    }

    // Trigger initial update of frequency band parameters (min/max/step)
    updateFrequencyBandParameters();
}

void Radio::onDisconnected()
{
    QStringListIterator eventIterator(events);
    RadioMessage *rmsg;

    while (eventIterator.hasNext()) {
        rmsg = new RadioMessage();
        QJsonObject parameter;
        parameter.insert("value", eventIterator.next());
        rmsg->createRequest("unsubscribe", parameter);
        m_mloop->sendMessage(rmsg);
        delete rmsg;
    }
}

void Radio::onMessageReceived(MessageType type, Message *msg)
{
    if (msg->isEvent() && type == RadioEventMessage) {
        RadioMessage *rmsg = qobject_cast<RadioMessage*>(msg);

        if (rmsg->isFrequencyEvent()) {
            unsigned int frequency = rmsg->eventData().value("value").toInt();
            m_frequency = frequency;
            if (!m_scanning && (m_frequency != frequency)) {
                emit frequencyChanged(m_frequency);
            }
        } else if (rmsg->isStationFoundEvent()) {
            m_scanning = false;
            emit scanningChanged(m_scanning);

            m_frequency = rmsg->eventData().value("value").toInt();
            emit frequencyChanged(m_frequency);
        } else if (rmsg->isStatusEvent()) {
            if (rmsg->eventData().value("value") == QString("playing")) {
                m_playing = true;
                emit playingChanged(m_playing);
            } else if (rmsg->eventData().value("value") == QString("stopped")) {
                m_playing = false;
                emit playingChanged(m_playing);
            }
        }
    } else if (msg->isReply() && type == ResponseRequestMessage) {
        ResponseMessage *rmsg = qobject_cast<ResponseMessage*>(msg);

        if (rmsg->requestVerb() == "frequency_range") {
            m_minFrequency = rmsg->replyData().value("min").toInt();
            emit minFrequencyChanged(m_minFrequency);
            m_maxFrequency = rmsg->replyData().value("max").toInt();
            emit maxFrequencyChanged(m_maxFrequency);

            // Handle start up
            if (!m_frequency) {
                m_frequency = m_minFrequency;
                emit frequencyChanged(m_frequency);
            }
        } else if (rmsg->requestVerb() == "frequency_step") {
            m_frequencyStep = rmsg->replyData().value("step").toInt();
            emit frequencyStepChanged(m_frequencyStep);
        }
    }
    msg->deleteLater();
}
