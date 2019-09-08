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

#include <QMetaEnum>
#include <QSortFilterProxyModel>
#include <QtQml/QQmlEngine>

#include "network.h"
#include "networkadapter.h"
#include "wifinetworkmodel.h"
#include "connectionprofile.h"

WifiAdapter::WifiAdapter(Network *network, QQmlContext *context, QObject *parent) :
    QObject(parent),
    AdapterIf(),
    m_wifiConnected(false),
    m_wifiEnabled(false),
    m_wifiStrength(0),
    m_model(nullptr),
    nw(network)
{
    m_model = new WifiNetworkModel();
    QSortFilterProxyModel *model = new QSortFilterProxyModel();
    model->setSourceModel(m_model);
    model->setSortRole(WifiNetworkModel::WifiNetworkRoles::SsidRole);
    model->setSortCaseSensitivity(Qt::CaseInsensitive);
    model->sort(0);

    context->setContextProperty("WifiNetworkModel", m_model);
    QObject::connect(m_model, &WifiNetworkModel::strengthChanged, this, &WifiAdapter::updateWifiStrength);
    context->setContextProperty("WifiAdapter", this);
}

WifiAdapter::~WifiAdapter()
{
    delete m_model;
}

void WifiAdapter::updateStatus(QJsonObject properties)
{
    if (properties.contains("connected")) {
        m_wifiConnected = properties.value("connected").toBool();
        emit wifiConnectedChanged(m_wifiConnected);
    }

    if (properties.contains("powered")) {
        m_wifiEnabled = properties.value("powered").toBool();
        emit wifiEnabledChanged(m_wifiEnabled);
        if (m_wifiEnabled)
            nw->getServices();
    }
}

void WifiAdapter::updateWifiStrength(int strength)
{
    m_wifiStrength = strength;
    emit wifiStrengthChanged(m_wifiStrength);
}

void WifiAdapter::updateProperties(QString id, QJsonObject properties)
{
     if (m_model->getNetwork(id))
         m_model->updateProperties(id, properties);
}

bool WifiAdapter::addService(QString id, QJsonObject properties)
{
    QString type = properties.value("type").toString();
    if (type != getType())
        return false;

    QString ssid = properties.value("name").toString();
    // Ignore hidden SSIDs or services already added
    if (m_model->getNetwork(id) || (ssid == ""))
        return false;

    QString state = properties.value("state").toString();
    int strength = properties.value("strength").toInt();
    // Initially support only IPv4 and the first security method found
    QString address = properties.value("ipv4").toObject().value("address").toString();
    QString security = properties.value("security").toArray().at(0).toString();

    ConnectionProfile *network = new ConnectionProfile(address, security, id, state, ssid,
                                                       strength, "", "", "", "", "");
    m_model->addNetwork(network);

    if ((state == "ready") || (state == "online"))
        updateWifiStrength(strength);

    return true;
}

void WifiAdapter::removeService(QString id)
{
    m_model->removeNetwork(m_model->getNetwork(id));

}
