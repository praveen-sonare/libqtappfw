#include "bluetoothmodel.h"
#include <QDebug>

BluetoothDevice::BluetoothDevice(const QString &id,
                                 const QString &address,
                                 const QString &name,
                                 const bool paired,
                                 const bool connected)
    : m_id(id), m_address(address), m_name(name), m_paired(paired),
      m_connected(connected)
{
}

QString BluetoothDevice::id() const
{
    return m_id;
}

QString BluetoothDevice::address() const
{
    return m_address;
}

QString BluetoothDevice::name() const
{
    return m_name;
}

bool BluetoothDevice::paired() const
{
    return m_paired;
}

bool BluetoothDevice::connected() const
{
    return m_connected;
}

void BluetoothDevice::setId(const QString id)
{
    m_id = id;
}

void BluetoothDevice::setAddress(const QString address)
{
    m_address = address;
}

void BluetoothDevice::setName(const QString name)
{
    m_name = name;
}

void BluetoothDevice::setPaired(const bool paired)
{
    m_paired = paired;
}

void BluetoothDevice::setConnected(const bool connected)
{
    m_connected = connected;
}

BluetoothModel::BluetoothModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void BluetoothModel::addDevice(BluetoothDevice *device)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_devices << device;
    endInsertRows();
}

void BluetoothModel::removeDevice(BluetoothDevice *device)
{
    int row = m_devices.indexOf(device);
    beginRemoveRows(QModelIndex(), row, row);
    m_devices.removeAt(row);
    endRemoveRows();
    delete device;
}

void BluetoothModel::removeAllDevices()
{
    beginRemoveRows(QModelIndex(), 0, m_devices.count() - 1);
    qDeleteAll(m_devices.begin(), m_devices.end());
    m_devices.clear();
    endRemoveRows();
}

int BluetoothModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_devices.count();
}

QVariant BluetoothModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_devices.count())
        return QVariant();

    const BluetoothDevice *device = m_devices[index.row()];

    switch (role) {
        case IdRole:
            return device->id();
        case AddressRole:
            return device->address();
        case NameRole:
            return device->name();
        case PairedRole:
            return device->paired();
        case ConnectedRole:
            return device->connected();
    }

    return QVariant();
}

QHash<int, QByteArray> BluetoothModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[AddressRole] = "address";
    roles[NameRole] = "name";
    roles[PairedRole] = "paired";
    roles[ConnectedRole] = "connected";

    return roles;
}

QModelIndex BluetoothModel::indexOf(BluetoothDevice *device)
{
    int row = m_devices.indexOf(device);

    return index(row);
}

BluetoothDevice *BluetoothModel::getDevice(QString id)
{
    for (auto device : m_devices) {
        if (device->id() == id)
            return device;
    }

    return nullptr;
}

BluetoothDevice *BluetoothModel::updateDeviceProperties(BluetoothDevice *device, QJsonObject data)
{
    QJsonObject properties = data.value("properties").toObject();
    QString id = data.value("device").toString();
    QString address = properties.value("address").toString();
    QString name = properties.value("name").toString();
    bool paired = properties.value("paired").toBool();
    bool connected = properties.value("connected").toBool();

    if (device == nullptr)
        return new BluetoothDevice(id, address, name, paired, connected);

    device->setId(id);

    if (!address.isEmpty())
        device->setAddress(address);

    if (!name.isEmpty())
        device->setName(name);

    if (properties.contains("paired"))
        device->setPaired(paired);

    if (properties.contains("connected"))
        device->setConnected(connected);

    emit dataChanged(indexOf(device), indexOf(device));

    return device;
}

BluetoothModelFilter::BluetoothModelFilter(QObject *parent) : QSortFilterProxyModel(parent)
{
}

bool BluetoothModelFilter::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    BluetoothModel *model = qobject_cast<BluetoothModel *>(sourceModel());
    QModelIndex index = model->index(sourceRow);
    bool paired = model->data(index, BluetoothModel::BluetoothRoles::PairedRole).toBool();

    return ((paired ? "true" : "false") == filterRegExp().pattern());
}
