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

#include "messageengine.h"

class Mediaplayer : public QObject
{
    Q_OBJECT

    public:
        explicit Mediaplayer(QUrl &url, QObject * parent = Q_NULLPTR);
        virtual ~Mediaplayer();

        // controls
        Q_INVOKABLE void play();
        Q_INVOKABLE void pause();
        Q_INVOKABLE void previous();
        Q_INVOKABLE void next();
        Q_INVOKABLE void seek(int);
        Q_INVOKABLE void fastforward(int);
        Q_INVOKABLE void rewind(int);
        Q_INVOKABLE void picktrack(int);
        Q_INVOKABLE void volume(int);
        Q_INVOKABLE void loop(int);

    signals:
        void playlistChanged(QJsonObject playlist);
        void metadataChanged(QJsonObject metadata);

    private:
        MessageEngine *m_mloop;

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
