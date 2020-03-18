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

#ifndef EVENTMESSAGE_H
#define EVENTMESSAGE_H

#include "message.h"


class EventMessage : public Message
{
	public:
		explicit EventMessage(QJsonDocument data);

                inline QString eventApi() const
		{
			return m_event_api;
		}

		inline QString eventName() const
		{
			return m_event_name;
		}

		inline QJsonObject eventData() const
		{
			return m_event_data;
		}

		bool isEvent() override
		{
			return true;
		}

		bool isReply() override
		{
			return false;
		}

		QByteArray serialize(QJsonDocument::JsonFormat format = QJsonDocument::Compact) override;

	private:
		QString m_event_api, m_event_name;
		QJsonObject m_event_data;
};

#endif // EVENTMESSAGE_H
