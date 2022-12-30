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

#include <QDebug>
#include <QSettings>
#include <QUrl>
#include <QFile>
#include <QSslKey>
#include <QTimer>
#include <QVariantMap>
#include <QJsonDocument>
#include <QJsonObject>

#include "vehiclesignals.h"

#define DEFAULT_CLIENT_KEY_FILE  "/etc/kuksa-val/Client.key"
#define DEFAULT_CLIENT_CERT_FILE "/etc/kuksa-val/Client.pem"
#define DEFAULT_CA_CERT_FILE     "/etc/kuksa-val/CA.pem"

VehicleSignalsConfig::VehicleSignalsConfig(const QString &hostname,
					   const unsigned port,
					   const QByteArray &clientKey,
					   const QByteArray &clientCert,
					   const QByteArray &caCert,
					   const QString &authToken,
					   bool verifyPeer) :
	m_hostname(hostname),
	m_port(port),
	m_clientKey(clientKey),
	m_clientCert(clientCert),
	m_caCert(caCert),
	m_authToken(authToken),
	m_verifyPeer(verifyPeer),
	m_verbose(0),
	m_valid(true)
{
	// Potentially could do some certificate validation here...
}

VehicleSignalsConfig::VehicleSignalsConfig(const QString &appname)
{
	m_valid = false;

	QSettings *pSettings = new QSettings("AGL", appname);
	if (!pSettings)
		return;

	m_hostname = pSettings->value("vis-client/server", "localhost").toString();
	if (m_hostname.isEmpty()) {
		qCritical() << "Invalid server hostname";
		return;
	}

	m_port = pSettings->value("vis-client/port", 8090).toInt();
	if (m_port == 0) {
		qCritical() << "Invalid server port";
		return;
	}

	// Default to disabling peer verification for now to be able
        // to use the default upstream KUKSA.val certificates for
	// testing.  Wrangling server and CA certificate generation
	// and management to be able to verify will require further
	// investigation.
	m_verifyPeer = pSettings->value("vis-client/verify-server", false).toBool();

	QString keyFileName = pSettings->value("vis-client/key", DEFAULT_CLIENT_KEY_FILE).toString();
	if (keyFileName.isEmpty()) {
		qCritical() << "Invalid client key filename";
		return;
	}
	QFile keyFile(keyFileName);
	if (!keyFile.open(QIODevice::ReadOnly)) {
		qCritical() << "Could not open client key file";
		return;
	}
	QByteArray keyData = keyFile.readAll();
	if (keyData.isEmpty()) {
		qCritical() << "Invalid client key file";
		return;
	}
	m_clientKey = keyData;

	QString certFileName = pSettings->value("vis-client/certificate", DEFAULT_CLIENT_CERT_FILE).toString();
	if (certFileName.isEmpty()) {
		qCritical() << "Invalid client certificate filename";
		return;
	}
	QFile certFile(certFileName);
	if (!certFile.open(QIODevice::ReadOnly)) {
		qCritical() << "Could not open client certificate file";
		return;
	}
	QByteArray certData = certFile.readAll();
	if (certData.isEmpty()) {
		qCritical() << "Invalid client certificate file";
		return;
	}
	m_clientCert = certData;

	QString caCertFileName = pSettings->value("vis-client/ca-certificate", DEFAULT_CA_CERT_FILE).toString();
	if (caCertFileName.isEmpty()) {
		qCritical() << "Invalid CA certificate filename";
		return;
	}
	QFile caCertFile(caCertFileName);
	if (!caCertFile.open(QIODevice::ReadOnly)) {
		qCritical() << "Could not open CA certificate file";
		return;
	}
	QByteArray caCertData = caCertFile.readAll();
	if (caCertData.isEmpty()) {
		qCritical() << "Invalid CA certificate file";
		return;
	}
	// Pre-check CA certificate
	QList<QSslCertificate> newSslCaCerts = QSslCertificate::fromData(caCertData);
	if (newSslCaCerts.isEmpty()) {
		qCritical() << "Invalid CA certificate";
		return;
	}
	m_caCert = caCertData;

	QString authTokenFileName = pSettings->value("vis-client/authorization").toString();
	if (authTokenFileName.isEmpty()) {
		qCritical() << "Invalid authorization token filename";
		return;
	}
	QFile authTokenFile(authTokenFileName);
	if (!authTokenFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qCritical() << "Could not open authorization token file";
		return;
	}
	QTextStream in(&authTokenFile);
	QString authToken = in.readLine();
	if (authToken.isEmpty()) {
		qCritical() << "Invalid authorization token file";
		return;
	}
	m_authToken = authToken;

	m_verbose = 0;
	QString verbose = pSettings->value("vis-client/verbose").toString();
	if (!verbose.isEmpty()) {
		if (verbose == "true" || verbose == "1")
			m_verbose = 1;
		if (verbose == "2")
			m_verbose = 2;
	}

	m_valid = true;
}

