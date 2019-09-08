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
#include "wirednetworkmodel.h"
#include "connectionprofile.h"

WiredAdapter::WiredAdapter(Network *network, QQmlContext *context, QObject *parent) :
    QObject(parent),
    AdapterIf(),
    m_wiredConnected(false),
    m_wiredEnabled(false),
    m_model(nullptr),
    nw(network)
{
    m_model = new WiredNetworkModel();
    QSortFilterProxyModel *model = new QSortFilterProxyModel();
    model->setSourceModel(m_model);
    model->setSortRole(WiredNetworkModel::WiredNetworkRoles::ServiceRole);
    model->setSortCaseSensitivity(Qt::CaseInsensitive);
    model->sort(0);

    context->setContextProperty("WiredNetworkModel", m_model);
    context->setContextProperty("WiredAdapter", this);
}

WiredAdapter::~WiredAdapter()
{
    delete m_model;
}

void WiredAdapter::updateStatus(QJsonObject properties)
{
    if (properties.contains("connected")) {
        m_wiredConnected = properties.value("connected").toBool();
        emit wiredConnectedChanged(m_wiredConnected);
    }

    if (properties.contains("powered")) {
        m_wiredEnabled = properties.value("powered").toBool();
        emit wiredEnabledChanged(m_wiredEnabled);
        if (m_wiredEnabled)
            nw->getServices();
    }
}

void WiredAdapter::updateProperties(QString id, QJsonObject properties)
{
     if (m_model->getNetwork(id))
         m_model->updateProperties(id, properties);
}

bool WiredAdapter::addService(QString id, QJsonObject properties)
{
    QString type = properties.value("type").toString();
    if (type != getType())
        return false;

    // Ignore services already added
    if (m_model->getNetwork(id))
        return false;

    QString state = properties.value("state").toString();
    // Initially support only IPv4 and the first security method found
    QString security = properties.value("security").toArray().at(0).toString();
    QJsonObject ipv4obj = properties.value("ipv4").toObject();
    QString address = ipv4obj.value("address").toString();
    QString netmask = ipv4obj.value("netmask").toString();
    QString gateway = ipv4obj.value("gateway").toString();
    QString amethod = ipv4obj.value("method").toString();
    QString ns = properties.value("nameservers").toString();
    QString nsmethod = (amethod == "dhcp")? "auto" : "manual";

    ConnectionProfile *network = new ConnectionProfile(address, security, id,
                                                       state, "", 0, netmask,
                                                       gateway, amethod, ns,
                                                       nsmethod);
    m_model->addNetwork(network);

    return true;
}

void WiredAdapter::removeService(QString id)
{
    m_model->removeNetwork(m_model->getNetwork(id));

}
