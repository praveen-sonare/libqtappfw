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

#include "voiceagentmodel.h"
#include "voiceagentprofile.h"
#include <QVector>
#include <QDebug>

VoiceAgentModel::VoiceAgentModel(QObject *parent)
	: QAbstractListModel(parent)
{
}

QVariant VoiceAgentModel::data(const QModelIndex &index, int role) const
{
	QVariant ret;

	if (!index.isValid())
		return ret;

	if (index.row() < 0 || index.row() >= m_agents.count())
		return ret;

	const VoiceAgentProfile *vap = m_agents[index.row()];
	switch (role) {
		case IdRole:
			return vap->vaid();
		case NameRole:
			return vap->name();
		case WuwRole:
			return vap->activewuw();
		case AuthStateRole:
			return vap->authstate();
		case ConnStateRole:
			return vap->connstate();
		case DialogStateRole:
			return vap->dialogstate();
		case LoginParamsRole:
			return readLoginParams(index);
		case ActiveRole:
			return vap->isactive()? "active" : "inactive";
		case VendorRole:
			return vap->vendor();
	}
	return ret;
}

int VoiceAgentModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return m_agents.count();
}

QVariantList VoiceAgentModel::readLoginParams(const QModelIndex &index) const
{
	QVariantList ret;

	if (!index.isValid())
		return ret;

	if (index.row() < 0 || index.row() >= this->m_agents.count())
		return ret;

	const VoiceAgentProfile *vap = this->m_agents[index.row()];
	ret.append(vap->logincode());
	ret.append(vap->loginurl());
	ret.append(vap->isloginpairexpired()? "expired" : "valid");
	return ret;
}

void VoiceAgentModel::addAgent(VoiceAgentProfile *vap)
{
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	m_agents.insert(rowCount(), vap);
	endInsertRows();
}

void VoiceAgentModel::removeAgent(VoiceAgentProfile *vap)
{
	if (m_agents.isEmpty())
		return;

	int row = m_agents.indexOf(vap);
	beginRemoveRows(QModelIndex(), row, row);
	m_agents.removeAt(row);
	endRemoveRows();
	delete vap;
}

void VoiceAgentModel::removeAll()
{
	if (m_agents.isEmpty())
		return;

	beginRemoveRows(QModelIndex(), 0, m_agents.count() -1);
	qDeleteAll(m_agents.begin(), m_agents.end());
	endRemoveRows();
	m_agents.clear();
}

bool VoiceAgentModel::agentExists(QString name, QString id, QString api) const
{
	VoiceAgentProfile *vap = getAgentFromName(name);
	if (!vap)
		return false;
	bool sameid = id == vap->vaid();
	bool sameapi = api == vap->vaapi();
	return sameapi && (sameid || id != "UNKNOWN");
}

VoiceAgentProfile* VoiceAgentModel::getAgentFromName(QString name) const
{
	if (m_agents.isEmpty())
		return nullptr;

	for (auto agent : m_agents) {
		if (agent->name() == name)
			return agent;
	}
	return nullptr;
}

VoiceAgentProfile* VoiceAgentModel::getAgentFromId(QString id) const
{
	if (m_agents.isEmpty())
		return nullptr;

	for (auto agent : m_agents) {
		if (agent->vaid() == id)
			return agent;
	}
	return nullptr;
}

void VoiceAgentModel::updateAgentProperties(QString name, QString id, QString api,
					    bool active, QString wuw)
{
	QVector<int> vroles;
	VoiceAgentProfile *vap = getAgentFromName(name);
	if (!vap) {
		qWarning() << "Unknown agent";
		return;
	}
	if ((vap->vaapi() == api) && (vap->vaid() != id) && (id != "UNKNOWN")) {
		vap->setVaid(id);
		vroles.push_back(IdRole);
	}
	vap->setActive(active);
	vroles.push_back(ActiveRole);
	if (!wuw.isEmpty()) {
		vap->setWuw(wuw);
		vroles.push_back(WuwRole);
	}
	if (!vroles.isEmpty())
	emit dataChanged(indexOf(vap), indexOf(vap), vroles);
}

void VoiceAgentModel::updateAgentState(QString id)
{
	QVector<int> vroles;
	VoiceAgentProfile *vap = getAgentFromId(id);

	if (!vap) {
		qWarning() << "Unknown agent";
		return;
	}

	vroles.push_back(AuthStateRole);
	vroles.push_back(ConnStateRole);
	vroles.push_back(DialogStateRole);

	if (!vroles.isEmpty())
		emit dataChanged(indexOf(vap), indexOf(vap), vroles);
}

void VoiceAgentModel::updateAgentLoginData(QString id)
{
	QVector<int> vroles;
	VoiceAgentProfile *vap = getAgentFromId(id);

	if (!vap) {
		qWarning() << "Unknown agent";
		return;
	}

	vroles.push_back(LoginParamsRole);
	if (!vroles.isEmpty())
		emit dataChanged(indexOf(vap), indexOf(vap), vroles);
}

QModelIndex VoiceAgentModel::indexOf(VoiceAgentProfile *vap)
{
	int row = m_agents.indexOf(vap);
	return index(row);
}

QHash<int, QByteArray> VoiceAgentModel::roleNames() const
{
	QHash<int, QByteArray> roles;
	roles[NameRole] = "name";
	roles[IdRole] = "id";
	roles[WuwRole] = "wuw";
	roles[AuthStateRole] = "authstate";
	roles[ConnStateRole] = "connstate";
	roles[DialogStateRole] = "dialogstate";
	roles[LoginParamsRole] = "usrauth";
	roles[ActiveRole] = "active";
	roles[VendorRole] = "vendor";
	return roles;
}
