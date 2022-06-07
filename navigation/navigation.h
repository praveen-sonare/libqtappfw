 /*
 * Copyright (C) 2019-2022 Konsulko Group
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

#ifndef NAVIGATION_H
#define NAVIGATION_H

#include <QObject>
#include <QVariant>

class VehicleSignals;

class Navigation : public QObject
{
	Q_OBJECT

public:
	explicit Navigation(VehicleSignals *vs, QObject *parent = Q_NULLPTR);
	virtual ~Navigation();

	Q_INVOKABLE void broadcastPosition(double lat, double lon, double drc, double dst);
	Q_INVOKABLE void broadcastRouteInfo(double lat, double lon, double route_lat, double route_lon);
	Q_INVOKABLE void broadcastStatus(QString state);

	// only support one waypoint for now
	Q_INVOKABLE void sendWaypoint(double lat, double lon);

signals:
	void statusEvent(QVariantMap data);
	void positionEvent(QVariantMap data);
	void waypointsEvent(QVariantMap data);

private slots:
	void onConnected();
	void onAuthorized();
	void onDisconnected();
	void onSignalNotification(QString path, QString value, QString timestamp);

private:
	VehicleSignals *m_vs;
	bool m_connected;
	double m_latitude;
	double m_longitude;
	double m_heading;
	double m_distance;
	double m_dest_latitude;
	double m_dest_longitude;
};

#endif // NAVIGATION_H
