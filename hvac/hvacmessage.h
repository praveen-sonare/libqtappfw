/*
 * Copyright (C) 2020 Konsulko Group
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

#ifndef HVAC_MESSAGE_H
#define HVAC_MESSAGE_H

#include "message.h"

class HVACMessage : public Message
{
    Q_OBJECT
    public:
        bool createRequest(QString verb, QJsonObject parameter);
        bool isLanguageEvent() {return (this->eventName() == "language"); };

    private:
        QStringList verbs {
            "get_fanspeed",
            "get_temp_left_zone",
            "get_temp_right_zone",
            "temp_left_zone_led",
            "temp_right_zone_led",
            "get",
            "set",
        };
};

#endif // HVAC_MESSAGE_H
