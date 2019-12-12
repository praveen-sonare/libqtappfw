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

#ifndef VOICE_H
#define VOICE_H

#include <QDebug>
#include <QObject>
#include <QJsonArray>
#include <QtQml/QQmlContext>
#include "message.h"

class MessageEngine;
class ResponseMessage;
class VoiceMessage;
class VoiceAgentRegistry;

class Voice : public QObject
{
	Q_OBJECT

	public:
		explicit Voice(QUrl &url, QQmlContext *context,
			       QObject * parent = Q_NULLPTR);
		virtual ~Voice();

		//enumerate agents:
		Q_INVOKABLE void scan();
		//obtain code based login params:
		Q_INVOKABLE void getCBLpair(QString id);

	private:
		MessageEngine *m_loop;
		VoiceAgentRegistry *m_var;

		void subscribeAgentToVshlEvents(QString id);
		void unsubscribeAgentFromVshlEvents(QString id);
		void triggerCBLProcess(QString id);
		void parseAgentsList(QJsonArray agents);
		void processVshlEvent(VoiceMessage *vmsg);
		void processLoginEvent(VoiceMessage *vmsg);

		void processEvent(VoiceMessage *vmsg);
		void processReply(ResponseMessage *rmsg);

		// slots
		void onConnected();
		void onDisconnected();
		void onMessageReceived(MessageType type, Message *msg);

		const QStringList vshl_events {
			"voice_authstate_event",
			"voice_dialogstate_event",
			"voice_connectionstate_event",
			"voice_cbl_codepair_received_event",
			"voice_cbl_codepair_expired_event",
		};
};

#endif // VOICE_H
