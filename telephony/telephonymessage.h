/*
 * Copyright (C) 2017 Konsulko Group
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

#ifndef TELEPHONY_MESSAGE_H
#define TELEPHONY_MESSAGE_H

#include "message.h"

class TelephonyMessage : public Message
{
	Q_OBJECT
	public:
		bool isCallStateChanged() { return (this->eventName() == "callStateChanged"); };
		bool isDialingCall() { return m_event ? (this->eventName() == "dialingCall") : false; };
		bool isIncomingCall() { return m_event ? (this->eventName() == "incomingCall") : false; };
		bool isTerminatedCall() { return (this->eventName() == "terminatedCall"); };
		bool isOnline() { return (this->eventName() == "online"); };
		QString clip() { return m_event_data.find("clip").value().toString(); };
		QString colp() { return m_event_data.find("colp").value().toString(); };
		QString state() { return m_event_data.find("state").value().toString(); };
		bool connected() { return m_event_data.find("connected").value().toBool(); };
		bool createRequest(QString verb, QString value = "None");
};

#endif // TELEPHONY_MESSAGE_H
