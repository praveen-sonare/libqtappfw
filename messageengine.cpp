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
#include "telephonymessage.h"

#include <QJsonArray>

MessageEngine::MessageEngine(const QUrl &url, QObject *parent) :
	QObject(parent),
	m_url(url)
{
	connect(&m_websocket, &QWebSocket::connected, this, &MessageEngine::onConnected);
	connect(&m_websocket, &QWebSocket::disconnected, this, &MessageEngine::onDisconnected);

	m_websocket.open(url);
}

bool MessageEngine::sendMessage(Message *message)
{
	if (!message->isValid())
		return false;

	qint64 size = m_websocket.sendTextMessage(message->toJson().data());
	if (size == 0)
		return false;

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
	m_websocket.deleteLater();
	emit disconnected();
}

void MessageEngine::onTextMessageReceived(QString jsonStr)
{
	QJsonDocument jdoc(QJsonDocument::fromJson(jsonStr.toUtf8()));
	if (jdoc.isEmpty()) {
		qWarning() << "Received invalid JSON: empty appfw message";
		return;
	}

	QJsonArray msg = jdoc.array();
	QStringList api_str_list = msg[1].toString().split(QRegExp("/"));
	QString api = api_str_list[0];

	Message *message;
	MessageType type;
	// FIXME: This should be rewritten using a factory class with a
	// parser parameter to remove API specific handling here
	if (api == "telephony") {
		message = new TelephonyMessage;
		type = TelephonyEventMessage;
	} else {
		message = new Message;
		type = GenericMessage;
	}

	if (message->fromJDoc(jdoc) == false) {
		delete message;
		return;
	}

	emit messageReceived(type, message);
}
