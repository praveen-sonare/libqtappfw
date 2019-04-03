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
        bool isDeviceChangesEvent() { return (this->eventName() == "device_changes"); };
        bool isAdapterChangesEvent() { return (this->eventName() == "adapter_changes"); };
        bool isAgentEvent() { return (this->eventName() == "agent"); };
        bool createRequest(QString verb, QJsonObject parameter);

    private:
        QStringList verbs {
            "connect",             "disconnect",        "managed_objects",
            "adapter_state",       "pair",              "cancel_pairing",
            "confirm_pairing",     "remove_device",     "version",
            "subscribe",           "unsubscribe",
        };
};

#endif // BLUETOOTH_MESSAGE_H
