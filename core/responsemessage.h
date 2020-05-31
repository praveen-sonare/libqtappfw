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

#ifndef RESPONSEMESSAGE_H
#define RESPONSEMESSAGE_H


#include "message.h"

class ResponseMessage : public Message
{


	public:
		inline QString requestApi() const
		{
			return m_request["api"].toString();
		}

		inline QString requestVerb() const
		{
			return m_request["verb"].toString();
		}

		inline QVariantMap requestParameters() const
		{
			return m_request["parameter"].toMap();
		}

		inline QString replyStatus() const
		{
			return m_reply_status;
		}

		inline QString replyInfo() const
		{
			return m_reply_info;
		}

		inline QJsonObject replyData() const
		{
			return m_reply_data;
		}

		bool getCallId(unsigned int *id) const override
		{
			*id = m_reply_callid;
			return true;
		}
		bool isEvent() override
		{
			return false;
		}

		bool isReply() override
		{
			return true;
		}

		bool setAdditionalData(QByteArray data) override;
		bool copyCallId(unsigned int *id);

		QByteArray serialize(QJsonDocument::JsonFormat format = QJsonDocument::Compact) override;

	private:
		QString m_reply_info, m_reply_status, m_reply_uuid;
		unsigned int m_reply_callid;
		QJsonObject m_reply_data;
		QMap<QString, QVariant> m_request;

		explicit ResponseMessage(QJsonDocument data);
		friend class MessageFactory;
};

#endif // RESPONSEMESSAGE_H
