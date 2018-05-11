/*
 * Copyright (C) 2018 Konsulko Group
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
#include <QJsonDocument>
#include <QJsonObject>

#include "mediaplayermessage.h"

bool MediaplayerMessage::createRequest(QString verb, QJsonObject parameter)
{
	QStringList verbs {"playlist", "metadata", "controls", "subscribe", "unsubscribe"};
	if (!verbs.contains(verb))
		return false;

	return Message::createRequest("mediaplayer", verb, parameter);
}
