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
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "responsemessage.h"

//deprecated method call new constructor and  setAdditionalData() instead:
ResponseMessage::ResponseMessage(QByteArray request)
{

	QJsonDocument jdoc(QJsonDocument::fromJson(request));

	if (!jdoc.isArray()) {
		qWarning("Invalid appfw message: not an array");
		return;
	}

	QJsonArray msg = jdoc.array();

	if (msg.size() != 4) {
		qWarning("Invalid appfw message: invalid array size");
		return;
	}

	QStringList api_str_list = msg[2].toString().split(QRegExp("/"));

	m_request["msgid"] = msg.at(0);
	m_request["callid"] = msg.at(1);
	m_request["api"] = api_str_list[0];
	m_request["verb"] = api_str_list[1];
	m_request["parameter"] = msg.at(3);
}

ResponseMessage::ResponseMessage(QJsonDocument content)
{
	QJsonArray msg = content.array();
	if (!msg[2].isObject()) {
		qWarning("Invalid appfw payload: no JSON object");
		return;
	}

	//deserialize:
	auto callid = msg[1].toString().toInt();
	QJsonObject payload = msg[2].toObject();

	auto request_iter = payload.find("request");
	auto request = request_iter.value().toObject();
	if (request.empty()) {
		qWarning("Invalid appfw reply message: empty request data");
		return;
	}

	auto status_iter = request.find("status");
	auto info_iter = request.find("info");
	auto response_iter = payload.find("response");
	auto response = response_iter.value().toObject();
	m_reply_status = status_iter.value().toString();
	m_reply_info = info_iter.value().toString();
	m_reply_data = response;
	m_reply_callid = callid;
	m_init = false; //not complete yet, missing matching request data
}

bool ResponseMessage::setAdditionalData(QByteArray data)
{
	QJsonDocument jdoc(QJsonDocument::fromJson(data));
	if (!jdoc.isArray()) {
		qWarning("Invalid data: not an array");
		return false;
	}

	QJsonArray content = jdoc.array();
	if (content.size() != 4) {
		qWarning("Invalid data: invalid array size");
	return false;
	}

	QStringList api_str_list = content[2].toString().split(QRegExp("/"));
	m_request["msgid"] = content.at(0);
	m_request["callid"] = content.at(1);
	m_request["api"] = api_str_list[0];
	m_request["verb"] = api_str_list[1];
	m_request["parameter"] = content.at(3);
	m_init = true;
	return true;
}

bool ResponseMessage::copyCallId(unsigned int *id)
{
	*id = m_reply_callid;
	return true;
}

QByteArray ResponseMessage::serialize(QJsonDocument::JsonFormat format)
{
	QJsonArray array;
	(m_reply_status == "failed")?
		array.append(static_cast<int>(MessageId::RetErr)) :
		array.append(static_cast<int>(MessageId::RetOk));
	array.append(static_cast<int>(m_reply_callid));
	array.append(m_request["api"].toString() + "/" + m_request["verb"].toString());
	array.append(m_reply_data);

	QJsonDocument jdoc;
	jdoc.setArray(array);

	return jdoc.toJson(format).data();
}
