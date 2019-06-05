#include "connectionprofile.h"

ConnectionProfile::ConnectionProfile(const QString &address,
                                     const QString &security,
                                     const QString &service,
                                     const QString &state,
                                     const QString &ssid,
                                     const int &strength)
    : m_address(address), m_security(security), m_service(service),
      m_state(state), m_ssid(ssid), m_strength(strength)
{
}

QString ConnectionProfile::address() const
{
    return m_address;
}

QString ConnectionProfile::security() const
{
    return m_security;
}

QString ConnectionProfile::service() const
{
    return m_service;
}

QString ConnectionProfile::state() const
{
    return m_state;
}

QString ConnectionProfile::ssid() const
{
    return m_ssid;
}

int ConnectionProfile::strength() const
{
    return m_strength;
}

void ConnectionProfile::setAddress(const QString address)
{
    m_address = address;
}

void ConnectionProfile::setState(const QString state)
{
    m_state = state;
}

void ConnectionProfile::setStrength(const int strength)
{
    m_strength = strength;
}
