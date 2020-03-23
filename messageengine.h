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

#ifndef MESSAGEENGINE_H
#define MESSAGEENGINE_H

#include <memory>
#include <atomic>
#include <QUrl>
#include <QWebSocket>

#include "message.h"

class MessageEngine : public QObject
{
	Q_OBJECT
	public:
		explicit MessageEngine(const QUrl &url, QObject *parent = Q_NULLPTR);
		bool sendMessage(std::unique_ptr<Message> message);

	Q_SIGNALS:
		void disconnected();
		void connected();
		void messageReceived(std::shared_ptr<Message> message);

	private Q_SLOTS:
		void onConnected();
		void onDisconnected();
		void onTextMessageReceived(QString message);

	private:
		QWebSocket m_websocket;
		QMap<qint32, QByteArray> m_calls;
		QUrl m_url;
		std::atomic<unsigned int> m_callid;
};

#endif // MESSAGEENGINE_H
