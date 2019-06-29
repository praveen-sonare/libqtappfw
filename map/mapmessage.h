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

#ifndef MAP_MESSAGE_H
#define MAP_MESSAGE_H

#include "message.h"

class MapMessage : public Message
{
    Q_OBJECT
    public:
        bool createRequest(QString verb, QJsonObject parameter);
        bool isNotificationEvent() {return (this->eventName() == "notification"); };

    private:
        QStringList verbs {
            "compose",
            "message",
            "list_messages",
            "subscribe",
            "unsubscribe",
        };
};

#endif // MAP_MESSAGE_H
