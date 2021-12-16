/*
 * Copyright (C) 2021 Konsulko Group
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
#include <bluez-glib.h>

#include "bluetootheventhandler.h"
#include "bluetooth.h"
#include "bluetoothmodel.h"


BluetoothEventHandler::BluetoothEventHandler (Bluetooth *parent, bool register_agent) :
	m_parent(parent),
	m_agent(register_agent)
{
	bluez_add_adapter_event_callback(adapter_event_cb, this);
	bluez_add_device_event_callback(device_event_cb, this);
	if (register_agent)
		bluez_add_agent_event_callback(agent_event_cb, this);
}

BluetoothEventHandler::~BluetoothEventHandler()
{
}

void BluetoothEventHandler::handle_init_event(gchar *adapter, gboolean status)
{
	if (status)
		m_parent->init_adapter_state(QString(adapter));
	else
		qCritical() << "BlueZ initialization failed";
}

void BluetoothEventHandler::handle_adapter_event(gchar *adapter,
						 bluez_event_t event,
						 GVariant *properties)
{
	if (!adapter || event != BLUEZ_EVENT_CHANGE)
		return;

	GVariantDict *props_dict = g_variant_dict_new(properties);
	if (!props_dict)
		return;

	gboolean powered = FALSE;
	if (!g_variant_dict_lookup(props_dict, "Powered", "b", &powered)) {
		g_variant_dict_unref(props_dict);
		return;
	}

	g_variant_dict_unref(props_dict);

	m_parent->update_adapter_power(powered);
}

void BluetoothEventHandler::handle_device_event(gchar *adapter,
						gchar *device,
						bluez_event_t event,
						GVariant *properties)
{
	if (!device)
		return;

	BluetoothDevice *model_device = m_parent->m_bluetooth->getDevice(QString(device));
	if (event == BLUEZ_EVENT_REMOVE) {
		if(model_device != nullptr)
			m_parent->m_bluetooth->removeDevice(model_device);

		return;
	}

	BluetoothDevice *new_device = m_parent->m_bluetooth->updateDeviceProperties(model_device, device, properties);
	if (new_device == nullptr) {
		qCritical() << "Failed to create device object with id: " << QString(device);
		return;
	}
	if (model_device == nullptr && event == BLUEZ_EVENT_ADD) {
		// device not previously in model
		m_parent->m_bluetooth->addDevice(new_device);
	}
}

void BluetoothEventHandler::handle_agent_event(gchar *device,
					       bluez_agent_event_t event,
					       GVariant *properties)
{
	const gchar *path = NULL;
	int pincode;

	if (event == BLUEZ_AGENT_EVENT_REQUEST_CONFIRMATION) {
		g_variant_get(properties, "(ou)", &path, &pincode);
		if (path)
			m_parent->request_confirmation(pincode);
	}
}

void BluetoothEventHandler::handle_connect_event(gchar *device, gboolean status)
{
	if (!status)
		qDebug() << "connect failed";
}

void BluetoothEventHandler::handle_pair_event(gchar *device, gboolean status)
{
	if (!status)
		qDebug() << "pairing failed";
}

