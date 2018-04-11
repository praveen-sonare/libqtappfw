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

#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <QDebug>
#include <QObject>

#include "messageengine.h"

class Bluetooth : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool power READ power WRITE setPower NOTIFY powerChanged)
    Q_PROPERTY(bool discoverable READ discoverable WRITE setDiscoverable NOTIFY discoverableChanged)

    public:
        explicit Bluetooth(QUrl &url, QObject * parent = Q_NULLPTR);
        virtual ~Bluetooth();

        void setPower(bool);
        void setDiscoverable(bool);

        Q_INVOKABLE void start_discovery(void);
        Q_INVOKABLE void stop_discovery(void);

        Q_INVOKABLE void remove_device(QString address);
        Q_INVOKABLE void pair(QString address);
        Q_INVOKABLE void cancel_pair(QString address);

        Q_INVOKABLE void connect(QString address, QString uuid);
        Q_INVOKABLE void connect(QString address);

        Q_INVOKABLE void disconnect(QString address, QString uuid);
        Q_INVOKABLE void disconnect(QString address);

        Q_INVOKABLE void send_confirmation(void);

        bool power() const { return m_power; };
        bool discoverable() const { return m_discoverable; };

    signals:
        void powerChanged();
        void discoverableChanged();

        void connectionEvent(QJsonObject data);
        void requestConfirmationEvent(QJsonObject data);
        void deviceAddedEvent(QJsonObject data);
        void deviceRemovedEvent(QJsonObject data);
        void deviceUpdatedEvent(QJsonObject data);
        void deviceListEvent(QJsonObject data);

    private:
        MessageEngine *m_mloop;
        void generic_command(QString, QString);

        // slots
        void onConnected();
        void onDisconnected();
        void onMessageReceived(MessageType, Message*);

        bool isDiscoveryListResponse(Message *tmsg) { return (tmsg->replyInfo() == "BT - Scan Result is Displayed"); };
        bool isPowerResponse(Message *tmsg) { return (tmsg->replyInfo() == "Radio - Power set"); };

        // values
        bool m_power;
        bool m_discoverable;

        const QStringList events {
            "connection",
            "request_confirmation",
            "device_added",
            "device_removed",
            "device_updated",
        };
};

#endif // BLUETOOTH_H
