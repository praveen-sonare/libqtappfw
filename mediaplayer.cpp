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
#include "mediaplayer.h"
#include "mediaplayermessage.h"

Mediaplayer::Mediaplayer (QUrl &url, QObject * parent) :
    QObject(parent),
    m_mloop(nullptr)
{
    m_mloop = new MessageEngine(url);
    QObject::connect(m_mloop, &MessageEngine::connected, this, &Mediaplayer::onConnected);
    QObject::connect(m_mloop, &MessageEngine::disconnected, this, &Mediaplayer::onDisconnected);
    QObject::connect(m_mloop, &MessageEngine::messageReceived, this, &Mediaplayer::onMessageReceived);
}

Mediaplayer::~Mediaplayer()
{
    delete m_mloop;
}


// Control Verb methods

void Mediaplayer::control(QString control, QJsonObject parameter)
{
    MediaplayerMessage *tmsg = new MediaplayerMessage();

    parameter.insert("value", control);

    tmsg->createRequest("controls", parameter);
    m_mloop->sendMessage(tmsg);
    tmsg->deleteLater();
}


void Mediaplayer::control(QString control)
{
    QJsonObject parameter;

    Mediaplayer::control(control, parameter);
}

void Mediaplayer::play()
{
    control("play");
}

void Mediaplayer::pause()
{
    control("pause");
}

void Mediaplayer::previous()
{
    control("previous");
}

void Mediaplayer::next()
{
    control("next");
}

void Mediaplayer::seek(int milliseconds)
{
    QJsonObject parameter;
    parameter.insert("position", QString::number(milliseconds));

    control("seek", parameter);
}

void Mediaplayer::fastforward(int milliseconds)
{
    QJsonObject parameter;
    parameter.insert("position", QString::number(milliseconds));

    control("fast-forward", parameter);
}

void Mediaplayer::rewind(int milliseconds)
{
    QJsonObject parameter;
    parameter.insert("position", QString::number(milliseconds));

    control("rewind", parameter);
}

void Mediaplayer::picktrack(int track)
{
    QJsonObject parameter;
    parameter.insert("index", QString::number(track));

    control("pick-track", parameter);
}

void Mediaplayer::volume(int volume)
{
    QJsonObject parameter;
    parameter.insert("volume", QString(volume));

    control("volume", parameter);
}

void Mediaplayer::loop(int state)
{
    QJsonObject parameter;
    parameter.insert("state", state ? "true" : "false");

    control("loop", parameter);
}

void Mediaplayer::onConnected()
{
    QStringListIterator eventIterator(events);
    MediaplayerMessage *tmsg;

    while (eventIterator.hasNext()) {
        tmsg = new MediaplayerMessage();
        QJsonObject parameter;
        parameter.insert("value", eventIterator.next());
        tmsg->createRequest("subscribe", parameter);
        m_mloop->sendMessage(tmsg);
        tmsg->deleteLater();
    }
}

void Mediaplayer::onDisconnected()
{
    QStringListIterator eventIterator(events);
    MediaplayerMessage *tmsg;

    while (eventIterator.hasNext()) {
        tmsg = new MediaplayerMessage();
        QJsonObject parameter;
        parameter.insert("value", eventIterator.next());
        tmsg->createRequest("unsubscribe", parameter);
        m_mloop->sendMessage(tmsg);
        tmsg->deleteLater();
    }
}

void Mediaplayer::onMessageReceived(MessageType type, Message *message)
{
    if (type == MediaplayerEventMessage) {
        MediaplayerMessage *tmsg = qobject_cast<MediaplayerMessage*>(message);

        if (tmsg->isEvent()) {
            if (tmsg->isPlaylistEvent()) {
                emit playlistChanged(tmsg->eventData());
            } else if (tmsg->isMetadataEvent()) {
                emit metadataChanged(tmsg->eventData());
            }
        }
    }
    message->deleteLater();
}
