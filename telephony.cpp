/*
 * Copyright (C) 2017 Konsulko Group
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
#include "telephony.h"
#include "telephonymessage.h"

Telephony::Telephony (QUrl &url, QObject * parent) :
	QObject(parent),
	m_connected(false),
	m_mloop(nullptr),
	m_call_state("disconnected")
{
	m_mloop = new MessageEngine(url);
	QObject::connect(m_mloop, &MessageEngine::connected, this, &Telephony::onConnected);
	QObject::connect(m_mloop, &MessageEngine::disconnected, this, &Telephony::onDisconnected);
	QObject::connect(m_mloop, &MessageEngine::messageReceived, this, &Telephony::onMessageReceived);
}

Telephony::~Telephony()
{
	delete m_mloop;
}

void Telephony::dial(QString number)
{
	TelephonyMessage *tmsg = new TelephonyMessage();
	tmsg->createRequest("dial", number);
	m_mloop->sendMessage(tmsg);
	tmsg->deleteLater();
}

void Telephony::answer()
{
	TelephonyMessage *tmsg = new TelephonyMessage();
	tmsg->createRequest("answer");
	m_mloop->sendMessage(tmsg);
	tmsg->deleteLater();
}

void Telephony::hangup()
{
	TelephonyMessage *tmsg = new TelephonyMessage();
	tmsg->createRequest("hangup");
	m_mloop->sendMessage(tmsg);
	tmsg->deleteLater();
}

void Telephony::onConnected()
{
	QStringList events {
		"callStateChanged",
		"dialingCall",
		"incomingCall",
		"terminatedCall"};
	QStringListIterator eventIterator(events);
	TelephonyMessage *tmsg;

	while (eventIterator.hasNext()) {
		tmsg = new TelephonyMessage();
		tmsg->createRequest("subscribe", eventIterator.next());
		m_mloop->sendMessage(tmsg);
		tmsg->deleteLater();
	}

	setConnected(true);
}

void Telephony::onDisconnected()
{
	setConnected(false);
}

void Telephony::onMessageReceived(MessageType type, Message *message)
{
	if (type == TelephonyEventMessage) {
		TelephonyMessage *tmsg = qobject_cast<TelephonyMessage*>(message);

		if (tmsg->isEvent()) {
			if (tmsg->isCallStateChanged()) {
				setCallState(tmsg->state());
			} else if (tmsg->isDialingCall()) {
				m_colp = tmsg->colp();
				setCallState("dialing");
			} else if (tmsg->isIncomingCall()) {
				m_clip = tmsg->clip();
				setCallState("incoming");
			} else if (tmsg->isTerminatedCall()) {
				setCallState("disconnected");
				m_colp = "";
				m_clip = "";
			}
		}
	}
	message->deleteLater();
}
