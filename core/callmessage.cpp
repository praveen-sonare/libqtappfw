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
#include <QJsonValue>

#include "callmessage.h"
#include "message.h"


bool CallMessage::createRequest(QString api, QString verb, QJsonValue parameter)
{
	if (!m_request.isEmpty()){
		qWarning("Message instance has already been used. Cannot send another request.");
		return false;
	}

	m_request["msgid"] = static_cast<unsigned int>(MessageId::Call);
	m_request["callid"] = 0;
	m_request["api"] = api;
	m_request["verb"] = verb;
	m_request["parameter"] = parameter;

	m_init = true;

	return m_init;
}

QByteArray CallMessage::serialize(QJsonDocument::JsonFormat format)
{
	QJsonArray array;
	array.append(m_request["msgid"].toInt());
	array.append(m_request["callid"].toInt());
	array.append(m_request["api"].toString() + "/" + m_request["verb"].toString());
	array.append(m_request["parameter"].toJsonValue());

	m_jdoc.setArray(array);

	return m_jdoc.toJson(format).data();
}
