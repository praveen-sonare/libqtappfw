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

#ifndef CALLMESSAGE_H
#define CALLMESSAGE_H

#include "message.h"


class CallMessage : public Message
{
	public:
		bool createRequest(QString api, QString verb, QJsonValue parameter = "None");

		bool isEvent() override
		{
			return false;
		}

		bool isReply() override
		{
			return false;
		}

		void updateCallId(unsigned int id) override
		{
			m_request["callid"] = qint32(id);
		}

		QByteArray serialize(QJsonDocument::JsonFormat format = QJsonDocument::Compact) override;

	private:
		QMap<QString, QVariant> m_request;

		CallMessage() = default;
		friend class MessageFactory;
};

#endif // CALLMESSAGE_H
