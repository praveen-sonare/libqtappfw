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
		virtual bool isAuthStateEvent() const = 0;
		virtual bool isConnectionStateEvent() const = 0;
		virtual bool isDialogStateEvent() const = 0;
		virtual bool createRequest(QString verb, QJsonObject parameter) = 0;
};

class VshlCoreVoiceMessage : public VoiceMessage
{
	Q_OBJECT
	public:
		virtual ~VshlCoreVoiceMessage() {};
		bool isAuthStateEvent() const override {
			return (this->eventName() == "voice_authstate_event"); };
		bool isConnectionStateEvent() const override {
			return (this->eventName() == "voice_connectionstate_event"); };
		bool isDialogStateEvent() const override {
			return (this->eventName() == "voice_dialogstate_event"); };
		bool createRequest(QString verb, QJsonObject parameter) override;

	private:
		QStringList verbs {
			"startListening",
			"cancelListening",
			"subscribe",
			"unsubscribe",
			"enumerateVoiceAgents",
			"setDefaultVoiceAgent",
		};
		QStringList events {
			"",
		};
};

class VshlCpbltsVoiceMessage : public VoiceMessage
{
	Q_OBJECT
	public:
		virtual ~VshlCpbltsVoiceMessage() {};
		bool isAuthStateEvent() const override { return false; };
		bool isConnectionStateEvent() const override { return false; };
		bool isDialogStateEvent() const override { return false; };
		bool createRequest(QString verb, QJsonObject parameter) override;

	private:
		QStringList verbs {
			"guiMetadataSubscribe",
			"guiMetadataPublish",
			"phonecontrolSubscribe",
			"phonecontrolPublish",
			"navigationSubscribe",
			"navigationPublish",
			"playbackControllerSubscribe",
			"playbackControllerPublish",
		};
		QStringList events {
			"voice_authstate_event",
			"voice_dialogstate_event",
			"voice_connectionstate_event",
		};
};

/* We shouldnt access an agent directly, but CBL events
 * are not abstracted/forwarded by vshl bindings.
 */
class AlexaVoiceMessage : public VoiceMessage
{
	Q_OBJECT
	public:
		virtual ~AlexaVoiceMessage() {};
		bool isAuthStateEvent() const override {
			return (!events.contains(this->eventName())); };
		bool isConnectionStateEvent() const override { return false; };
		bool isDialogStateEvent() const override { return false; };
		bool createRequest(QString verb, QJsonObject parameter) override;

	private:
		QStringList verbs {
			"subscribeToCBLEvents",
		};
		QStringList events {
			"voice_cbl_codepair_received_event",
			"voice_cbl_codepair_expired_event",
		};
};

#endif // VOICE_MESSAGE_H
