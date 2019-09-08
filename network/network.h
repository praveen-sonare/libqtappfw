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
#include "wifiadapter.h"
#include "wiredadapter.h"

class Network : public QObject
{
    Q_OBJECT

    public:
        explicit Network(QUrl &url, QQmlContext *context, QObject * parent = Q_NULLPTR);
        virtual ~Network();

        Q_INVOKABLE void connect(QString service);
        Q_INVOKABLE void disconnect(QString service);
        Q_INVOKABLE void remove(QString service);
        Q_INVOKABLE void power(bool on, QString type = "wifi");
        Q_INVOKABLE void input(int id, QString passphrase);
        Q_INVOKABLE void configureAddress(QString service, QVariantList paramlist);
        Q_INVOKABLE void configureNameServer(QString service, QVariantList paramlist);

        void getServices();
        AdapterIf* findAdapter(QString type);

    signals:
        void inputRequest(int id);
        void invalidPassphrase(QString service);
        void searchResults(QString name);

    private:
        MessageEngine *m_mloop;
        QQmlContext *m_context;
        QList<AdapterIf*> m_adapters;

        void updateServiceProperties(QJsonObject data);
        bool addService(QJsonObject service);
        void removeService(QJsonObject remove);

        void addServices(QJsonArray services);

        void scanServices(QString type);
        void disableTechnology(QString type);
        void enableTechnology(QString type);
        void parseTechnologies(QJsonArray technologies);
        void getTechnologies();
        void processEvent(NetworkMessage *nmsg);
        void processReply(ResponseMessage *rmsg);

        // slots
        void onMessageReceived(MessageType, Message*);
        void onConnected();
        void onDisconnected();

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
