/*
 * Copyright (C) 2018-2021 Konsulko Group
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

#include "bluetooth.h"
#include "bluetoothmodel.h"
#include "bluetootheventhandler.h"


Bluetooth::Bluetooth (bool register_agent, QQmlContext *context, QObject * parent) :
	QObject(parent),
	m_context(context),
	m_agent(register_agent)
{
	m_bluetooth = new BluetoothModel();
	BluetoothModelFilter *m_model = new BluetoothModelFilter();
	m_model->setSourceModel(m_bluetooth);
	m_model->setFilterFixedString("true");
	context->setContextProperty("BluetoothPairedModel", m_model);

	m_model = new BluetoothModelFilter();
	m_model->setSourceModel(m_bluetooth);
	m_model->setFilterFixedString("false");
	context->setContextProperty("BluetoothDiscoveryModel", m_model);

	m_event_handler = new BluetoothEventHandler(this, register_agent);

	uuids.insert("a2dp", "0000110a-0000-1000-8000-00805f9b34fb");
	uuids.insert("avrcp", "0000110e-0000-1000-8000-00805f9b34fb");
	uuids.insert("hfp", "0000111f-0000-1000-8000-00805f9b34fb");
}

Bluetooth::~Bluetooth()
{
}

void Bluetooth::setPower(bool state)
{
	bluez_adapter_set_powered(NULL, state ? TRUE : FALSE);
}

void Bluetooth::setDiscoverable(bool state)
{
	bluez_adapter_set_discoverable(NULL, state);

	m_discoverable = state;

	emit discoverableChanged();
}

void Bluetooth::start()
{
	bluez_init(m_agent, m_agent, m_event_handler->init_cb, m_event_handler);	
}

void Bluetooth::discovery_command(bool state)
{
	set_discovery_filter();

	bluez_adapter_set_discovery(NULL, state ? TRUE : FALSE);
}

void Bluetooth::start_discovery()
{
	discovery_command(true);
}

void Bluetooth::stop_discovery()
{
	discovery_command(false);
}

void Bluetooth::remove_device(QString device)
{
	QByteArray device_ba = device.toLocal8Bit();
	const char *device_cstr = device_ba.data();

	bluez_device_remove(device_cstr);
}

void Bluetooth::pair(QString device)
{
	QByteArray device_ba = device.toLocal8Bit();
	const char *device_cstr = device_ba.data();

	bluez_device_pair(device_cstr, m_event_handler->device_pair_cb, m_event_handler);
}

void Bluetooth::cancel_pair(void)
{
	bluez_cancel_pairing();
}

void Bluetooth::connect(QString device, QString uuid)
{
	QByteArray device_ba = device.toLocal8Bit();
	const char *device_cstr = device_ba.data();

	uuid = process_uuid(uuid);
	QByteArray uuid_ba = uuid.toLocal8Bit();
	const char *uuid_cstr = uuid_ba.data();

	bluez_device_connect(device_cstr,
			     uuid_cstr,
			     m_event_handler->device_connect_cb, 
			     m_event_handler); 
}

void Bluetooth::connect(QString device)
{
	QByteArray device_ba = device.toLocal8Bit();
	const char *device_cstr = device_ba.data();

	bluez_device_connect(device_cstr,
			     NULL,
			     m_event_handler->device_connect_cb, 
			     m_event_handler); 
}

void Bluetooth::disconnect(QString device, QString uuid)
{
	QByteArray device_ba = device.toLocal8Bit();
	const char *device_cstr = device_ba.data();

	uuid = process_uuid(uuid);
	QByteArray uuid_ba = uuid.toLocal8Bit();
	const char *uuid_cstr = uuid_ba.data();

	bluez_device_disconnect(device_cstr, uuid_cstr);
}

void Bluetooth::disconnect(QString device)
{
	QByteArray device_ba = device.toLocal8Bit();
	const char *device_cstr = device_ba.data();

	bluez_device_disconnect(device_cstr, NULL);
}

void Bluetooth::send_confirmation(int pincode)
{
	QString pincode_str;
	pincode_str.setNum(pincode);
	QByteArray pincode_ba = pincode_str.toLocal8Bit();
	const char *pincode_cstr = pincode_ba.data();

	bluez_confirm_pairing(pincode_cstr);
}

void Bluetooth::init_adapter_state(QString adapter)
{
	// Get initial power state
	GVariant *reply = NULL;
	gboolean rc = bluez_adapter_get_state(NULL, &reply);
	if (rc && reply) {
		GVariantDict *props_dict = g_variant_dict_new(reply);
		gboolean powered = FALSE;
	        if (g_variant_dict_lookup(props_dict, "Powered", "b", &powered)) {
			if (m_power != powered) {
				m_power = powered;
				emit powerChanged(m_power);
			}
		}
		g_variant_dict_unref(props_dict);
		g_variant_unref(reply);
        }

	// Get initial device list
	refresh_device_list();
}

void Bluetooth::refresh_device_list(void)
{
	gboolean rc;
	GVariant *reply = NULL;

	rc = bluez_adapter_get_devices(NULL, &reply);
	if(!rc)
		return;

	m_bluetooth->removeAllDevices();

	GVariantIter *array = NULL;
	g_variant_get(reply, "a{sv}", &array);
	const gchar *key = NULL;
	GVariant *var = NULL;
	while (g_variant_iter_next(array, "{&sv}", &key, &var)) {
		BluetoothDevice *device = m_bluetooth->updateDeviceProperties(nullptr, key, var);
		if (device)
			m_bluetooth->addDevice(device);

		g_variant_unref(var);
	}
	g_variant_iter_free(array);
	g_variant_unref(reply);
}

void Bluetooth::set_discovery_filter(void)
{
	QList<QString> values = uuids.values();
	QStringListIterator eventIterator(values);

	gchar **uuids_array = (gchar**) g_malloc0((values.count() + 1) * sizeof(gchar*));
	int i = 0;
	while (eventIterator.hasNext()) {
		QByteArray uuid_ba = eventIterator.next().toLocal8Bit();
		gchar *uuid_cstr = g_strdup(uuid_ba.data());
		uuids_array[i++] = uuid_cstr;
	}

	gchar *transport = g_strdup("bredr");
	bluez_adapter_set_discovery_filter(NULL, uuids_array, transport);

	for (i = 0; i < values.count(); i++)
		g_free(uuids_array[i]);
	g_free(uuids_array);
	g_free(transport);
}

void Bluetooth::update_adapter_power(bool powered)
{
	if (!powered)
		m_bluetooth->removeAllDevices();

	if (m_power != powered) {
		m_power = powered;
		emit powerChanged(powered);

		if (powered)
			refresh_device_list();
	}


	if (!m_power) {
		bool discoverable = m_discoverable;
		m_discoverable = false;
		if (discoverable != m_discoverable)
			emit discoverableChanged();
	}
}

void Bluetooth::request_confirmation(int pincode)
{
	QString pincode_str;
	pincode_str.setNum(pincode);
	emit requestConfirmationEvent(pincode_str);
}
