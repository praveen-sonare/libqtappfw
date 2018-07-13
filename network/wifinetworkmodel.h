#ifndef WIFI_NETWORK_MODEL_H
#define WIFI_NETWORK_MODEL_H

#include <QAbstractListModel>
#include <QStringList>
#include <QtQml/QQmlContext>
#include <QJsonObject>

class WifiNetwork
{
    public:
        WifiNetwork(const QString &address,
                    const QString &security,
                    const QString &service,
                    const QString &ssid,
                    const QString &state,
                    const int &strength);
        QString address() const;
        QString service() const;
        QString ssid() const;
        QString security() const;
        QString state() const;
        int strength() const;
        void setAddress(const QString address);
        void setState(const QString state);
        void setStrength(const int strength);

    private:
        QString m_address;
        QString m_security;
        QString m_service;
        QString m_ssid;
        QString m_state;
        int m_strength;
};

class WifiNetworkModel : public QAbstractListModel
{
    Q_OBJECT

    public:
        enum WifiNetworkRoles {
            AddressRole = Qt::UserRole + 1,
            SecurityRole,
            ServiceRole,
            SsidRole,
            StateRole,
            StrengthRole
        };

        WifiNetworkModel(QObject *parent = Q_NULLPTR);

        void addNetwork(WifiNetwork *network);
        void removeNetwork(WifiNetwork *network);
        void removeAllNetworks();
        WifiNetwork *getNetwork(QString service);
        int rowCount(const QModelIndex &parent = QModelIndex()) const;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
        void updateProperties(QString service, QJsonObject properties);

    protected:
        QHash<int, QByteArray> roleNames() const;

    private:
        QList<WifiNetwork *> m_networks;
        QModelIndex indexOf(WifiNetwork *network);
};
#endif // WIFI_NETWORK_MODEL_H
