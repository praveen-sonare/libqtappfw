/*
 * Copyright (C) 2019 Konsulko Group
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

#include "voicemessage.h"

bool VoiceMessage::createRequest(QString verb, QJsonObject parameter)
{
	if (!verbs.contains(verb))
		return false;
	return Message::createRequest("vshl-core", verb, parameter);
}

bool VoiceMessage::fromJDoc(QJsonDocument jdoc)
{
	// Validate message is array
	if (!jdoc.isArray()) {
		qWarning("Invalid appfw message: not an array");
		return false;
	}
	QJsonArray msg = jdoc.array();

	// Validate array is proper length
	if ((msg.size() < 3) || (msg.size() > 4)) {
		qWarning("Invalid appfw message: invalid array size");
		return false;
	}

	// Validate msgid type
	double msgid;
	if (msg[0].isDouble()) {
		msgid = msg[0].toDouble();
	} else {
		qWarning("Invalid appfw message: invalid msgid type");
		return false;
	}

	// Validate msgid element
	if ((msgid < Call) || (msgid > Event)) {
		qWarning("Invalid appfw message: msgid out of range");
		return false;
	}

	// Validate that the payload has a request object
	QJsonObject payload;
	if (msg[2].isObject()) {
		payload = msg[2].toObject();
	} else {
		qWarning("Invalid appfw payload: no JSON object");
		return false;
	}

	if ((msgid == RetOk) || (msgid == RetErr)) {
		auto request_iter = payload.find("request");
		auto request = request_iter.value().toObject();
		if (request.empty()) {
			qWarning("Invalid appfw reply message: empty request data");
			return false;
		}
		auto status_iter = request.find("status");
		auto info_iter = request.find("info");
		auto response_iter = payload.find("response");
		auto response = response_iter.value().toObject();
		m_reply_status = status_iter.value().toString();
		m_reply_info = info_iter.value().toString();
		m_reply_data = response;
		m_reply = true;
	} else if (msgid == Event) {
		// If event, save data object
		auto data_iter = payload.find("data");
		auto data = data_iter.value().toObject();
		auto data_string = data_iter.value().toString();
		if (!data_string.isEmpty())
			data_string.remove('\n');

		QJsonDocument datadoc = QJsonDocument::fromJson(data_string.toUtf8());
		m_event_data = datadoc.object();

		auto event_iter = payload.find("event");
		auto event_string = event_iter.value().toString();
		if (event_string.isEmpty()) {
			qWarning("Invalid appfw event message: empty event name");
			return false;
		}
		QStringList event_strings = event_string.split(QRegExp("/"));
		if (event_strings.size() != 2) {
			qWarning("Invalid appfw event message: malformed event name");
			return false;
		}
		m_event_api = event_strings[0];
		m_event_name = event_strings[1];
		m_event = true;
	}

	m_jdoc = jdoc;
	m_init = true;
	return m_init;
}
