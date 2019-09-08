#include "wirednetworkmodel.h"
#include "connectionprofile.h"
#include <QVector>
#include <QDebug>

WiredNetworkModel::WiredNetworkModel(QObject *parent)
    : AbstractNetworkModel(parent)
{
}

QVariantList WiredNetworkModel::readCurrentRouteConfig(const QModelIndex &index) const
{
    QVariantList ret;

    if (!index.isValid())
        return ret;

    if (index.row() < 0 || index.row() >= this->m_networks.count())
        return ret;

    const ConnectionProfile *network = this->m_networks[index.row()];
    ret.append(network->addrmethod());
    ret.append(network->address());
    ret.append(network->netmask());
    ret.append(network->gateway());
    return ret;
}

QVariantList WiredNetworkModel::readCurrentNameServerConfig(const QModelIndex &index) const
{
    QVariantList ret;

    if (!index.isValid())
        return ret;

    if (index.row() < 0 || index.row() >= this->m_networks.count())
        return ret;

    const ConnectionProfile *network = this->m_networks[index.row()];
    ret.append(network->nsmethod());
    ret.append(network->nameservers());
    return ret;
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
        case RouteRole:
            return readCurrentRouteConfig(index);
        case NameServerRole:
            return readCurrentNameServerConfig(index);
    }

    return ret;
}

QHash<int, QByteArray> WiredNetworkModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[AddressRole] = "address";
    roles[SecurityRole] = "security";
    roles[ServiceRole] = "service";
    roles[StateRole] = "sstate";
    roles[RouteRole] = "sroute";
    roles[NameServerRole] = "nservers";

    return roles;
}

void WiredNetworkModel::updateProperties(QString service, QJsonObject properties)
{
    ConnectionProfile *network;
    QVector<int> vroles;

    if ((network = getNetwork(service))) {
        if (properties.contains("ipv4")) {
            QJsonObject ipv4obj = properties.value("ipv4").toObject();
            network->setAddress(ipv4obj.value("address").toString());
            network->setNetmask(ipv4obj.value("netmask").toString());
            network->setGateway(ipv4obj.value("gateway").toString());
            network->setAddrMethod(ipv4obj.value("method").toString());
            vroles.push_back(AddressRole);
            vroles.push_back(RouteRole);
        }
        if (properties.contains("nameservers")) {
            QString nservers = properties.value("nameservers").toString();
            network->setNameservers(nservers);
            (network->addrmethod() == "dhcp")? network->setNSMethod("auto") :
                                             network->setNSMethod("manual");
            vroles.push_back(NameServerRole);
        }
        if (properties.contains("state")) {
            network->setState(properties.value("state").toString());
            vroles.push_back(StateRole);
        }
        if (!vroles.isEmpty())
             emit dataChanged(indexOf(network), indexOf(network), vroles);

    }
}
