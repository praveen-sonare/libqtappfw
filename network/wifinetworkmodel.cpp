#include "wifinetworkmodel.h"
#include <QDebug>

WifiNetwork::WifiNetwork(const QString &address,
                         const QString &security,
                         const QString &service,
                         const QString &ssid,
                         const QString &state,
                         const int &strength)
    : m_address(address), m_security(security), m_service(service),
      m_ssid(ssid), m_state(state), m_strength(strength)
{
}

QString WifiNetwork::address() const
{
    return m_address;
}

QString WifiNetwork::security() const
{
    return m_security;
}

QString WifiNetwork::service() const
{
    return m_service;
}

QString WifiNetwork::ssid() const
{
    return m_ssid;
}

QString WifiNetwork::state() const
{
    return m_state;
}

int WifiNetwork::strength() const
{
    return m_strength;
}

void WifiNetwork::setAddress(const QString address)
{
    m_address = address;
}

void WifiNetwork::setState(const QString state)
{
    m_state = state;
}

void WifiNetwork::setStrength(const int strength)
{
    m_strength = strength;
}

WifiNetworkModel::WifiNetworkModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void WifiNetworkModel::addNetwork(WifiNetwork *network)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_networks << network;
    endInsertRows();
}

void WifiNetworkModel::removeNetwork(WifiNetwork *network)
{
    int row = m_networks.indexOf(network);
    beginRemoveRows(QModelIndex(), row, row);
    m_networks.removeAt(row);
    endRemoveRows();
    delete network;
}

void WifiNetworkModel::removeAllNetworks()
{
    beginRemoveRows(QModelIndex(), 0, m_networks.count() - 1);
    qDeleteAll(m_networks.begin(), m_networks.end());
    endRemoveRows();
}

int WifiNetworkModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_networks.count();
}

QVariant WifiNetworkModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_networks.count())
        return QVariant();

    const WifiNetwork *network = m_networks[index.row()];

    switch (role) {
        case AddressRole:
            return network->address();
        case SecurityRole:
            return network->security();
        case ServiceRole:
            return network->service();
        case SsidRole:
            return network->ssid();
        case StateRole:
            return network->state();
        case StrengthRole:
            return network->strength();
    }

    return QVariant();
}

QHash<int, QByteArray> WifiNetworkModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[AddressRole] = "address";
    roles[SecurityRole] = "security";
    roles[ServiceRole] = "service";
    roles[SsidRole] = "ssid";
    roles[StateRole] = "sstate";
    roles[StrengthRole] = "strength";

    return roles;
}

QModelIndex WifiNetworkModel::indexOf(WifiNetwork *network)
{
    int row = m_networks.indexOf(network);

    return index(row);
}

WifiNetwork *WifiNetworkModel::getNetwork(QString service)
{
    for (auto network : m_networks) {
        if (network->service() == service)
            return network;
    }

    return nullptr;
}

void WifiNetworkModel::updateProperties(QString service, QJsonObject properties)
{
    WifiNetwork *network;

    // FIXME: add role parameter to emits
    if ((network = getNetwork(service))) {
        if (properties.contains("ipv4")) {
            QString address = properties.value("ipv4").toObject().value("address").toString();
            network->setAddress(address);
            emit dataChanged(indexOf(network), indexOf(network));
        }
        if (properties.contains("state")) {
            network->setState(properties.value("state").toString());
            emit dataChanged(indexOf(network), indexOf(network));
        }
        if (properties.contains("strength")) {
            network->setStrength(properties.value("strength").toInt());
            emit dataChanged(indexOf(network), indexOf(network));
        }
    }
}
