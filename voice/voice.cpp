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
	VoiceMessage *vmsg = new VoiceMessage();
	QJsonObject parameter;

	vmsg->createRequest("enumerateVoiceAgents", parameter);
	m_loop->sendMessage(vmsg);
	delete vmsg;
}

void Voice::getCBLpair(QString id)
{
	triggerCBLProcess(id);
}

void Voice::subscribeAgentToVshlEvents(QString id)
{
	QJsonArray events = QJsonArray::fromStringList(vshl_events);
	VoiceMessage *vmsg = new VoiceMessage();
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
	VoiceMessage *vmsg = new VoiceMessage();
	QJsonObject parameter;

	parameter.insert("va_id", id);
	parameter.insert("events", events);
	vmsg->createRequest("unsubscribe", parameter);
	m_loop->sendMessage(vmsg);
	delete vmsg;
}

void Voice::triggerCBLProcess(QString id)
{
	QJsonArray events;
	VoiceMessage *vmsg = new VoiceMessage();
	QJsonObject parameter;

	parameter.insert("va_id", id);
	parameter.insert("events", events);
	vmsg->createRequest("subscribeToLoginEvents", parameter);
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



void Voice::processEvent(VoiceMessage *vmsg)
{
	const QString str = vmsg->eventName();
	QJsonObject data = vmsg->eventData();
	QString agentId = data.value("va_id").toString();
	QString state = data.value("state").toString();

	if (vmsg->isAuthStateEvent()) {
		m_var->setAuthState(
			agentId,
			static_cast<VoiceAgentRegistry::ServiceAuthState>(
			m_var->stringToEnum(state, "ServiceAuthState")));

		return;
	}
	else if (vmsg->isConnectionStateEvent()) {
		m_var->setConnectionState(
			agentId,
			static_cast<VoiceAgentRegistry::AgentConnectionState>(
			m_var->stringToEnum(state, "AgentConnectionState")));
		return;
	}
	else if (vmsg->isDialogStateEvent()) {
		m_var->setDialogState(
			agentId,
			static_cast<VoiceAgentRegistry::VoiceDialogState>(
			m_var->stringToEnum(state, "VoiceDialogState")));
		return;
	}
	else if (vmsg->isCblEvent()) {
		QJsonObject payload = data.value("payload").toObject();
		QString url = payload.value("url").toString();
		QString code = payload.value("code").toString();
		if (str.contains("expired"))
			m_var->updateLoginData(agentId, code, url, true);
		else if (str.contains("received")) {
			m_var->updateLoginData(agentId, code, url, false);
		} else
			qWarning() << "Unknown cbl event";
		return;
	}

	qWarning() << "Unknown vshl event:" << str;
}

void Voice::processReply(ResponseMessage *rmsg)
{
	if (rmsg->replyStatus() == "failed") {
		qWarning() << "Reply Failed received for verb:" <<  rmsg->requestVerb();
	} else	if (rmsg->requestVerb() == "enumerateVoiceAgents") {
		parseAgentsList(rmsg->replyData().value("agents").toArray());
		m_var->setDefaultId(
				rmsg->replyData().value("default").toString());
	} else
		qDebug() << "discarding reply received for verb:" <<
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
