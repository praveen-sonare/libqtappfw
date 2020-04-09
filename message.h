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

#ifndef MESSAGE_H
#define MESSAGE_H

#include <QObject>
#include <QMap>
#include <QVariant>
#include <QJsonDocument>
#include <QJsonObject>

enum class MessageId {
	Invalid = 0,
	Call = 2,
	RetOk = 3,
	RetErr = 4,
	Event = 5,
};

class QWebSocket;

class Message
{
	public:
		Message();
		virtual bool  setAdditionalData(QByteArray data)
		{
			return false;
		}
		QByteArray send(QWebSocket& transport, unsigned int callid);

		inline bool isComplete() const
		{
			return m_init;
		}

		virtual bool getCallId(unsigned int *id) const
		{
			return false;
		}

		static MessageId isValid(QJsonDocument );

		virtual bool isEvent() = 0;
		virtual bool isReply() = 0;

	protected:
		virtual void updateCallId(unsigned int id) {};
		virtual QByteArray serialize(QJsonDocument::JsonFormat format = QJsonDocument::Compact);

		bool m_init;
		QJsonDocument m_jdoc;
};

#endif // MESSAGE_H
