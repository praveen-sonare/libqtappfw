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

#ifndef HVAC_H
#define HVAC_H

#include <QDebug>
#include <QObject>
#include <QJsonArray>
#include <QtQml/QQmlContext>
#include <QtQml/QQmlListProperty>

#include "messageengine.h"

class HVAC : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int fanSpeed READ get_fanspeed WRITE set_fanspeed NOTIFY fanSpeedChanged)
    Q_PROPERTY(int leftTemperature READ get_temp_left_zone WRITE set_temp_left_zone NOTIFY leftTemperatureChanged)
    Q_PROPERTY(int rightTemperature READ get_temp_right_zone WRITE set_temp_right_zone NOTIFY rightTemperatureChanged)

    public:
        explicit HVAC(QUrl &url, QObject * parent = Q_NULLPTR);
        virtual ~HVAC();

    signals:
        void fanSpeedChanged(int fanSpeed);
        void leftTemperatureChanged(int temp);
        void rightTemperatureChanged(int temp);
        void languageChanged(QString language);

    private:
        MessageEngine *m_mloop;

        int m_fanspeed;
        int m_temp_left_zone;
        int m_temp_right_zone;

        int get_fanspeed() const { return m_fanspeed; };
        int get_temp_left_zone() const { return m_temp_left_zone; };
        int get_temp_right_zone() const { return m_temp_right_zone; };

        void control(QString verb, QString field, int value);
        void set_fanspeed(int speed);
        void set_temp_left_zone(int temp);
        void set_temp_right_zone(int temp);

        // slots
        void onMessageReceived(MessageType, Message*);
};

#endif // HVAC_H
