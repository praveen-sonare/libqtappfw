/*
 * Copyright (C) 2019 Konsulko Group
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

#ifndef SIGNALCOMPOSER_H
#define SIGNALCOMPOSER_H

#include <QDebug>
#include <QObject>
#include <QJsonArray>

#include "messageengine.h"

class SignalComposer : public QObject
{
    Q_OBJECT

    public:
        explicit SignalComposer(QUrl &url, QObject * parent = Q_NULLPTR);
        virtual ~SignalComposer();

    signals:
        void signalEvent(QString uid, QString value, QString units, quint64 timestamp);

    private:
        MessageEngine *m_mloop;

        void onConnected();
        void onDisconnected();
        void onMessageReceived(MessageType, Message*);

        const QStringList events {
            "event.vehicle.speed",
            "event.engine.speed",
            "event.cruise.enable",
            "event.cruise.resume",
            "event.cruise.set",
            "event.cruise.cancel",
            "event.cruise.limit",
            "event.cruise.distance",
            "event.lane_departure_warning.enable",
            "event.info",
            "event.horn"
    };
};

#endif // SIGNALCOMPOSER_H
