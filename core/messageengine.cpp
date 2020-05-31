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

#include <QJsonArray>
#include <QDebug>

#include "message.h"
#include "messagefactory.h"
#include "messageengine.h"


MessageEngine::MessageEngine(const QUrl &url) :
	QObject(Q_NULLPTR),
	m_callid(0),
	m_url(url)
{
	connect(&m_websocket, &QWebSocket::connected, this, &MessageEngine::onConnected);
	connect(&m_websocket, &QWebSocket::disconnected, this, &MessageEngine::onDisconnected);

	m_websocket.open(url);
}

bool MessageEngine::sendMessage(std::unique_ptr<Message> msg)
{
	if (!msg)
		return false;

	unsigned int callid = m_callid++;
	QByteArray forkeeps = msg->send(m_websocket, callid);
	if (forkeeps.isEmpty())
		return false;

	std::lock_guard<std::mutex> localguard(m_mutex);
	m_calls.insert(callid, forkeeps);

	return true;
}

void MessageEngine::onConnected()
{
	connect(&m_websocket, &QWebSocket::textMessageReceived, this, &MessageEngine::onTextMessageReceived);
	emit connected();
}

void MessageEngine::onDisconnected()
{
	disconnect(&m_websocket, &QWebSocket::textMessageReceived, this, &MessageEngine::onTextMessageReceived);
	emit disconnected();
}

void MessageEngine::onTextMessageReceived(QString jsonStr)
{
	jsonStr = jsonStr.simplified();
	QJsonDocument jdoc(QJsonDocument::fromJson(jsonStr.toUtf8()));
	if (jdoc.isEmpty()) {
		qWarning() << "Received invalid JSON: empty appfw message";
		return;
	}

	MessageId id = Message::isValid(jdoc);
	if (id == MessageId::Invalid) {
		qWarning() << "Received unknown message, discarding";
		return;
	}

	std::shared_ptr<Message> message = MessageFactory::getInstance().createInboundMessage(id, jdoc);

	unsigned int callid;
	if (message->isReply() && message->getCallId(&callid)) {
		message->setAdditionalData(m_calls[callid]);
		std::lock_guard<std::mutex> localguard(m_mutex);
		m_calls.remove(callid);
	}

	if (message->isComplete())
		emit messageReceived(message);
}
