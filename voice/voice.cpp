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

#include <QStringList>
#include "voice.h"
#include "message.h"
#include "messageengine.h"
#include "responsemessage.h"
#include "voicemessage.h"
#include "voiceagentregistry.h"

Voice::Voice (QUrl &url, QQmlContext *context, QObject *parent) :
	QObject(parent),
	m_loop(nullptr)
{
	m_loop = new MessageEngine(url);
	m_var = new VoiceAgentRegistry(this, context, parent);

	QObject::connect(m_loop, &MessageEngine::connected,
			 this, &Voice::onConnected);
	QObject::connect(m_loop, &MessageEngine::disconnected,
			 this, &Voice::onDisconnected);
	QObject::connect(m_loop, &MessageEngine::messageReceived,
			 this, &Voice::onMessageReceived);
}

Voice::~Voice()
{
	delete m_loop;
	delete m_var;
}

void Voice::scan()
{
	VoiceMessage *vmsg = new VshlCoreVoiceMessage();
	QJsonObject parameter;

	vmsg->createRequest("enumerateVoiceAgents", parameter);
	m_loop->sendMessage(vmsg);
	delete vmsg;
}

void Voice::getCBLpair(QString id)
{
	VoiceMessage *vmsg = new AlexaVoiceMessage();
	QJsonObject parameter;

	vmsg->createRequest("subscribeToCBLEvents", parameter);
	m_loop->sendMessage(vmsg);
	delete vmsg;
}

void Voice::subscribeAgentToVshlEvents(QString id)
{
	QJsonArray events = QJsonArray::fromStringList(vshl_events);
	VoiceMessage *vmsg = new VshlCoreVoiceMessage();
	QJsonObject parameter;

	parameter.insert("va_id", id);
	parameter.insert("events", events);
	vmsg->createRequest("subscribe", parameter);
	m_loop->sendMessage(vmsg);
	delete vmsg;
}

void Voice::unsubscribeAgentFromVshlEvents(QString id)
{
	QJsonArray events = QJsonArray::fromStringList(vshl_events);
	VoiceMessage *vmsg = new VshlCoreVoiceMessage();
	QJsonObject parameter;

	parameter.insert("va_id", id);
	parameter.insert("events", events);
	vmsg->createRequest("unsubscribe", parameter);
	m_loop->sendMessage(vmsg);
	delete vmsg;
}

void Voice::subscribeAgentToCblEvents(QString id)
{
	QJsonArray events = QJsonArray::fromStringList(cbl_events);
	VoiceMessage *vmsg = new AlexaVoiceMessage();
	QJsonObject parameter;

	parameter.insert("va_id", id);
	parameter.insert("events", events);
	vmsg->createRequest("subscribeToCBLEvent", parameter);
	m_loop->sendMessage(vmsg);
	delete vmsg;
}

void Voice::parseAgentsList(QJsonArray agents)
{
	for (auto value : agents) {
		QJsonObject a = value.toObject();
		QString id = m_var->addAgent(a);
		subscribeAgentToVshlEvents(id);
	}
}

void Voice::processVshlEvent(VoiceMessage *vmsg)
{
	const QString str = vmsg->eventName();
	const QJsonObject obj = vmsg->eventData();
	QStringList strlist;

	if (str.contains('#'))
		strlist = str.split('#');
	QString agentId = (strlist.isEmpty())?  m_var->getDefaultId() :
						strlist.takeLast();
	if (vmsg->isAuthStateEvent()) {
		const QString authstate = obj.value("state").toString();
		m_var->setAuthState(
			agentId,
			static_cast<VoiceAgentRegistry::ServiceAuthState>(
				m_var->stringToEnum(authstate, "ServiceAuthState")));
	} else if (vmsg->isConnectionStateEvent()) {
		const QString connstate = obj.value("state").toString();
		m_var->setConnectionState(
			agentId,
			static_cast<VoiceAgentRegistry::AgentConnectionState>(
				m_var->stringToEnum(connstate, "AgentConnectionState")));
	} else if (vmsg->isDialogStateEvent()) {
		const QString dialogstate = obj.value("state").toString();
		m_var->setDialogState(
			agentId,
			static_cast<VoiceAgentRegistry::VoiceDialogState>(
			m_var->stringToEnum(dialogstate, "VoiceDialogState")));
	} else
		qWarning() << "Discarding vshl event:" << str;
}

void Voice::processCblEvent(VoiceMessage *vmsg)
{
	const QString str = vmsg->eventName();
	const QJsonObject obj = vmsg->eventData();
	QStringList strlist;

	if (str.contains('#'))
		strlist = str.split('#');
	QString cblevent = (strlist.isEmpty())? QString() : strlist.takeFirst();
	QString agentId =  (strlist.isEmpty())? m_var->getDefaultId() :
						strlist.takeLast();
	if (cblevent == "voice_cbl_codepair_received_event") {
		QString code = obj.value("code").toString();
		QString url = obj.value("url").toString();
		m_var->updateCblPair(agentId, code, url, false);
	} else if (cblevent == "voice_cbl_codepair_expired_event") {
		QString code = obj.value("code").toString();
		QString url = obj.value("url").toString();
		m_var->updateCblPair(agentId, code, url, true);
	} else
		qWarning() << "Discarding cbl event:" << str;
}

void Voice::processEvent(VoiceMessage *vmsg)
{
	const QString api = vmsg->eventApi();
	if (api == "vshl-core")
		processVshlEvent(vmsg);
	else if (api == "alexa-voiceagent")
		processCblEvent(vmsg);
	else
		qWarning() << "Unknown api:" << api;
}

void Voice::processReply(ResponseMessage *rmsg)
{
	if (rmsg->requestVerb() == "enumerateVoiceAgents") {
		parseAgentsList(rmsg->replyData().value("agents").toArray());
		m_var->setDefaultId(
				rmsg->replyData().value("default").toString());
	} else
		qWarning() << "Reply received for unknown verb:" <<
							rmsg->requestVerb();
}

void Voice::onConnected()
{
	scan();
}

void Voice::onDisconnected()
{
	QStringList mvarlist = m_var->getAgentsListById();
	QStringList::iterator it;
	for (it = mvarlist.begin(); it != mvarlist.end(); ++it)
		unsubscribeAgentFromVshlEvents(*it);
}

void Voice::onMessageReceived(MessageType type, Message *msg)
{
	if (msg->isEvent() && type == VoiceEventMessage) {
		processEvent(qobject_cast<VoiceMessage*>(msg));
	} else if (msg->isReply() && (type == ResponseRequestMessage)) {
		processReply(qobject_cast<ResponseMessage*>(msg));
	} else
		qWarning() << "Received unknown message type:" << type;
	msg->deleteLater();
}
