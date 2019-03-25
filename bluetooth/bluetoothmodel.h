#ifndef BLUETOOTH_MODEL_H
#define BLUETOOTH_MODEL_H

#include <QAbstractListModel>
#include <QSortFilterProxyModel>
#include <QStringList>
#include <QtQml/QQmlContext>
#include <QJsonObject>

class BluetoothDevice
{
    public:
        BluetoothDevice(const QString &id,
                        const QString &address,
                        const QString &name,
                        const bool paired,
                        const bool connected);
        QString id() const;
        QString address() const;
        QString name() const;
        bool paired() const;
        bool connected() const;
        void setId(const QString id);
        void setAddress(const QString address);
        void setName(const QString name);
        void setPaired(const bool paired);
        void setConnected(const bool connected);

    private:
        QString m_id;
        QString m_address;
        QString m_name;
        bool m_paired;
        bool m_connected;
};

class BluetoothModel : public QAbstractListModel
{
    Q_OBJECT

    public:
        enum BluetoothRoles {
            IdRole = Qt::UserRole + 1,
            AddressRole,
            NameRole,
            PairedRole,
            ConnectedRole
        };

        BluetoothModel(QObject *parent = Q_NULLPTR);

        void addDevice(BluetoothDevice *device);
        void removeDevice(BluetoothDevice *device);
        void removeAllDevices();
        BluetoothDevice *getDevice(QString address);
        BluetoothDevice *updateDeviceProperties(BluetoothDevice *device, QJsonObject data);
        int rowCount(const QModelIndex &parent = QModelIndex()) const;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    signals:
        void propertiesChanged(int connected);

    protected:
        QHash<int, QByteArray> roleNames() const;

    private:
        QList<BluetoothDevice *> m_devices;
        QModelIndex indexOf(BluetoothDevice *device);
};

class BluetoothModelFilter : public QSortFilterProxyModel
{
    Q_OBJECT

    public:
        BluetoothModelFilter(QObject *parent = nullptr);

    protected:
        bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
};
#endif // BLUETOOTH_MODEL_H
