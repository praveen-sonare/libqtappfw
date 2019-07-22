#include "wirednetworkmodel.h"
#include "connectionprofile.h"
#include <QVector>
#include <QDebug>

WiredNetworkModel::WiredNetworkModel(QObject *parent)
    : AbstractNetworkModel(parent)
{
}

QVariant WiredNetworkModel::data(const QModelIndex &index, int role) const
{
    QVariant ret;

    if (!index.isValid())
        return ret;

    if (index.row() < 0 || index.row() >= m_networks.count())
        return ret;

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

    return ret;
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
    QVector<int> vroles;

    if ((network = getNetwork(service))) {
        if (properties.contains("ipv4")) {
            QString address = properties.value("ipv4").toObject().value("address").toString();
            network->setAddress(address);
            vroles.push_back(AddressRole);
        }
        if (properties.contains("state")) {
            network->setState(properties.value("state").toString());
            vroles.push_back(StateRole);
        }
        if (!vroles.isEmpty())
             emit dataChanged(indexOf(network), indexOf(network), vroles);

    }
}
