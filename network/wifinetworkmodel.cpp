#include "wifinetworkmodel.h"
#include "connectionprofile.h"
#include <QDebug>

WifiNetworkModel::WifiNetworkModel(QObject *parent)
    : AbstractNetworkModel(parent)
{
}

QVariant WifiNetworkModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_networks.count())
        return QVariant();

    ConnectionProfile *network = m_networks[index.row()];

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

void WifiNetworkModel::updateProperties(QString service, QJsonObject properties)
{
    ConnectionProfile *network;

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
            if ((network->state() == "ready") ||
                (network->state() == "online"))
                    emit strengthChanged(network->strength());
        }
    }
}
