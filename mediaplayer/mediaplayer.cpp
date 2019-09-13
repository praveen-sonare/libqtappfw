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
    QObject(parent),
    m_mloop(nullptr)
{
    m_mloop = new MessageEngine(url);
    m_context = context;
    m_context->setContextProperty("MediaplayerModel", QVariant::fromValue(m_playlist));
    QObject::connect(m_mloop, &MessageEngine::connected, this, &Mediaplayer::onConnected);
    QObject::connect(m_mloop, &MessageEngine::disconnected, this, &Mediaplayer::onDisconnected);
    QObject::connect(m_mloop, &MessageEngine::messageReceived, this, &Mediaplayer::onMessageReceived);
}

Mediaplayer::~Mediaplayer()
{
    delete m_mloop;
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
    MediaplayerMessage *tmsg = new MediaplayerMessage();

    parameter.insert("value", control);

    tmsg->createRequest("controls", parameter);
    m_mloop->sendMessage(tmsg);
    delete tmsg;
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
    MediaplayerMessage *tmsg;

    while (eventIterator.hasNext()) {
        tmsg = new MediaplayerMessage();
        QJsonObject parameter;
        parameter.insert("value", eventIterator.next());
        tmsg->createRequest("subscribe", parameter);
        m_mloop->sendMessage(tmsg);
        delete tmsg;
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
        delete tmsg;
    }
}

void Mediaplayer::onMessageReceived(MessageType type, Message *message)
{
    if (type == MediaplayerEventMessage) {
        MediaplayerMessage *tmsg = qobject_cast<MediaplayerMessage*>(message);

        if (tmsg->isEvent()) {
            if (tmsg->isPlaylistEvent()) {
                updatePlaylist(tmsg->eventData().toVariantMap());
            } else if (tmsg->isMetadataEvent()) {
                QVariantMap map = tmsg->eventData().toVariantMap();

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
    message->deleteLater();
}
