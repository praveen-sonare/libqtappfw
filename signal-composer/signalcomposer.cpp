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
#include "messagefactory.h"
#include "messageengine.h"
#include "signalcomposer.h"


SignalComposer::SignalComposer (QUrl &url, QObject * parent) :
    QObject(parent),
    m_mloop(nullptr)
{
    m_mloop = new MessageEngine(url);
    QObject::connect(m_mloop, &MessageEngine::connected, this, &SignalComposer::onConnected);
    QObject::connect(m_mloop, &MessageEngine::disconnected, this, &SignalComposer::onDisconnected);
    QObject::connect(m_mloop, &MessageEngine::messageReceived, this, &SignalComposer::onMessageReceived);
}

SignalComposer::~SignalComposer()
{
    delete m_mloop;
}

void SignalComposer::onConnected()
{
    QStringListIterator eventIterator(events);

    while (eventIterator.hasNext()) {
        std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
        if (!msg)
            return;

        CallMessage* tmsg = static_cast<CallMessage*>(msg.get());
        QJsonObject parameter;
        parameter.insert("signal", eventIterator.next());
        tmsg->createRequest("signal-composer", "subscribe", parameter);
        m_mloop->sendMessage(std::move(msg));
    }
}

void SignalComposer::onDisconnected()
{
    QStringListIterator eventIterator(events);

    while (eventIterator.hasNext()) {
        std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
        if (!msg)
            return;

        CallMessage* tmsg = static_cast<CallMessage*>(msg.get());
        QJsonObject parameter;
        parameter.insert("signal", eventIterator.next());
        tmsg->createRequest("signal-composer", "unsubscribe", parameter);
        m_mloop->sendMessage(std::move(msg));
    }
}

void SignalComposer::onMessageReceived(std::shared_ptr<Message> msg)
{
    if (!msg)
        return;

    if (msg->isEvent()) {
        std::shared_ptr<EventMessage> emsg = std::static_pointer_cast<EventMessage>(msg);
        if (emsg->eventApi() != "signal-composer")
            return;

        QJsonObject data = emsg->eventData();
        QString uid = data.value("uid").toString();
        QVariant v = data.value("value").toVariant();
        QString value;
        if(v.canConvert(QMetaType::QString))
            value = v.toString();
        else
            qWarning() << "Unconvertible value type for uid " << uid;
        QString units = data.value("unit").toString();
        v = data.value("timestamp").toVariant();
        quint64 timestamp = 0;
        if(v.canConvert(QMetaType::ULongLong))
            timestamp = v.toULongLong();
        else
            qWarning() << "Unconvertible timestamp type for uid " << uid;

        emit signalEvent(uid, value, units, timestamp);
    }
}
