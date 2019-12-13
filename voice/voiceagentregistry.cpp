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

#include <QMetaEnum>
#include <QSortFilterProxyModel>
#include <QtQml/QQmlEngine>

#include "voice.h"
#include "voiceagentregistry.h"
#include "voiceagentmodel.h"
#include "voiceagentprofile.h"

VoiceAgentRegistry::VoiceAgentRegistry(Voice *voice, QQmlContext *context, QObject *parent) :
	QObject(parent),
	m_model(nullptr),
	vc(voice)
{
	m_model = new VoiceAgentModel();
	QSortFilterProxyModel *model = new QSortFilterProxyModel();
	model->setSourceModel(m_model);
	model->setSortRole(VoiceAgentModel::VoiceAgentRoles::IdRole);
	model->setSortCaseSensitivity(Qt::CaseInsensitive);
	model->sort(0);

	context->setContextProperty("VoiceAgentModel", m_model);
	context->setContextProperty("VoiceAgent", this);
}

VoiceAgentRegistry::~VoiceAgentRegistry()
{
	delete m_model;
}

QString VoiceAgentRegistry::addAgent(QJsonObject va)
{
	bool active = va.value("active").toBool();
	QString wuw = va.value("activewakeword").toString();
	QString api = va.value("api").toString();
	QString desc = va.value("description").toString();
	QString id = va.value("id").toString();
	QString name = va.value("name").toString();
	QString vendor = va.value("vendor").toString();
	QString wuws = va.value("wakewords").toString();

	if (!m_model->agentExists(name, id, api)) {
		VoiceAgentProfile *vap = new VoiceAgentProfile(name, id, api,
							       active, wuw,
							       vendor, wuws);
		m_model->addAgent(vap);
		m_regids.append(id);
	}
	else
		m_model->updateAgentProperties(name, id, api, active, wuw);
	return id;
}

bool VoiceAgentRegistry::removeAgent(QString id)
{
	VoiceAgentProfile *vap = m_model->getAgentFromId(id);
	if (!vap)
		return false;
	m_model->removeAgent(vap);
	return true;
}

void VoiceAgentRegistry::clearRegistry()
{
	m_default_aid.clear();
	m_regids.clear();
	m_model->removeAll();
}

QStringList VoiceAgentRegistry::getAgentsListById() const
{
	return m_regids;
}

QString VoiceAgentRegistry::getDefaultId() const
{
	return m_default_aid.isEmpty()? "UNKNOWN" : m_default_aid;
}
void VoiceAgentRegistry::setDefaultId(QString id)
{
	m_default_aid = id;
}

void VoiceAgentRegistry::setAuthState(QString id, ServiceAuthState state)
{
	QMetaEnum metaEnum = QMetaEnum::fromType<VoiceAgentRegistry::ServiceAuthState>();
	auto stateStr = metaEnum.valueToKey(state);
	VoiceAgentProfile *vap = m_model->getAgentFromId(id);
	if (vap) {
		vap->setAuthState(stateStr);
		m_model->updateAgentState(id);
	}
}

void VoiceAgentRegistry::setConnectionState(QString id, AgentConnectionState state)
{
	QMetaEnum metaEnum = QMetaEnum::fromType<VoiceAgentRegistry::AgentConnectionState>();
	auto stateStr = metaEnum.valueToKey(state);

	VoiceAgentProfile *vap = m_model->getAgentFromId(id);
	if (vap) {
		vap->setConnState(stateStr);
		m_model->updateAgentState(id);
	}
}

void VoiceAgentRegistry::setDialogState(QString id, VoiceDialogState state)
{
	QMetaEnum metaEnum = QMetaEnum::fromType<VoiceAgentRegistry::VoiceDialogState>();
	auto stateStr = metaEnum.valueToKey(state);

	VoiceAgentProfile *vap = m_model->getAgentFromId(id);
	if (vap) {
		vap->setDialogState(stateStr);
	m_model->updateAgentState(id);
	}
}

void VoiceAgentRegistry::updateLoginData(QString id, QString code, QString url,
					 bool expired)
{
	VoiceAgentProfile *vap = m_model->getAgentFromId(id);
	if (vap) {
		vap->setLoginCode(url);
		vap->setLoginUrl(code);
		vap->setLoginPairExpired(expired);
		m_model->updateAgentLoginData(id);
	};
}

int VoiceAgentRegistry::stringToEnum(const QString key, const QString enumtype)
{
	const QMetaObject *metaObject = VoiceAgentRegistry::metaObject();
	int enumIndex = metaObject->indexOfEnumerator(enumtype.toUtf8().constData());
	QMetaEnum metaEnum = metaObject->enumerator(enumIndex);
	int value = metaEnum.keyToValue(key.toUtf8().constData());
	return (value < 0)? 0 : value;
}
