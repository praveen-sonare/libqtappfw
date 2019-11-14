/*
 * Copyright (C) 2019 Konsulko Group
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

#ifndef NAVIGATION_MESSAGE_H
#define NAVIGATION_MESSAGE_H

#include "message.h"

class NavigationMessage : public Message
{
    Q_OBJECT
    public:
        bool createRequest(QString verb, QJsonObject parameter);
        bool isStatusEvent() { return (this->eventName() == "status"); };
        bool isPositionEvent() { return (this->eventName() == "position"); };
        bool isWaypointsEvent() { return (this->eventName() == "waypoints"); };

    private:
        QStringList verbs {
            "broadcast_status",
            "broadcast_position",
            "broadcast_waypoints",
            "subscribe",
            "unsubscribe",
        };
};

#endif // NAVIGATION_MESSAGE_H