VehicleSignals::VehicleSignals(const VehicleSignalsConfig &config, QObject *parent) :
	QObject(parent),
	m_config(config),
	m_request_id(0)
{
	QObject::connect(&m_websocket, &QWebSocket::connected, this, &VehicleSignals::onConnected);
	QObject::connect(&m_websocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
			 this, &VehicleSignals::onError);
	QObject::connect(&m_websocket, &QWebSocket::disconnected, this, &VehicleSignals::onDisconnected);
}

VehicleSignals::~VehicleSignals()
{
	m_websocket.close();
}

void VehicleSignals::connect()
{
	if (!m_config.valid()) {
		qCritical() << "Invalid VIS server configuration";
		return;
	}

	QUrl visUrl;
	visUrl.setScheme(QStringLiteral("wss"));
	visUrl.setHost(m_config.hostname());
	visUrl.setPort(m_config.port());

	QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();

	// Add client private key
        // i.e. kuksa_certificates/Client.key in source tree
	QSslKey sslKey(m_config.clientKey(), QSsl::Rsa);
	sslConfig.setPrivateKey(sslKey);

	// Add local client certificate
        // i.e. kuksa_certificates/Client.pem in source tree
	QList<QSslCertificate> sslCerts = QSslCertificate::fromData(m_config.clientCert());
	if (sslCerts.empty()) {
		qCritical() << "Invalid client certificate";
		return; 
	}
	sslConfig.setLocalCertificate(sslCerts.first());

	// Add CA certificate
        // i.e. kuksa_certificates/CA.pem in source tree
	// Note the following can be simplified with QSslConfiguration::addCaCertificate with Qt 5.15
	QList<QSslCertificate> sslCaCerts = sslConfig.caCertificates();
	QList<QSslCertificate> newSslCaCerts = QSslCertificate::fromData(m_config.caCert());
	if (newSslCaCerts.empty()) {
		qCritical() << "Invalid CA certificate";
		return;
	}
	sslCaCerts.append(newSslCaCerts.first());
	sslConfig.setCaCertificates(sslCaCerts);

	sslConfig.setPeerVerifyMode(m_config.verifyPeer() ? QSslSocket::VerifyPeer : QSslSocket::VerifyNone);

	m_websocket.setSslConfiguration(sslConfig);

	if (m_config.verbose())
		qInfo() << "Opening VIS websocket";
	m_websocket.open(visUrl);
}

void VehicleSignals::onConnected()
{
	if (m_config.verbose() > 1)
		qDebug() << "VehicleSignals::onConnected: enter";
	QObject::connect(&m_websocket, &QWebSocket::textMessageReceived, this, &VehicleSignals::onTextMessageReceived);
	emit connected();
}

void VehicleSignals::onError(QAbstractSocket::SocketError error)
{
	if (m_config.verbose() > 1)
		qDebug() << "VehicleSignals::onError: enter";
	QTimer::singleShot(1000, this, &VehicleSignals::reconnect);
}

void VehicleSignals::reconnect()
{
	if (m_config.verbose() > 1)
		qDebug() << "VehicleSignals::reconnect: enter";
	connect();
}

void VehicleSignals::onDisconnected()
{
	if (m_config.verbose() > 1)
		qDebug() << "VehicleSignals::onDisconnected: enter";
	QObject::disconnect(&m_websocket, &QWebSocket::textMessageReceived, this, &VehicleSignals::onTextMessageReceived);
	emit disconnected();

	// Try to reconnect
	QTimer::singleShot(1000, this, &VehicleSignals::reconnect);
}

void VehicleSignals::authorize()
{
	QVariantMap map;
	map["action"] = QString("authorize");
	map["tokens"] = m_config.authToken();
	map["requestId"] = QString::number(m_request_id++);
	QJsonDocument doc = QJsonDocument::fromVariant(map);
	m_websocket.sendTextMessage(doc.toJson(QJsonDocument::Compact).data());
}

void VehicleSignals::get(const QString &path)
{
	QVariantMap map;
	map["action"] = QString("get");
	map["tokens"] = m_config.authToken();
	map["path"] = path;
	map["requestId"] = QString::number(m_request_id++);
	QJsonDocument doc = QJsonDocument::fromVariant(map);
	m_websocket.sendTextMessage(doc.toJson(QJsonDocument::Compact).data());
}

