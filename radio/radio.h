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

#ifndef RADIO_H
#define RADIO_H

#include <QDebug>
#include <QObject>

#include <QtQml/QQmlContext>
#include "messageengine.h"

class Radio : public QObject
{
    Q_OBJECT
    Q_PROPERTY(unsigned int band READ band WRITE setBand NOTIFY bandChanged)
    Q_PROPERTY(unsigned int amBand READ amBand CONSTANT)
    Q_PROPERTY(unsigned int fmBand READ fmBand CONSTANT)
    Q_PROPERTY(unsigned int frequency READ frequency WRITE setFrequency NOTIFY frequencyChanged)
    Q_PROPERTY(bool playing READ playing NOTIFY playingChanged)
    Q_PROPERTY(bool scanning READ scanning NOTIFY scanningChanged)
    Q_PROPERTY(unsigned int minFrequency READ minFrequency NOTIFY minFrequencyChanged)
    Q_PROPERTY(unsigned int maxFrequency READ maxFrequency NOTIFY maxFrequencyChanged)
    Q_PROPERTY(unsigned int frequencyStep READ frequencyStep NOTIFY frequencyStepChanged)

    public:
        explicit Radio(QUrl &url, QQmlContext *context, QObject * parent = Q_NULLPTR);
        virtual ~Radio();

        unsigned int band() const { return m_band; }
        void setBand(int band);

        unsigned int amBand() const { return 0; }
        unsigned int fmBand() const { return 1; }

        unsigned int frequency() const { return m_frequency; }
        void setFrequency(int frequency);

        unsigned int minFrequency() const { return m_minFrequency; }
        unsigned int maxFrequency() const { return m_maxFrequency; }
        unsigned int frequencyStep() const { return m_frequencyStep; }

        bool playing() const { return m_playing; }

        bool scanning() const { return m_scanning; }

	// controls
        Q_INVOKABLE void start();
        Q_INVOKABLE void stop();
        Q_INVOKABLE void scanForward();
        Q_INVOKABLE void scanBackward();
        Q_INVOKABLE void scanStop();

    signals:
	void bandChanged(int band);
	void frequencyChanged(int frequency);
	void playingChanged(bool status);
	void scanningChanged(bool scanning);
	void minFrequencyChanged(int minFrequency);
	void maxFrequencyChanged(int maxFrequency);
	void frequencyStepChanged(int frequencyStep);

    private:
        MessageEngine *m_mloop;
        QQmlContext *m_context;

	unsigned int m_band;
	unsigned int m_frequency;
	unsigned int m_minFrequency;
	unsigned int m_maxFrequency;
	unsigned int m_frequencyStep;
        bool m_playing;
	bool m_scanning;

        void updateFrequencyBandParameters();
        void onConnected();
        void onDisconnected();
        void onMessageReceived(MessageType type, Message *msg);

        const QStringList events {
            "frequency",
            "station_found",
            "status",
        };
};

#endif // RADIO_H
