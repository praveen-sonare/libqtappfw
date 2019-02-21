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

#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <QDebug>
#include <QObject>

#include <QtQml/QQmlContext>
#include <QtQml/QQmlListProperty>
#include "messageengine.h"

class Playlist : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int index READ index NOTIFY indexChanged)
    Q_PROPERTY(int duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(QString path READ path NOTIFY pathChanged)

    // METADATA FIELDS
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QString album READ album NOTIFY albumChanged)
    Q_PROPERTY(QString artist READ artist NOTIFY artistChanged)
    Q_PROPERTY(QString genre READ genre NOTIFY genreChanged)

    public:
        explicit Playlist(QVariantMap& item);
        virtual ~Playlist();

        bool operator<(Playlist& c) { return ((this->m_index < c.m_index)); };
        int index() { return m_index; };
        int duration() { return m_duration; };
        QString path() { return m_path; };

        // METADATA FIELDS
        QString title() { return m_title; };
        QString album() { return m_album; };
        QString artist() { return m_artist; };
        QString genre() { return m_genre; };

    signals:
        void indexChanged();
        void durationChanged();
        void pathChanged();
        void titleChanged();
        void albumChanged();
        void artistChanged();
        void genreChanged();

    private:
        int m_index, m_duration;
        QString m_path, m_title, m_album, m_artist, m_genre;
};

class Mediaplayer : public QObject
{
    Q_OBJECT

    public:
        explicit Mediaplayer(QUrl &url, QQmlContext *context, QObject * parent = Q_NULLPTR);
        virtual ~Mediaplayer();

        // controls
        Q_INVOKABLE void disconnect();
        Q_INVOKABLE void connect();
        Q_INVOKABLE void play();
        Q_INVOKABLE void pause();
        Q_INVOKABLE void previous();
        Q_INVOKABLE void next();
        Q_INVOKABLE void seek(int);
        Q_INVOKABLE void fastforward(int);
        Q_INVOKABLE void rewind(int);
        Q_INVOKABLE void picktrack(int);
        Q_INVOKABLE void volume(int);
        Q_INVOKABLE void loop(QString);

        void updatePlaylist(QVariantMap playlist);

    signals:
        void metadataChanged(QVariantMap metadata);

    private:
        MessageEngine *m_mloop;
        QQmlContext *m_context;
        QList<QObject *> m_playlist;

        void control(QString, QJsonObject);
        void control(QString);

        void onConnected();
        void onDisconnected();
        void onMessageReceived(MessageType, Message*);

        const QStringList events {
            "playlist",
            "metadata",
        };
};

#endif // MEDIAPLAYER_H
