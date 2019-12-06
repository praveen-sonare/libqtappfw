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
#include "signalcomposer.h"
#include "signalcomposermessage.h"

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
    SignalComposerMessage *tmsg;

    while (eventIterator.hasNext()) {
        tmsg = new SignalComposerMessage();
        QJsonObject parameter;
        parameter.insert("signal", eventIterator.next());
        tmsg->createRequest("subscribe", parameter);
        m_mloop->sendMessage(tmsg);
        delete tmsg;
    }
}

void SignalComposer::onDisconnected()
{
    QStringListIterator eventIterator(events);
    SignalComposerMessage *tmsg;

    while (eventIterator.hasNext()) {
        tmsg = new SignalComposerMessage();
        QJsonObject parameter;
        parameter.insert("signal", eventIterator.next());
        tmsg->createRequest("unsubscribe", parameter);
        m_mloop->sendMessage(tmsg);
        delete tmsg;
    }
}

void SignalComposer::onMessageReceived(MessageType type, Message *message)
{
    if (type == SignalComposerEventMessage) {
        SignalComposerMessage *tmsg = qobject_cast<SignalComposerMessage*>(message);

        if (tmsg->isEvent()) {
            QString uid = tmsg->eventData().value("uid").toString();
            QVariant v = tmsg->eventData().value("value").toVariant();
            QString value;
            if(v.canConvert(QMetaType::QString))
                value = v.toString();
            else
                qWarning() << "Unconvertible value type for uid " << uid;
            QString units = tmsg->eventData().value("unit").toString();
	    v = tmsg->eventData().value("timestamp").toVariant();
            quint64 timestamp = 0;
            if(v.canConvert(QMetaType::ULongLong))
                timestamp = v.toULongLong();
            else
                qWarning() << "Unconvertible timestamp type for uid " << uid;

            emit signalEvent(uid, value, units, timestamp);
        }
    }
    message->deleteLater();
}
