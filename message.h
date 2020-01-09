/*
 * Copyright (C) 2017, 2018, 2019 Konsulko Group
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

enum MessageId {
	Call = 2,
	RetOk = 3,
	RetErr = 4,
	Event = 5,
};

enum MessageType {
	GenericMessage,
	ResponseRequestMessage,
	TelephonyEventMessage,
	WeatherEventMessage,
	MediaplayerEventMessage,
	NetworkEventMessage,
	BluetoothEventMessage,
	PbapEventMessage,
	RadioEventMessage,
	MapEventMessage,
	NavigationEventMessage,
	VoiceEventMessage,
	SignalComposerEventMessage,
	GuiMetadataCapabilityEventMessage,
	HVACEventMessage,
};

class Message : public QObject
{
	Q_OBJECT
	Q_ENUM(MessageId)
	Q_ENUM(MessageType)

	public:
		Message();

		bool fromJson(QByteArray jsonData);
		virtual bool fromJDoc(QJsonDocument jdocData);
		QByteArray toJson(QJsonDocument::JsonFormat format = QJsonDocument::Compact);
		bool createRequest(QString api, QString verb, QJsonValue parameter = "None");
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

		inline bool isEvent() const
		{
			return m_event;
		}

		inline bool isReply() const
		{
			return m_reply;
		}

		inline bool isValid() const
		{
			return m_init;
		}

		inline void setCallId(qint32 callId) {
			m_request["callid"] = callId;
		}

		inline QMap<QString, QVariant> requestData() const
		{
			return m_request;
		}

	protected:
		bool m_event, m_init, m_reply;
		QString m_event_api, m_event_name, m_reply_info, m_reply_status, m_reply_uuid;
		QMap<QString, QVariant> m_request;
		QJsonDocument m_jdoc;
		QJsonObject m_event_data, m_reply_data;
};

#endif // MESSAGE_H
