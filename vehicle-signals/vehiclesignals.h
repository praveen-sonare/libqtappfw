/*
 * Copyright (C) 2022 Konsulko Group
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef VEHICLESIGNALS_H
#define VEHICLESIGNALS_H

#include <QObject>
#include <QWebSocket>

// Class to read/hold VIS server configuration

class VehicleSignalsConfig
{
public:
        explicit VehicleSignalsConfig(const QString &hostname,
				      const unsigned port,
				      const QByteArray &clientKey,
				      const QByteArray &clientCert,
				      const QByteArray &caCert,
				      const QString &authToken,
				      bool verifyPeer = true);
        explicit VehicleSignalsConfig(const QString &appname);
        ~VehicleSignalsConfig() {};

	QString hostname() { return m_hostname; };
	unsigned port() { return m_port; };
	QByteArray clientKey() { return m_clientKey; };
	QByteArray clientCert() { return m_clientCert; };
	QByteArray caCert() { return m_caCert; };
	QString authToken() { return m_authToken; };
	bool verifyPeer() { return m_verifyPeer; };
	bool valid() { return m_valid; };
	unsigned verbose() { return m_verbose; };

private:
	QString m_hostname;
	unsigned m_port;
	QByteArray m_clientKey;
	QByteArray m_clientCert;
	QByteArray m_caCert;
	QString m_authToken;
	bool m_verifyPeer;
	bool m_valid;
	unsigned m_verbose;
};

// VIS signaling interface class

class VehicleSignals : public QObject
{
	Q_OBJECT

public:
        explicit VehicleSignals(const VehicleSignalsConfig &config, QObject * parent = Q_NULLPTR);
        virtual ~VehicleSignals();

	Q_INVOKABLE void connect();	
	Q_INVOKABLE void authorize();

	Q_INVOKABLE void get(const QString &path);
	Q_INVOKABLE void set(const QString &path, const QString &value);
	Q_INVOKABLE void subscribe(const QString &path);

signals:
	void connected();
	void authorized();
        void getSuccessResponse(QString path, QString value, QString timestamp);
        void signalNotification(QString path, QString value, QString timestamp);
	void disconnected();

private slots:
	void onConnected();
	void onError(QAbstractSocket::SocketError error);
	void reconnect();
	void onDisconnected();
	void onTextMessageReceived(QString message);

private:
	VehicleSignalsConfig m_config;
	QWebSocket m_websocket;
	std::atomic<unsigned int> m_request_id;

	bool parseData(const QJsonObject &response, QString &path, QString &value, QString &timestamp);
};

#endif // VEHICLESIGNALS_H
