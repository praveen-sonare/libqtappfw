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

#ifndef RESPONSEMESSAGE_H
#define RESPONSEMESSAGE_H

#include <QObject>

#include "message.h"

class ResponseMessage : public Message
{
    Q_OBJECT

    public:
        explicit ResponseMessage(QByteArray request = nullptr);

        inline QString requestVerb() const
        {
            return m_request["verb"].toString();
        }

        inline QVariantMap requestParameters() const
        {
            return m_request["parameter"].toMap();
        }
};

#endif // RESPONSEMESSAGE_H
