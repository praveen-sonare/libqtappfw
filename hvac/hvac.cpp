/*
 * Copyright (C) 2020-2021 Konsulko Group
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

#include <QDebug>
#include <QMetaEnum>
#include <QMimeDatabase>
#include <QtQml/QQmlEngine>
#include "hvac.h"


// TODO: don't duplicate defaults from HVAC service here
HVAC::HVAC (QObject * parent) :
    QObject(parent),
    m_fanspeed(0),
    m_temp_left_zone(21),
    m_temp_right_zone(21)
{
}

HVAC::~HVAC()
{
}

void HVAC::control(QString verb, QString field, int value)
{
}

void HVAC::set_fanspeed(int speed)
{
    emit fanSpeedChanged(speed);
}

void HVAC::set_temp_left_zone(int temp)
{
    emit leftTemperatureChanged(temp);
}

void HVAC::set_temp_right_zone(int temp)
{
    emit rightTemperatureChanged(temp);
}
