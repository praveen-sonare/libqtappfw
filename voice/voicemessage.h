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

#ifndef VOICE_MESSAGE_H
#define VOICE_MESSAGE_H

#include "message.h"

class VoiceMessage : public Message
{
	Q_OBJECT
	public:
		virtual ~VoiceMessage() {};

		bool isAuthStateEvent() const {
			return (this->eventName().contains("voice_authstate_event")); };
		bool isConnectionStateEvent() const {
			return (this->eventName().contains("voice_connectionstate_event")); };
		bool isDialogStateEvent() const {
			return (this->eventName().contains("voice_dialogstate_event")); };
		bool isCblEvent() const {
			return (this->eventName().contains("cbl")); };
		bool createRequest(QString verb, QJsonObject parameter);

	private:
		QStringList verbs {
			"startListening",
			"cancelListening",
			"subscribe",
			"unsubscribe",
			"enumerateVoiceAgents",
			"setDefaultVoiceAgent",
			"subscribeToLoginEvents",
		};
		QStringList events {
			"voice_authstate_event",
			"voice_dialogstate_event",
			"voice_connectionstate_event",
			"voice_cbl_codepair_received_event",
			"voice_cbl_codepair_expired_event",
		};
};


#endif // VOICE_MESSAGE_H
