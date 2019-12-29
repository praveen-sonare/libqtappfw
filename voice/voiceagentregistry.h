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

#ifndef VOICEAGENTREGISTRY_H
#define VOICEAGENTREGISTRY_H

#include <QDebug>
#include <QObject>
#include <QJsonArray>
#include <QtQml/QQmlContext>

class Voice;
class VoiceAgentModel;

class VoiceAgentRegistry : public QObject
{
	Q_OBJECT
	public:
		explicit VoiceAgentRegistry(Voice *voice, QQmlContext *context,
					    QObject *parent);
		virtual ~VoiceAgentRegistry();

		enum AgentConnectionState {
			DISCONNECTED = 0,
			CONNECTED,
		};
		Q_ENUM(AgentConnectionState)

		enum VoiceDialogState {
			IDLE = 0,
			LISTENING,
			THINKING,
			SPEAKING,
			MICROPHONEOFF,
		};
		Q_ENUM(VoiceDialogState)

		enum ServiceAuthState {
			UNINITIALIZED = 0,
			REFRESHED,
			EXPIRED,
			UNRECOVERABLE_ERROR
		};
		Q_ENUM(ServiceAuthState)

		QString addAgent(QJsonObject va);
		bool removeAgent(QString id);
		void clearRegistry();
		QStringList getAgentsListById() const;
		QString getDefaultId() const;
		void setDefaultId(QString id);
		void setAuthState(QString id, ServiceAuthState state);
		void setConnectionState(QString id, AgentConnectionState state);
		void setDialogState(QString id, VoiceDialogState state);
		void updateLoginData(QString id, QString code, QString url,
				     bool expired);
		int stringToEnum(QString value, QString enumtype);
	private:
		VoiceAgentModel *m_model;
		Voice *vc;
		QString m_default_aid;
		QStringList m_regids;
};

#endif // VOICEAGENTREGISTRY_H
