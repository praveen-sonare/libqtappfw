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
#include <QJsonArray>

#include "messageengine.h"
#include "bluetoothmodel.h"

class Bluetooth : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool power READ power WRITE setPower NOTIFY powerChanged)
    Q_PROPERTY(bool discoverable READ discoverable WRITE setDiscoverable NOTIFY discoverableChanged)

    public:
        explicit Bluetooth(QUrl &url, QQmlContext *context, QObject * parent = Q_NULLPTR);
        virtual ~Bluetooth();

        void setPower(bool);
        void setDiscoverable(bool);

        Q_INVOKABLE void start_discovery(void);
        Q_INVOKABLE void stop_discovery(void);

        Q_INVOKABLE void remove_device(QString device);
        Q_INVOKABLE void pair(QString device);
        Q_INVOKABLE void cancel_pair(QString device);

        Q_INVOKABLE void connect(QString device, QString uuid);
        Q_INVOKABLE void connect(QString device);

        Q_INVOKABLE void disconnect(QString device, QString uuid);
        Q_INVOKABLE void disconnect(QString device);

        Q_INVOKABLE void send_confirmation(int pincode);

        bool power() const { return m_power; };
        bool discoverable() const { return m_discoverable; };

    signals:
        void powerChanged(bool state);
        void discoverableChanged();

        void connectionEvent(QJsonObject data);
        void requestConfirmationEvent(QJsonObject data);

    private:
        MessageEngine *m_mloop;
        QQmlContext *m_context;
        BluetoothModel *m_bluetooth;
        void send_command(QString, QJsonObject);
        void set_discovery_filter();
        void discovery_command(bool);
        void populateDeviceList(QJsonObject data);
        void processDeviceChangesEvent(QJsonObject data);
        void processAdapterChangesEvent(QJsonObject data);

        // slots
        void onConnected();
        void onDisconnected();
        void onMessageReceived(MessageType, Message*);

        QString process_uuid(QString uuid) { if (uuid.length() == 36) return uuid; return uuids.value(uuid); };

        // values
        bool m_power;
        bool m_discoverable;

        QMap<QString, QString> uuids;

        const QStringList events {
            "adapter_changes",
            "device_changes",
            "agent",
        };
};

#endif // BLUETOOTH_H
