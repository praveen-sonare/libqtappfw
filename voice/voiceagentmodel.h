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

#ifndef VOICEAGENTMODEL_H
#define VOICEAGENTMODEL_H

#include <QAbstractListModel>
#include <QStringList>
#include <QtQml/QQmlContext>
#include <QJsonObject>

#include "voiceagentprofile.h"

class VoiceAgentModel : public QAbstractListModel
{
	Q_OBJECT

	public:
		enum VoiceAgentRoles {
			IdRole = Qt::UserRole + 1,
			NameRole,
			WuwRole,
			AuthStateRole,
			ConnStateRole,
			DialogStateRole,
			LoginParamsRole,
			ActiveRole,
			VendorRole,
		};

		VoiceAgentModel(QObject *parent = Q_NULLPTR);

		QVariant data(const QModelIndex &index,
			      int role = Qt::DisplayRole) const;
		int rowCount(const QModelIndex &parent = QModelIndex()) const;
		QVariantList readLoginParams(const QModelIndex &index) const;
		void addAgent(VoiceAgentProfile *vap);
		void removeAgent(VoiceAgentProfile* vap);
		void removeAll();
		bool agentExists(QString name, QString id, QString api) const;
		VoiceAgentProfile *getAgentFromName(QString name) const;
		VoiceAgentProfile *getAgentFromId(QString id) const;
		void updateAgentProperties(QString name, QString id,
					   QString api, bool active, QString wuw);
		void updateAgentState(QString id);
		void updateAgentLoginData(QString id);

	private:
		QList<VoiceAgentProfile *> m_agents;
		QModelIndex indexOf(VoiceAgentProfile *agent);
		QHash<int, QByteArray> roleNames() const;
};
#endif // VOICEAGENTMODEL_H
