/*
 * Copyright (C) 2017-2020 Konsulko Group
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
#include "telephony.h"


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
	std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
	if (!msg)
		return;

	CallMessage *tmsg = static_cast<CallMessage*>(msg.get());
	tmsg->createRequest("telephony", "dial", number);
	m_mloop->sendMessage(std::move(msg));
}

void Telephony::answer()
{
	std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
	if (!msg)
		return;

	CallMessage *tmsg = static_cast<CallMessage*>(msg.get());
	tmsg->createRequest("telephony", "answer");
	m_mloop->sendMessage(std::move(msg));
}

void Telephony::hangup()
{
	std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
	if (!msg)
		return;

	CallMessage *tmsg = static_cast<CallMessage*>(msg.get());
	tmsg->createRequest("telephony", "hangup");
	m_mloop->sendMessage(std::move(msg));
}

void Telephony::onConnected()
{
	QStringList events {
		"callStateChanged",
		"dialingCall",
		"incomingCall",
		"terminatedCall",
		"online"};
	QStringListIterator eventIterator(events);

	while (eventIterator.hasNext()) {
		std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
		if (!msg)
			return;

		CallMessage *tmsg = static_cast<CallMessage*>(msg.get());
		tmsg->createRequest("telephony", "subscribe", eventIterator.next());
		m_mloop->sendMessage(std::move(msg));
	}

	setConnected(true);
}

void Telephony::onDisconnected()
{
	setConnected(false);
}

void Telephony::onMessageReceived(std::shared_ptr<Message> msg)
{
	if (!msg)
		return;

	if (msg->isEvent()) {
		std::shared_ptr<EventMessage> emsg = std::static_pointer_cast<EventMessage>(msg);
		if (emsg->eventApi() != "telephony");
			return;
		QString ename = emsg->eventName();
		QJsonObject data = emsg->eventData();
		if (ename == "callStateChanged") {
			setCallState(data.find("state").value().toString());
		} else if (ename == "dialingCall") {
			m_colp = data.find("colp").value().toString();
				setCallState("dialing");
		} else if (ename == "incomingCall") {
			m_clip = data.find("clip").value().toString();
				setCallState("incoming");
		} else if (ename == "terminatedCall") {
				setCallState("disconnected");
				m_colp = "";
				m_clip = "";
		} else if (ename == "online") {
			setOnlineState(data.find("connected").value().toBool());
			}
		}
}
