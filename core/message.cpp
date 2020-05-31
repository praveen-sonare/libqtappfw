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
#include <QJsonArray>
#include <QJsonDocument>
#include <QWebSocket>

#include "message.h"

MessageId Message::isValid(QJsonDocument candidate)
{
	MessageId  id = MessageId::Invalid;

	// Validate message is array
	if (!candidate.isArray()) {
		qWarning("Invalid appfw message: not an array");
		return id;
	}
	QJsonArray msg = candidate.array();

	// Validate array is proper length
	if ((msg.size() < 3) || (msg.size() > 4)) {
		qWarning("Invalid appfw message: invalid array size");
		return id;
	}

	// Validate msgid type
	double msgid;
	if (msg[0].isDouble()) {
		msgid = msg[0].toDouble();
	} else {
		qWarning("Invalid appfw message: invalid msgid type");
		return id;
	}

	// Validate msgid element
	if ((msgid >= static_cast<double>(MessageId::Call)) && (msgid <= static_cast<double>(MessageId::Event)))
		id = static_cast<MessageId>(msgid);

	return id;
}

Message::Message()
	: m_init(false)
{
}

QByteArray Message::serialize(QJsonDocument::JsonFormat format)
{
	QByteArray dummy;
	return dummy;
	}

QByteArray Message::send(QWebSocket& transport, unsigned int id)
{
	QByteArray blob;
	qint64 size = 0;
	updateCallId(id);
	if (m_init)
		blob = serialize().data();
	if (!blob.isEmpty())
		size = transport.sendTextMessage(blob);

	return blob;
}
