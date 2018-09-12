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
}

Bluetooth::~Bluetooth()
{
    delete m_mloop;
}

void Bluetooth::generic_command(QString verb, QString value)
{
    BluetoothMessage *tmsg = new BluetoothMessage();
    QJsonObject parameter;

    if (!value.isEmpty())
        parameter.insert("value", value);

    tmsg->createRequest(verb, parameter);
    m_mloop->sendMessage(tmsg);
    delete tmsg;
}

void Bluetooth::setPower(bool state)
{
    generic_command("power", state ? "true": "false");

    m_power = state;

    emit powerChanged(m_power);
}

void Bluetooth::setDiscoverable(bool state)
{
    const QStringList properties { "Pairable", "Discoverable" };
    QStringListIterator propertyIterator(properties);

    while (propertyIterator.hasNext()) {
        BluetoothMessage *tmsg = new BluetoothMessage();
        QJsonObject parameter;

        parameter.insert("Property", propertyIterator.next());
        parameter.insert("value", state ? "true" : "false");

        tmsg->createRequest("set_property", parameter);
        m_mloop->sendMessage(tmsg);
        delete tmsg;
    }

    m_discoverable = state;

    emit discoverableChanged();
}

void Bluetooth::start_discovery()
{
    generic_command("start_discovery", "");
    generic_command("discovery_result", "");
}

void Bluetooth::stop_discovery()
{
    generic_command("stop_discovery", "");
}

void Bluetooth::remove_device(QString address)
{
    generic_command("remove_device", address);
}

void Bluetooth::pair(QString address)
{
    generic_command("pair", address);
}

void Bluetooth::cancel_pair(QString address)
{
    generic_command("cancel_pair", address);
}

void Bluetooth::connect(QString address, QString uuid)
{
    BluetoothMessage *tmsg = new BluetoothMessage();
    QJsonObject parameter;

    uuid = process_uuid(uuid);

    parameter.insert("value", address);
    parameter.insert("uuid", uuid);
    tmsg->createRequest("connect", parameter);
    m_mloop->sendMessage(tmsg);
    delete tmsg;
}

void Bluetooth::connect(QString address)
{
    generic_command("connect", address);
}

void Bluetooth::disconnect(QString address, QString uuid)
{
    BluetoothMessage *tmsg = new BluetoothMessage();
    QJsonObject parameter;

    uuid = process_uuid(uuid);

    parameter.insert("value", address);
    parameter.insert("uuid", uuid);
    tmsg->createRequest("disconnect", parameter);
    m_mloop->sendMessage(tmsg);
    delete tmsg;
}

void Bluetooth::disconnect(QString address)
{
    generic_command("disconnect", address);
}

void Bluetooth::send_confirmation()
{
    generic_command("send_confirmation", "yes");
}

void Bluetooth::set_avrcp_controls(QString address, QString cmd)
{
    BluetoothMessage *tmsg = new BluetoothMessage();
    QJsonObject parameter;

    parameter.insert("Address", address);
    parameter.insert("value", cmd);
    tmsg->createRequest("set_avrcp_controls", parameter);
    m_mloop->sendMessage(tmsg);
    delete tmsg;
}

void Bluetooth::onConnected()
{
    QStringListIterator eventIterator(events);
    BluetoothMessage *tmsg;

    while (eventIterator.hasNext()) {
        tmsg = new BluetoothMessage();
        QJsonObject parameter;
        parameter.insert("value", eventIterator.next());
        tmsg->createRequest("subscribe", parameter);
        m_mloop->sendMessage(tmsg);
        delete tmsg;
    }

    // get initial power state
    generic_command("power", QString());

    // send initial list
    generic_command("discovery_result", "");
}

void Bluetooth::onDisconnected()
{
    QStringListIterator eventIterator(events);
    BluetoothMessage *tmsg;

    while (eventIterator.hasNext()) {
        tmsg = new BluetoothMessage();
        QJsonObject parameter;
        parameter.insert("value", eventIterator.next());
        tmsg->createRequest("unsubscribe", parameter);
        m_mloop->sendMessage(tmsg);
        delete tmsg;
    }
}

void Bluetooth::onMessageReceived(MessageType type, Message *msg)
{
    if (msg->isEvent() && type == BluetoothEventMessage) {
        BluetoothMessage *tmsg = qobject_cast<BluetoothMessage*>(msg);

        if (tmsg->isConnectionEvent()) {
            emit connectionEvent(tmsg->eventData());
        } else if (tmsg->isRequestConfirmationEvent()) {
            emit requestConfirmationEvent(tmsg->eventData());
        } else if (tmsg->isDeviceAddedEvent()) {
            emit deviceAddedEvent(tmsg->eventData());
        } else if (tmsg->isDeviceRemovedEvent()) {
            emit deviceRemovedEvent(tmsg->eventData());
        } else if (tmsg->isDeviceUpdatedEvent()) {
            emit deviceUpdatedEvent(tmsg->eventData());
        }
    } else if (msg->isReply() && type == ResponseRequestMessage) {
        ResponseMessage *tmsg = qobject_cast<ResponseMessage*>(msg);

        if (tmsg->requestVerb() == "discovery_result") {
            emit deviceListEvent(tmsg->replyData());
        } else if (tmsg->requestVerb() == "power") {
            m_power = tmsg->replyData().value("power").toString() == "on";
            emit powerChanged(m_power);
        }
    }

    msg->deleteLater();
}
