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
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "eventmessage.h"

EventMessage::EventMessage(QJsonDocument content): Message()
{
	QJsonArray msg = content.array();
	if (!msg[2].isObject()) {
		qWarning("Invalid appfw payload: no JSON object");
		return;
	}
	
	//deserialize:
	QJsonObject payload = msg[2].toObject();

	auto data_iter = payload.find("data");
	m_event_data = data_iter.value().toObject();

	auto event_iter = payload.find("event");
	auto event_string = event_iter.value().toString();
	if (event_string.isEmpty()) {
		qWarning("Invalid appfw event message: empty event name");
		return;
	}
	QStringList event_strings = event_string.split(QRegExp("/"));
	if (event_strings.size() != 2) {
		qWarning("Invalid appfw event message: malformed event name");
		return;
	}
	m_event_api = event_strings[0];
	m_event_name = event_strings[1];
	m_init = true;
}

QByteArray EventMessage::serialize(QJsonDocument::JsonFormat format)
{
	QJsonArray array;
	array.append(static_cast<int>(MessageId::Event));
	array.append(0); //unused field
	array.append(m_event_api + "/" + m_event_name);
	array.append(m_event_data);

	QJsonDocument jdoc;
	jdoc.setArray(array);

	return jdoc.toJson(format).data();
}
