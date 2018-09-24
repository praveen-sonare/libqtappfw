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

#ifndef BLUETOOTH_MESSAGE_H
#define BLUETOOTH_MESSAGE_H

#include "message.h"

class BluetoothMessage : public Message
{
    Q_OBJECT
    public:
        bool isConnectionEvent() { return (this->eventName() == "connection"); };
        bool isRequestConfirmationEvent() { return (this->eventName() == "request_confirmation"); };
        bool isDeviceAddedEvent() { return (this->eventName() == "device_added"); };
        bool isDeviceRemovedEvent() { return (this->eventName() == "device_removed"); };
        bool isDeviceUpdatedEvent() { return (this->eventName() == "device_updated"); };
        bool createRequest(QString verb, QJsonObject parameter);

    private:
        QStringList verbs {
            "start_discovery" ,    "stop_discovery",    "power",
            "remove_device",       "pair",              "cancel_pair",
            "connect",             "disconnect",        "device_priorites",
            "set_device_property", "set_property",      "discovery_result",
            "set_avrcp_controls",  "send_confirmation", "version",
            "subscribe",           "unsubscribe",
        };
};

#endif // BLUETOOTH_MESSAGE_H
