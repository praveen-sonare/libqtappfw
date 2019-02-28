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

#ifndef NETWORK_H
#define NETWORK_H

#include <QDebug>
#include <QObject>
#include <QJsonArray>
#include <QtQml/QQmlContext>
#include <QtQml/QQmlListProperty>

#include "messageengine.h"
#include "networkmessage.h"
#include "responsemessage.h"
#include "wifinetworkmodel.h"

class Network : public QObject
{
    Q_OBJECT

    public:
        explicit Network(QUrl &url, QQmlContext *context, QObject * parent = Q_NULLPTR);
        virtual ~Network();

        Q_INVOKABLE void connect(QString service);
        Q_INVOKABLE void disconnect(QString service);
        Q_INVOKABLE void power(bool on);
        Q_INVOKABLE void input(int id, QString passphrase);

        Q_PROPERTY(bool wifiConnected READ wifiConnected NOTIFY wifiConnectedChanged)
        Q_PROPERTY(bool wifiEnabled READ wifiEnabled NOTIFY wifiEnabledChanged)
        Q_PROPERTY(bool wifiStrength READ wifiStrength NOTIFY wifiStrengthChanged)

        bool wifiConnected() const { return m_wifiConnected; }
        bool wifiEnabled() const { return m_wifiEnabled; }
        bool wifiStrength() const { return m_wifiStrength; }

    signals:
        void inputRequest(int id);
        void invalidPassphrase(QString service);
        void searchResults(QString name);
        void statusChanged(bool connected);
        void wifiConnectedChanged(bool connected);
        void wifiEnabledChanged(bool enabled);
        void wifiStrengthChanged(int strength);

    private:
        MessageEngine *m_mloop;
        QQmlContext *m_context;
        WifiNetworkModel *m_wifi;
        bool m_wifiConnected;
        bool m_wifiEnabled;
        int m_wifiStrength;

        void updateWifiStatus(QJsonObject properties);
        void updateServiceProperties(QJsonObject data);
        bool addService(QJsonObject service);
        void removeService(QJsonObject remove);
        void addServices(QJsonArray services);
        void getServices();
        void scanServices(QString type);
        void disableTechnology(QString type);
        void enableTechnology(QString type);
        void parseTechnologies(QJsonArray technologies);
        void getTechnologies();
        void processEvent(NetworkMessage *nmsg);
        void processReply(ResponseMessage *rmsg);

        // slots
        void onConnected();
        void onDisconnected();
        void onMessageReceived(MessageType, Message*);
        void updateWifiStrength(int);

        const QStringList events {
            "agent",
            "global_state",
            "services",
            "service_properties",
            "technologies",
            "technology_properties",
        };
};

#endif // NETWORK_H
