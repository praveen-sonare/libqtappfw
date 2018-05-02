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
#include <QJsonValue>

#include "responsemessage.h"

ResponseMessage::ResponseMessage(QByteArray request)
{

    QJsonDocument jdoc(QJsonDocument::fromJson(request));

    if (!jdoc.isArray()) {
        qWarning("Invalid appfw message: not an array");
        return;
    }

    QJsonArray msg = jdoc.array();

    if (msg.size() != 4) {
        qWarning("Invalid appfw message: invalid array size");
        return;
    }

    QStringList api_str_list = msg[2].toString().split(QRegExp("/"));

    m_request["msgid"] = msg.at(0);
    m_request["callid"] = msg.at(1);
    m_request["api"] = api_str_list[0];
    m_request["verb"] = api_str_list[1];
    m_request["parameter"] = msg.at(3);
}
