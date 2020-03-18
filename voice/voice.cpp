/*
 * Copyright (C) 2019, 2020 Konsulko Group
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
#include <QDebug>
#include <QStringList>

#include "callmessage.h"
#include "responsemessage.h"
#include "eventmessage.h"
#include "messagefactory.h"
#include "messageengine.h"
#include "voiceagentregistry.h"
#include "voice.h"

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
	std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
	if (!msg)
		return;

	CallMessage *vmsg = static_cast<CallMessage*>(msg.get());
	QJsonObject parameter;

	vmsg->createRequest("vshl-core", "enumerateVoiceAgents", parameter);
	m_loop->sendMessage(std::move(msg));
}

void Voice::getCBLpair(QString id)
{
	triggerCBLProcess(id);
}

void Voice::subscribeAgentToVshlEvents(QString id)
{
	std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
	if (!msg)
		return;

	CallMessage *vmsg = static_cast<CallMessage*>(msg.get());
	QJsonArray events = QJsonArray::fromStringList(vshl_events);
	QJsonObject parameter;

	parameter.insert("va_id", id);
	parameter.insert("events", events);
	vmsg->createRequest("vshl-core", "subscribe", parameter);
	m_loop->sendMessage(std::move(msg));
}

void Voice::unsubscribeAgentFromVshlEvents(QString id)
{
	std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
	if (!msg)
		return;

	CallMessage *vmsg = static_cast<CallMessage*>(msg.get());
	QJsonArray events = QJsonArray::fromStringList(vshl_events);
	QJsonObject parameter;

	parameter.insert("va_id", id);
	parameter.insert("events", events);
	vmsg->createRequest("vshl-core", "unsubscribe", parameter);
	m_loop->sendMessage(std::move(msg));
}

void Voice::triggerCBLProcess(QString id)
{
	std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
	if (!msg)
		return;

	CallMessage *vmsg = static_cast<CallMessage*>(msg.get());
	QJsonArray events;
	QJsonObject parameter;

	parameter.insert("va_id", id);
	parameter.insert("events", events);
	vmsg->createRequest("vshl-core", "subscribeToLoginEvents", parameter);
	m_loop->sendMessage(std::move(msg));
}

void Voice::parseAgentsList(QJsonArray agents)
{
	for (auto value : agents) {
		QJsonObject a = value.toObject();
		QString id = m_var->addAgent(a);
		subscribeAgentToVshlEvents(id);
	}
}



void Voice::processEvent(std::shared_ptr<Message> msg)
{
	std::shared_ptr<EventMessage> emsg = std::static_pointer_cast<EventMessage>(msg);
	QString eapi = emsg->eventApi();

	if (eapi != "vshl-core")
		return;

	QString ename = emsg->eventName();
	QJsonObject data = emsg->eventData();
	QString agentId = data.value("va_id").toString();
	QString state = data.value("state").toString();

	if (ename.contains("voice_authstate_event")) {
		m_var->setAuthState(
			agentId,
			static_cast<VoiceAgentRegistry::ServiceAuthState>(
			m_var->stringToEnum(state, "ServiceAuthState")));

		return;
	}
	else if (ename.contains("voice_connectionstate_event")) {
		m_var->setConnectionState(
			agentId,
			static_cast<VoiceAgentRegistry::AgentConnectionState>(
			m_var->stringToEnum(state, "AgentConnectionState")));
		return;
	}
	else if (ename.contains("voice_dialogstate_event")) {
		m_var->setDialogState(
			agentId,
			static_cast<VoiceAgentRegistry::VoiceDialogState>(
			m_var->stringToEnum(state, "VoiceDialogState")));
		return;
	}
	else if (ename.contains("cbl")) {
		QJsonObject payload = data.value("payload").toObject();
		QString url = payload.value("url").toString();
		QString code = payload.value("code").toString();
		if (ename.contains("expired"))
			m_var->updateLoginData(agentId, code, url, true);
		else if (ename.contains("received")) {
			m_var->updateLoginData(agentId, code, url, false);
		} else
			qWarning() << "Unknown cbl event";
		return;
	}

	qWarning() << "Unknown vshl event:" << ename;
}

void Voice::processReply(std::shared_ptr<Message> msg)
{
	std::shared_ptr<ResponseMessage> rmsg = std::static_pointer_cast<ResponseMessage>(msg);
	QString verb = rmsg->requestVerb();
	QJsonObject data = rmsg->replyData();
	if (rmsg->replyStatus() == "failed") {
		qWarning() << "Reply Failed received for verb:" << verb;
	} else	if (verb == "enumerateVoiceAgents") {
		parseAgentsList(data.value("agents").toArray());
		m_var->setDefaultId(data.value("default").toString());
	} else
		qDebug() << "discarding reply received for verb:" << verb;
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

void Voice::onMessageReceived(std::shared_ptr<Message> msg)
{
	if (msg->isEvent()) {
		processEvent(msg);
	} else if (msg->isReply()) {
		processReply(msg);
	} else
		qWarning() << "Received invalid inbound message";
}
