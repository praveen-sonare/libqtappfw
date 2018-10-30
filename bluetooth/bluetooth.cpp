/*
 * Copyright (C) 2018 Konsulko Group
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

#include "message.h"
#include "messageengine.h"
#include "bluetooth.h"
#include "bluetoothmessage.h"
#include "responsemessage.h"

Bluetooth::Bluetooth (QUrl &url, QObject * parent) :
    QObject(parent),
    m_mloop(nullptr)
{
    m_mloop = new MessageEngine(url);
    QObject::connect(m_mloop, &MessageEngine::connected, this, &Bluetooth::onConnected);
    QObject::connect(m_mloop, &MessageEngine::disconnected, this, &Bluetooth::onDisconnected);
    QObject::connect(m_mloop, &MessageEngine::messageReceived, this, &Bluetooth::onMessageReceived);

    uuids.insert("a2dp", "0000110a-0000-1000-8000-00805f9b34fb");
    uuids.insert("avrcp", "0000110e-0000-1000-8000-00805f9b34fb");
    uuids.insert("hfp", "0000111f-0000-1000-8000-00805f9b34fb");
}

Bluetooth::~Bluetooth()
{
    delete m_mloop;
}

void Bluetooth::send_command(QString verb, QJsonObject parameter)
{
    BluetoothMessage *tmsg = new BluetoothMessage();
    tmsg->createRequest(verb, parameter);
    m_mloop->sendMessage(tmsg);
    delete tmsg;
}

void Bluetooth::setPower(bool state)
{
    QJsonObject parameter;
    parameter.insert("powered", state ? "true" : "false");
    send_command("adapter_state", parameter);
}

void Bluetooth::setDiscoverable(bool state)
{
    QJsonObject parameter;
    parameter.insert("discoverable", state ? "true" : "false");
    send_command("adapter_state", parameter);

    m_discoverable = state;

    emit discoverableChanged();
}

void Bluetooth::discovery_command(bool state)
{
    QJsonObject parameter;
    parameter.insert("discovery", state ? "true" : "false");

    set_discovery_filter();

    send_command("adapter_state", parameter);
}

void Bluetooth::start_discovery()
{
    discovery_command(true);

    // temp workaround to list already discovered devices
    send_command("managed_objects", QJsonObject());
}

void Bluetooth::stop_discovery()
{
    discovery_command(false);
}

void Bluetooth::remove_device(QString device)
{
    QJsonObject parameter;
    parameter.insert("device", device);

    send_command("remove_device", parameter);
}

void Bluetooth::pair(QString device)
{
    QJsonObject parameter;
    parameter.insert("device", device);

    send_command("pair", parameter);
}

void Bluetooth::cancel_pair(QString device)
{
    send_command("cancel_pairing", QJsonObject());
}

void Bluetooth::connect(QString device, QString uuid)
{
    QJsonObject parameter;
    uuid = process_uuid(uuid);
    parameter.insert("device", device);
    parameter.insert("uuid", uuid);
    send_command("connect", parameter);
}

void Bluetooth::connect(QString device)
{
    QJsonObject parameter;
    parameter.insert("device", device);
    send_command("connect", parameter);
}

void Bluetooth::disconnect(QString device, QString uuid)
{
    QJsonObject parameter;
    uuid = process_uuid(uuid);
    parameter.insert("device", device);
    parameter.insert("uuid", uuid);
    send_command("disconnect", parameter);
}

void Bluetooth::disconnect(QString device)
{
    QJsonObject parameter;
    parameter.insert("device", device);
    send_command("disconnect", parameter);
}

void Bluetooth::send_confirmation(int pincode)
{
    QJsonObject parameter;
    parameter.insert("pincode", pincode);
    send_command("confirm_pairing", parameter);
}


void Bluetooth::set_discovery_filter()
{
    QStringListIterator eventIterator(uuids.values());
    QJsonObject parameter;
    QJsonArray array;

    while (eventIterator.hasNext())
        array.push_back(eventIterator.next());

    // send inital adapter state + discovery filter
    parameter.insert("filter", array);
    send_command("adapter_state", parameter);
}

void Bluetooth::onConnected()
{
    QStringListIterator eventIterator(events);

    while (eventIterator.hasNext()) {
        QJsonObject parameter;
        parameter.insert("value", eventIterator.next());
        send_command("subscribe", parameter);
    }

    // send initial list
    send_command("managed_objects", QJsonObject());

    // get initial power state
    send_command("adapter_state", QJsonObject());
}

void Bluetooth::onDisconnected()
{
    QStringListIterator eventIterator(events);

    while (eventIterator.hasNext()) {
        QJsonObject parameter;
        parameter.insert("value", eventIterator.next());
        send_command("unsubscribe", parameter);
    }
}

void Bluetooth::processDeviceChangesEvent(QJsonObject data)
{
    QString action = data.value("action").toString();

    if (action == "added")
        emit deviceAddedEvent(data);
    else if (action == "changed")
        emit deviceUpdatedEvent(data);
    else if (action == "removed")
        emit deviceRemovedEvent(data);
}

void Bluetooth::onMessageReceived(MessageType type, Message *msg)
{
    if (msg->isEvent() && type == BluetoothEventMessage) {
        BluetoothMessage *tmsg = qobject_cast<BluetoothMessage*>(msg);

        if (tmsg->isDeviceChangesEvent()) {
            processDeviceChangesEvent(tmsg->eventData());
        } else if (tmsg->isAgentEvent()) {
            emit requestConfirmationEvent(tmsg->eventData());
        }

    } else if (msg->isReply() && type == ResponseRequestMessage) {
        ResponseMessage *tmsg = qobject_cast<ResponseMessage*>(msg);

        if (tmsg->requestVerb() == "managed_objects") {
            emit deviceListEvent(tmsg->replyData());
        } else if (tmsg->requestVerb() == "adapter_state") {
            m_power = tmsg->replyData().value("powered").toBool();
            emit powerChanged(m_power);
        }
    }

    msg->deleteLater();
}
