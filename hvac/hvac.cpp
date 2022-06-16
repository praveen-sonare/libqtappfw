/*
 * Copyright (C) 2020-2021 Konsulko Group
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
#include "hvac.h"
#include "vehiclesignals.h"


// TODO: don't duplicate defaults from HVAC service here
HVAC::HVAC(VehicleSignals *vs, QObject * parent) :
	QObject(parent),
	m_vs(vs),
	m_connected(false),
	m_fanspeed(0),
	m_temp_left_zone(21),
	m_temp_right_zone(21)
{
	QObject::connect(m_vs, &VehicleSignals::connected, this, &HVAC::onConnected);
	QObject::connect(m_vs, &VehicleSignals::authorized, this, &HVAC::onAuthorized);
	QObject::connect(m_vs, &VehicleSignals::disconnected, this, &HVAC::onDisconnected);

	if (m_vs)
		m_vs->connect();
}

HVAC::~HVAC()
{
	delete m_vs;
}

void HVAC::set_fanspeed(int speed)
{
	if (!(m_vs && m_connected))
		return;

	// Scale incoming 0-255 speed to 0-100 to match VSS signal
	double value = (speed % 256) * 100.0 / 255.0;
	m_vs->set("Vehicle.Cabin.HVAC.Station.Row1.Left.FanSpeed", QString::number((int) (value + 0.5)));
	emit fanSpeedChanged(speed);
}

void HVAC::set_temp_left_zone(int temp)
{
	if (!(m_vs && m_connected))
		return;

	// Make sure value is within VSS signal range
	int value = temp;
	if (value > 50)
		value = 50;
	else if (value < -50)
		value = -50;
	m_vs->set("Vehicle.Cabin.HVAC.Station.Row1.Left.Temperature", QString::number(value));
	emit leftTemperatureChanged(temp);
}

void HVAC::set_temp_right_zone(int temp)
{
	if (!(m_vs && m_connected))
		return;

	// Make sure value is within VSS signal range
	int value = temp;
	if (value > 50)
		value = 50;
	else if (value < -50)
		value = -50;
	m_vs->set("Vehicle.Cabin.HVAC.Station.Row1.Right.Temperature", QString::number(value));
	emit rightTemperatureChanged(temp);
}

void HVAC::onConnected()
{
	if (!m_vs)
		return;

	m_vs->authorize();
}

void HVAC::onAuthorized()
{
	if (!m_vs)
		return;

	// Could subscribe and connect notification signal here to monitor
	// external updates...

	m_connected = true;
}

void HVAC::onDisconnected()
{
	m_connected = false;
}
