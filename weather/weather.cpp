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

#include "message.h"
#include "weathermessage.h"
#include "messageengine.h"
#include "weather.h"


Weather::Weather (QUrl &url, QObject * parent) :
	QObject(parent),
	m_mloop(nullptr)
{
	m_mloop = new MessageEngine(url);
	QObject::connect(m_mloop, &MessageEngine::connected, this, &Weather::onConnected);
	QObject::connect(m_mloop, &MessageEngine::disconnected, this, &Weather::onDisconnected);
	QObject::connect(m_mloop, &MessageEngine::messageReceived, this, &Weather::onMessageReceived);
}

Weather::~Weather()
{
	delete m_mloop;
}

void Weather::onConnected()
{
	WeatherMessage *tmsg = new WeatherMessage();
	tmsg->createRequest("subscribe", "weather");
	m_mloop->sendMessage(tmsg);
	delete tmsg;
}

void Weather::onDisconnected()
{
	WeatherMessage *tmsg = new WeatherMessage();
	tmsg->createRequest("unsubscribe", "weather");
	m_mloop->sendMessage(tmsg);
	delete tmsg;
}

void Weather::onMessageReceived(MessageType type, Message *message)
{
	if (type == MessageType::WeatherEventMessage) {
		WeatherMessage *tmsg = qobject_cast<WeatherMessage*>(message);

		if (tmsg->isEvent()) {
			m_temperature = tmsg->temperature();
			m_condition = tmsg->condition();

			emit temperatureChanged(m_temperature);
			emit conditionChanged(m_condition);
		}
	}
	message->deleteLater();
}
