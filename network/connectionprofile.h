#ifndef CONNECTION_PROFILE_H
#define CONNECTION_PROFILE_H

#include <QString>

class ConnectionProfile
{
    public:
        ConnectionProfile(const QString &address,
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
        QString m_state;
        QString m_ssid;
        int m_strength;
};

#endif // CONNECTION_PROFILE_H
