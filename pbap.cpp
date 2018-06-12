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

#include "message.h"
#include "messageengine.h"
#include "pbap.h"
#include "pbapmessage.h"
#include "responsemessage.h"

Pbap::Pbap (QUrl &url, QObject * parent) :
    QObject(parent),
    m_mloop(nullptr)
{
    m_mloop = new MessageEngine(url);
    QObject::connect(m_mloop, &MessageEngine::messageReceived, this, &Pbap::onMessageReceived);
}

Pbap::~Pbap()
{
    delete m_mloop;
}

void Pbap::search(QString number)
{
    PbapMessage *tmsg = new PbapMessage();
    QJsonObject parameter;

    if (!number.isEmpty())
        parameter.insert("number", number);
    parameter.insert("max_entries", 1);

    tmsg->createRequest("search", parameter);
    m_mloop->sendMessage(tmsg);
    tmsg->deleteLater();
}

void Pbap::sendSearchResults(QJsonArray results)
{
    QString name;

    if (results.empty())
        name = "Not Found";
    else
        name = results.at(0).toObject().value("name").toString();

    emit searchResults(name);
}

void Pbap::onMessageReceived(MessageType type, Message *msg)
{
    if (msg->isReply() && type == ResponseRequestMessage) {
        ResponseMessage *tmsg = qobject_cast<ResponseMessage*>(msg);

        if (tmsg->requestVerb() == "search") {
            sendSearchResults(tmsg->replyData().value("results").toArray());
        }
    }

    msg->deleteLater();
}
