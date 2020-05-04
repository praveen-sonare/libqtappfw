/*
 * Copyright (C) 2018-2020 Konsulko Group
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
#include <QJsonArray>

#include "callmessage.h"
#include "eventmessage.h"
#include "messagefactory.h"
#include "messageengine.h"
#include "messageenginefactory.h"
#include "weather.h"


Weather::Weather (QUrl &url, QObject * parent) :
	QObject(parent)
{
	m_mloop = MessageEngineFactory::getInstance().getMessageEngine(url);
	QObject::connect(m_mloop.get(), &MessageEngine::connected, this, &Weather::onConnected);
	QObject::connect(m_mloop.get(), &MessageEngine::disconnected, this, &Weather::onDisconnected);
	QObject::connect(m_mloop.get(), &MessageEngine::messageReceived, this, &Weather::onMessageReceived);
}

Weather::~Weather()
{
}

void Weather::onConnected()
{
	std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
	if (!msg)
		return;

	CallMessage *tmsg = static_cast<CallMessage*>(msg.get());
	tmsg->createRequest("weather", "subscribe", "weather");
	m_mloop->sendMessage(std::move(msg));
}

void Weather::onDisconnected()
{
	std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
	if (!msg)
		return;

	CallMessage *tmsg = static_cast<CallMessage*>(msg.get());
	tmsg->createRequest("weater", "unsubscribe", "weather");
	m_mloop->sendMessage(std::move(msg));
}

void Weather::onMessageReceived(std::shared_ptr<Message> msg)
{
	if (!msg)
		return;

	if (msg->isEvent()) {
		std::shared_ptr<EventMessage> emsg = std::static_pointer_cast<EventMessage>(msg);
		if (emsg->eventApi() != "weather")
			return;

		QJsonObject data = emsg->eventData();
		m_temperature = QString::number(data.value("main").toObject().value("temp").toDouble());
		m_condition = data.value("weather").toArray().at(0).toObject().value("description").toString();

			emit temperatureChanged(m_temperature);
			emit conditionChanged(m_condition);
		}
}