void VehicleSignals::set(const QString &path, const QString &value)
{
	QVariantMap map;
	map["action"] = QString("set");
	map["tokens"] = m_config.authToken();
	map["path"] = path;
	map["value"] = value;
	map["requestId"] = QString::number(m_request_id++);
	QJsonDocument doc = QJsonDocument::fromVariant(map);
	m_websocket.sendTextMessage(doc.toJson(QJsonDocument::Compact).data());
}

void VehicleSignals::subscribe(const QString &path)
{
	QVariantMap map;
	map["action"] = QString("subscribe");
	map["tokens"] = m_config.authToken();
	map["path"] = path;
	map["requestId"] = QString::number(m_request_id++);
	QJsonDocument doc = QJsonDocument::fromVariant(map);
	m_websocket.sendTextMessage(doc.toJson(QJsonDocument::Compact).data());
}

bool VehicleSignals::parseData(const QJsonObject &response, QString &path, QString &value, QString &timestamp)
{
	if (response.contains("error")) {
		QString error = response.value("error").toString();
		return false;
	}

	if (!(response.contains("data") && response["data"].isObject())) {
		qWarning() << "Malformed response (data missing)";
		return false;
	}
	QJsonObject data = response["data"].toObject();
	if (!(data.contains("path") && data["path"].isString())) {
		qWarning() << "Malformed response (path missing)";
		return false;
	}
	path = data["path"].toString();
	// Convert '/' to '.' in paths to ensure consistency for clients
	path.replace(QString("/"), QString("."));

	if (!(data.contains("dp") && data["dp"].isObject())) {
		qWarning() << "Malformed response (datapoint missing)";
		return false;
	}
	QJsonObject dp = data["dp"].toObject();
	if (!dp.contains("value")) {
		qWarning() << "Malformed response (value missing)";
		return false;
	} else if (dp["value"].isString()) {
		value = dp["value"].toString();
	} else if (dp["value"].isDouble()) {
		value = QString::number(dp["value"].toDouble(), 'f', 9);
	} else if (dp["value"].isBool()) {
		value = dp["value"].toBool() ? "true" : "false";
	} else {
		qWarning() << "Malformed response (unsupported value type)";
		return false;
	}

	if (!(dp.contains("ts") && dp["ts"].isString())) {
		qWarning() << "Malformed response (timestamp missing)";
		return false;
	}
	timestamp = dp["ts"].toString();

	return true;
}

//
// NOTE:
//
// Ideally request ids would be used to provide some form of mapping
// to callers of get/set for responses/errors.  At present the demo
// usecases are simple enough that it does not seem worth implementing
// just yet.
//
void VehicleSignals::onTextMessageReceived(QString msg)
{
	msg = msg.simplified();
	QJsonDocument doc(QJsonDocument::fromJson(msg.toUtf8()));
	if (doc.isEmpty()) {
		qWarning() << "Received invalid JSON: empty VIS message";
		return;
	}

	if (!doc.isObject()) {
		qWarning() << "Received invalid JSON: malformed VIS message";
		return;
	}
	QJsonObject obj = doc.object();

	if (!obj.contains("action")) {
		qWarning() << "Received unknown message (no action), discarding";
		return;
	}
	
	QString action = obj.value("action").toString();
	if (action == "authorize") {
		if (obj.contains("error")) {
			QString error = obj.value("error").toString();
			qWarning() << "VIS authorization failed: " << error;
		} else {
			if (m_config.verbose() > 1)
				qDebug() << "authorized";
			emit authorized();
		}
	} else if (action == "subscribe") {
		if (obj.contains("error")) {
			QString error = obj.value("error").toString();
			qWarning() << "VIS subscription failed: " << error;
		}
	} else if (action == "get") {
		if (obj.contains("error")) {
			QString error = obj.value("error").toString();
			qWarning() << "VIS get failed: " << error;
		} else {
			QString path, value, ts;
			if (parseData(obj, path, value, ts)) {
				if (m_config.verbose() > 1)
					qDebug() << "VehicleSignals::onTextMessageReceived: emitting response" << path << " = " << value;
				emit getSuccessResponse(path, value, ts);
			}
		}
	} else if (action == "set") {
		if (obj.contains("error")) {
			QString error = obj.value("error").toString();
			qWarning() << "VIS set failed: " << error;
		}
	} else if (action == "subscription") {
		QString path, value, ts;
		if (parseData(obj, path, value, ts)) {
			if (m_config.verbose() > 1)
				qDebug() << "VehicleSignals::onTextMessageReceived: emitting notification" << path << " = " << value;
			emit signalNotification(path, value, ts);
		}
	} else {
		qWarning() << "unhandled VIS response of type: " << action;
	}
}
