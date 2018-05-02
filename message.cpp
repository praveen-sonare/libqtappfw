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

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "message.h"

Message::Message()
	: m_init(false), m_event(false), m_reply(false)
{
}

bool Message::createRequest(QString api, QString verb, QJsonValue parameter)
{
	if (!m_request.isEmpty()){
		qWarning("Message instance has already been used. Cannot send another request.");
		return false;
	}

	m_request["msgid"] = Call;
	m_request["callid"] = 0;
	m_request["api"] = api;
	m_request["verb"] = verb;
	m_request["parameter"] = parameter;

	m_init = true;

	return true;
}

bool Message::fromJson(QByteArray jsonData)
{
	QJsonDocument jdoc(QJsonDocument::fromJson(jsonData));

	if (jdoc.isNull()) {
		qWarning("Imported invalid JSON: empty appfw message");
		return false;
	}

	return Message::fromJDoc(jdoc);
}

bool Message::fromJDoc(QJsonDocument jdoc)
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
		m_event_data = data_iter.value().toObject();

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

QByteArray Message::toJson(QJsonDocument::JsonFormat format)
{
	QJsonArray array;
	array.append(m_request["msgid"].toInt());
	array.append(m_request["callid"].toInt());
	array.append(m_request["api"].toString() + "/" + m_request["verb"].toString());
	array.append(m_request["parameter"].toJsonValue());

	QJsonDocument jdoc;
	jdoc.setArray(array);

	return jdoc.toJson(format).data();
}
