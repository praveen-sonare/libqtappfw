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

#include "callmessage.h"
#include "eventmessage.h"
#include "messageengine.h"
#include "messagefactory.h"
#include "messageenginefactory.h"
#include "mediaplayer.h"


Playlist::Playlist(QVariantMap &item)
{
    m_index = item["index"].toInt();
    m_duration = item["duration"].toInt();
    m_genre = item["genre"].toString();
    m_path = item["path"].toString();
    m_title = item["title"].toString();
    m_album = item["album"].toString();
    m_artist = item["artist"].toString();
}

Playlist::~Playlist() {}

Mediaplayer::Mediaplayer (QUrl &url, QQmlContext *context, QObject * parent) :
    QObject(parent)
{
    m_mloop = MessageEngineFactory::getInstance().getMessageEngine(url);
    m_context = context;
    m_context->setContextProperty("MediaplayerModel", QVariant::fromValue(m_playlist));
    QObject::connect(m_mloop.get(), &MessageEngine::connected, this, &Mediaplayer::onConnected);
    QObject::connect(m_mloop.get(), &MessageEngine::disconnected, this, &Mediaplayer::onDisconnected);
    QObject::connect(m_mloop.get(), &MessageEngine::messageReceived, this, &Mediaplayer::onMessageReceived);
}

Mediaplayer::~Mediaplayer()
{
}

// Qt UI Context

void Mediaplayer::updatePlaylist(QVariantMap playlist)
{
    QVariantList list = playlist["list"].toList();

    m_playlist.clear();

    for (auto i : list) {
        QVariantMap item = qvariant_cast<QVariantMap>(i);
        m_playlist.append(new Playlist(item));
    }

    if (m_playlist.count() == 0) {
        QVariantMap tmp, track;

        track.insert("title", "");
        track.insert("artist", "");
        track.insert("album", "");
        track.insert("duration", 0);

        tmp.insert("position", 0);
        tmp.insert("track", track);

        // clear metadata in UI
        m_context->setContextProperty("AlbumArt", "");
        emit metadataChanged(tmp);
    }

    // Refresh model
    m_context->setContextProperty("MediaplayerModel", QVariant::fromValue(m_playlist));
}

// Control Verb methods

void Mediaplayer::control(QString control, QJsonObject parameter)
{
    std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
    if (!msg)
        return;

    CallMessage* mpmsg = static_cast<CallMessage*>(msg.get());
    parameter.insert("value", control);

    mpmsg->createRequest("mediaplayer", "controls", parameter);
    m_mloop->sendMessage(std::move(msg));
}


void Mediaplayer::control(QString control)
{
    QJsonObject parameter;

    Mediaplayer::control(control, parameter);
}

void Mediaplayer::disconnect()
{
    control("disconnect");
}

void Mediaplayer::connect()
{
    control("connect");
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

void Mediaplayer::loop(QString state)
{
    QJsonObject parameter;
    parameter.insert("state", state);

    control("loop", parameter);
}

void Mediaplayer::onConnected()
{
    QStringListIterator eventIterator(events);

    while (eventIterator.hasNext()) {
        std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
        if (!msg)
            return;

        CallMessage* mpmsg = static_cast<CallMessage*>(msg.get());
        QJsonObject parameter;
        parameter.insert("value", eventIterator.next());
        mpmsg->createRequest("mediaplayer", "subscribe", parameter);
        m_mloop->sendMessage(std::move(msg));
    }
}

void Mediaplayer::onDisconnected()
{
    QStringListIterator eventIterator(events);

    while (eventIterator.hasNext()) {
        std::unique_ptr<Message> msg = MessageFactory::getInstance().createOutboundMessage(MessageId::Call);
        if (!msg)
            return;

	CallMessage* mpmsg = static_cast<CallMessage*>(msg.get());
        QJsonObject parameter;
        parameter.insert("value", eventIterator.next());
        mpmsg->createRequest("mediaplayer", "unsubscribe", parameter);
        m_mloop->sendMessage(std::move(msg));
    }
}

void Mediaplayer::onMessageReceived(std::shared_ptr<Message> msg)
{
    if (!msg)
        return;

    if (msg->isEvent()){
        std::shared_ptr<EventMessage> emsg = std::static_pointer_cast<EventMessage>(msg);
        QString ename = emsg->eventName();
        QString eapi = emsg->eventApi();
        QJsonObject data = emsg->eventData();
        if (eapi != "mediaplayer")
            return;

        if (ename == "playlist") {
            updatePlaylist(data.toVariantMap());
        } else if (ename == "metadata") {
            QVariantMap map = data.toVariantMap();

                if (map.contains("track")) {
                    QVariantMap track = map.value("track").toMap();

                    if (track.contains("image")) {
                        m_context->setContextProperty("AlbumArt",
                            QVariant::fromValue(track.value("image")));
                    }

                    if (!track.contains("artist")) {
                        track.insert("artist", "");
                        map["track"] = track;
                    }

                    if (!track.contains("album")) {
                        track.insert("album", "");
                        map["track"] = track;
                    }
                }

                emit metadataChanged(map);
            }
        }
}
