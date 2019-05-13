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

#ifndef ADAPTER_H
#define ADAPTER_H

#include <QDebug>
#include <QObject>
#include <QJsonArray>
#include <QtQml/QQmlContext>

class Network;
class WifiNetworkModel;

class AdapterIf
{
    public:
        virtual ~AdapterIf() {};

        virtual bool addService(QString id, QJsonObject properties) = 0;
        virtual void removeService(QString id) = 0;
        virtual void updateProperties(QString service, QJsonObject properties) = 0;
        virtual QString getType() = 0;
};

Q_DECLARE_INTERFACE(AdapterIf, "AdapterIf")

class WifiAdapter : public QObject, public AdapterIf
{
    Q_OBJECT
    Q_INTERFACES(AdapterIf)
    Q_PROPERTY(bool wifiConnected READ wifiConnected NOTIFY wifiConnectedChanged)
    Q_PROPERTY(bool wifiEnabled READ wifiEnabled NOTIFY wifiEnabledChanged)
    Q_PROPERTY(int wifiStrength READ wifiStrength NOTIFY wifiStrengthChanged)

    public:
        explicit WifiAdapter(Network *network, QQmlContext *context, QObject *parent);
        virtual ~WifiAdapter();

        bool wifiConnected() const { return m_wifiConnected; }
        bool wifiEnabled() const { return m_wifiEnabled; }
        int wifiStrength() const { return m_wifiStrength; }
        void updateWifiStatus(QJsonObject properties);

        bool addService(QString id, QJsonObject properties) override;
        void removeService(QString id) override;
        void updateProperties(QString service, QJsonObject properties) override;
        QString getType() override { return "wifi"; }

    //slots
        void updateWifiStrength(int);

    signals:
        void wifiConnectedChanged(bool connected);
        void wifiEnabledChanged(bool enabled);
        void wifiStrengthChanged(int strength);

    private:
        bool m_wifiConnected;
        bool m_wifiEnabled;
        int m_wifiStrength;
        WifiNetworkModel *m_model;
        Network *nw;
};

#endif // ADAPTER_H
