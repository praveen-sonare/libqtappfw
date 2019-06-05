#include "abstractnetworkmodel.h"
#include "connectionprofile.h"
#include <QDebug>


AbstractNetworkModel::AbstractNetworkModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void AbstractNetworkModel::addNetwork(ConnectionProfile *network)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_networks << network;
    endInsertRows();
}

void AbstractNetworkModel::removeNetwork(ConnectionProfile *network)
{
    int row = m_networks.indexOf(network);
    beginRemoveRows(QModelIndex(), row, row);
    m_networks.removeAt(row);
    endRemoveRows();
    delete network;
}

void AbstractNetworkModel::removeAllNetworks()
{
    beginRemoveRows(QModelIndex(), 0, m_networks.count() - 1);
    qDeleteAll(m_networks.begin(), m_networks.end());
    m_networks.clear();
    endRemoveRows();
}

ConnectionProfile *AbstractNetworkModel::getNetwork(QString service)
{
    for (auto network : m_networks) {
        if (network->service() == service)
            return network;
    }

    return nullptr;
}

int AbstractNetworkModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_networks.count();
}

QModelIndex AbstractNetworkModel::indexOf(ConnectionProfile *network)
{
    int row = m_networks.indexOf(network);

    return index(row);
}
