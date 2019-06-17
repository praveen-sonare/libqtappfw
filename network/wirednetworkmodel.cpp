#include "wirednetworkmodel.h"
#include "connectionprofile.h"
#include <QDebug>

WiredNetworkModel::WiredNetworkModel(QObject *parent)
    : AbstractNetworkModel(parent)
{
}

QVariant WiredNetworkModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_networks.count())
        return QVariant();

    const ConnectionProfile *network = m_networks[index.row()];

    switch (role) {
        case AddressRole:
            return network->address();
        case SecurityRole:
            return network->security();
        case ServiceRole:
            return network->service();
        case StateRole:
            return network->state();
    }

    return QVariant();
}

QHash<int, QByteArray> WiredNetworkModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[AddressRole] = "address";
    roles[SecurityRole] = "security";
    roles[ServiceRole] = "service";
    roles[StateRole] = "sstate";

    return roles;
}

void WiredNetworkModel::updateProperties(QString service, QJsonObject properties)
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
    }
}