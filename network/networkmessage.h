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

#ifndef NETWORK_MESSAGE_H
#define NETWORK_MESSAGE_H

#include "message.h"

class NetworkMessage : public Message
{
    Q_OBJECT
    public:
        bool createRequest(QString verb, QJsonObject parameter);
        bool isStatusEvent() {return (this->eventName() == "status"); };
        bool connected() { return m_event_data.find("connected").value().toBool(); };

    private:
        QStringList verbs {
            "state",
            "offline",
            "technologies",
            "enable_technology",
            "disable_technology",
            "services",
            "scan_services",
            "move_service",
            "remove_service",
            "connect_service",
            "disconnect_service",
            "get_property",
            "set_property",
            "agent_response",
            "reset_counters",
            "subscribe",
            "unsubscribe",
        };
};

#endif // NETWORK_MESSAGE_H
