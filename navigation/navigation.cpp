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

#include <QDebug>

#include "navigation.h"
#include "vehiclesignals.h"

Navigation::Navigation(VehicleSignals *vs, QObject * parent) :
	QObject(parent),
	m_vs(vs),
	m_connected(false)

{
	QObject::connect(m_vs, &VehicleSignals::connected, this, &Navigation::onConnected);
	QObject::connect(m_vs, &VehicleSignals::authorized, this, &Navigation::onAuthorized);
	QObject::connect(m_vs, &VehicleSignals::disconnected, this, &Navigation::onDisconnected);

	if (m_vs)
		m_vs->connect();
}

Navigation::~Navigation()
{
	delete m_vs;
}

void Navigation::sendWaypoint(double lat, double lon)
{
	if (!(m_vs && m_connected))
		return;

	// The original implementation resulted in at least 9 decimal places
	// making it through to the clients, so explicitly format for 9.  In
	// practice going from the QString default 6 to 9 does make a difference
	// with respect to smoothness in the position-based map rotations done
	// in tbtnavi.
	m_vs->set("Vehicle.Cabin.Infotainment.Navigation.DestinationSet.Latitude", QString::number(lat, 'f', 9));
	m_vs->set("Vehicle.Cabin.Infotainment.Navigation.DestinationSet.Longitude", QString::number(lon, 'f', 9));
}

void Navigation::broadcastPosition(double lat, double lon, double drc, double dst)
{
	if (!(m_vs && m_connected))
		return;

	m_vs->set("Vehicle.CurrentLocation.Latitude", QString::number(lat, 'f', 9));
	m_vs->set("Vehicle.CurrentLocation.Longitude", QString::number(lon, 'f', 9));
	m_vs->set("Vehicle.CurrentLocation.Heading", QString::number(drc, 'f', 9));

	// NOTES:
	// - This signal is an AGL addition, it may make sense to engage with the
	//   VSS specification upstream to discuss an addition along these lines.
	// - The signal makes more sense in kilometers wrt VSS expectations, so
	//   conversion from meters happens here for now to avoid changing the
	//   existing clients.  This may be worth revisiting down the road.
	m_vs->set("Vehicle.Cabin.Infotainment.Navigation.ElapsedDistance", QString::number(dst / 1000, 'f', 9));
}

void Navigation::broadcastRouteInfo(double lat, double lon, double route_lat, double route_lon)
{
	if (!(m_vs && m_connected))
		return;

	m_vs->set("Vehicle.CurrentLocation.Latitude", QString::number(lat, 'f', 9));
	m_vs->set("Vehicle.CurrentLocation.Longitude", QString::number(lon, 'f', 9));

	m_vs->set("Vehicle.Cabin.Infotainment.Navigation.DestinationSet.Latitude", QString::number(route_lat, 'f', 9));
	m_vs->set("Vehicle.Cabin.Infotainment.Navigation.DestinationSet.Longitude", QString::number(route_lon, 'f', 9));
}

void Navigation::broadcastStatus(QString state)
{
	if (!(m_vs && m_connected))
		return;

	m_vs->set("Vehicle.Cabin.Infotainment.Navigation.State", state);
}

void Navigation::onConnected()
{
	if (!m_vs)
		return;

	m_vs->authorize();
}

void Navigation::onAuthorized()
{
	if (!m_vs)
		return;

	m_connected = true;

	QObject::connect(m_vs, &VehicleSignals::signalNotification, this, &Navigation::onSignalNotification);

	// NOTE: This signal is another AGL addition where it is possible
	//       upstream may be open to adding it to VSS.
	m_vs->subscribe("Vehicle.Cabin.Infotainment.Navigation.State");
	m_vs->subscribe("Vehicle.CurrentLocation.Latitude");
	m_vs->subscribe("Vehicle.CurrentLocation.Longitude");
	m_vs->subscribe("Vehicle.CurrentLocation.Heading");
	m_vs->subscribe("Vehicle.Cabin.Infotainment.Navigation.DestinationSet.Latitude");
	m_vs->subscribe("Vehicle.Cabin.Infotainment.Navigation.DestinationSet.Longitude");
	m_vs->subscribe("Vehicle.Cabin.Infotainment.Navigation.ElapsedDistance");
}

void Navigation::onDisconnected()
{
	QObject::disconnect(m_vs, &VehicleSignals::signalNotification, this, &Navigation::onSignalNotification);

	m_connected = false;
}

void Navigation::onSignalNotification(QString path, QString value, QString timestamp)
{
	// NOTE: Since all the known AGL users of the VSS signals are users of
	//       this API, we know that updates occur in certain sequences and
	//       can leverage this to roll up for emitting the existing events.
	//       This is the path of least effort with respect to changing
	//       the existing clients, but it may make sense down the road to
	//       either switch them to using VehicleSignals directly or having
	//       a more granular signal scheme that maps more directly onto
	//       VSS.
	if (path == "Vehicle.Cabin.Infotainment.Navigation.State") {
		QVariantMap event;
		event["state"] = value;
		emit statusEvent(event);
	} else if (path == "Vehicle.CurrentLocation.Latitude") {
		m_latitude = value.toDouble();
	} else if (path == "Vehicle.CurrentLocation.Longitude") {
		m_longitude = value.toDouble();
	} else if (path == "Vehicle.CurrentLocation.Heading") {
		m_heading = value.toDouble();
	} else if (path == "Vehicle.Cabin.Infotainment.Navigation.ElapsedDistance") {
		m_distance = value.toDouble();
		QVariantMap event;
		event["position"] = "car";
		event["latitude"] = m_latitude;
		event["longitude"] = m_longitude;
		event["direction"] = m_heading;
		event["distance"] = m_distance * 1000;
		emit positionEvent(event);
	} else if (path == "Vehicle.Cabin.Infotainment.Navigation.DestinationSet.Latitude") {
		m_dest_latitude = value.toDouble();
	} else if (path == "Vehicle.Cabin.Infotainment.Navigation.DestinationSet.Longitude") {
		m_dest_longitude = value.toDouble();
		QVariantMap event;
		event["position"] = "route";
		event["latitude"] = m_latitude;
		event["longitude"] = m_longitude;
		event["route_latitude"] = m_dest_latitude;
		event["route_longitude"] = m_dest_longitude;
		emit positionEvent(event);

		// NOTE: Potentially could emit a waypointsEvent here, but
		//       nothing in the demo currently requires it, so do
		//       not bother for now.  If something like Alexa is
		//       added it or some other replacement / rework will
		//       be required.
	}
}
