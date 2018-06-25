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

#ifndef TELEPHONY_H
#define TELEPHONY_H

#include <QDebug>
#include <QObject>

#include "messageengine.h"

class Telephony : public QObject
{
	Q_OBJECT
	Q_PROPERTY(bool connected READ connected WRITE setConnected NOTIFY connectedChanged)
	Q_PROPERTY(QString callState READ callState WRITE setCallState NOTIFY callStateChanged)
	Q_PROPERTY(QString callClip READ callClip)
	Q_PROPERTY(QString callColp READ callColp)

	public:
		explicit Telephony(QUrl &url, QObject * parent = Q_NULLPTR);
		virtual ~Telephony();
		Q_INVOKABLE void dial(QString number);
		Q_INVOKABLE void answer();
		Q_INVOKABLE void hangup();

		void setConnected(bool state)
		{
			m_connected = state;
			emit connectedChanged(state);
		}

		void setCallState(QString callState)
		{
			m_call_state = callState;
			emit callStateChanged(m_call_state);
		}

		bool connected() { return m_connected; }
		QString callState() { return m_call_state; }
		QString callClip() { return m_clip; }
		QString callColp() { return m_colp; }

	signals:
		void connectedChanged(bool);
		void callStateChanged(QString);

	private:
		bool m_connected;
		MessageEngine *m_mloop;
		QString m_call_state;
		QString m_clip;
		QString m_colp;
		void onConnected();
		void onDisconnected();
		void onMessageReceived(MessageType, Message*);
};

#endif // TELEPHONY_H
