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

#ifndef GUIMETADATA_H
#define GUIMETADATA_H

#include <QDebug>
#include <QObject>

#include <QtQml/QQmlContext>
#include "messageengine.h"

class GuiMetadata : public QObject
{
	Q_OBJECT

	// Common to all template types
	Q_PROPERTY(QString type READ type)
	Q_PROPERTY(QString title READ title)
	Q_PROPERTY(QString subtitle READ subtitle)

	// BodyTemplate1/2
	Q_PROPERTY(QString bodyText READ bodyText)

	// BodyTemplate2
	Q_PROPERTY(QUrl bodyImageSmallUrl READ bodyImageSmallUrl)
	Q_PROPERTY(QUrl bodyImageMediumUrl READ bodyImageMediumUrl)
	Q_PROPERTY(QUrl bodyImageLargeUrl READ bodyImageLargeUrl)

	// WeatherTemplate
	Q_PROPERTY(QString weatherCurrentTemperature READ weatherCurrentTemperature)
	Q_PROPERTY(QUrl weatherCurrentWeatherIconSmallUrl READ weatherCurrentWeatherIconSmallUrl)
	Q_PROPERTY(QUrl weatherCurrentWeatherIconSmallDarkBgUrl READ weatherCurrentWeatherIconSmallDarkBgUrl)
	Q_PROPERTY(QUrl weatherCurrentWeatherIconMediumUrl READ weatherCurrentWeatherIconMediumUrl)
	Q_PROPERTY(QUrl weatherCurrentWeatherIconMediumDarkBgUrl READ weatherCurrentWeatherIconMediumDarkBgUrl)
	Q_PROPERTY(QUrl weatherCurrentWeatherIconLargeUrl READ weatherCurrentWeatherIconLargeUrl)
	Q_PROPERTY(QUrl weatherCurrentWeatherIconLargeDarkBgUrl READ weatherCurrentWeatherIconLargeDarkBgUrl)

	Q_PROPERTY(QString weatherLowTemperature READ weatherLowTemperature)
	Q_PROPERTY(QUrl weatherLowTemperatureArrowSmallUrl READ weatherLowTemperatureArrowSmallUrl)
	Q_PROPERTY(QUrl weatherLowTemperatureArrowSmallDarkBgUrl READ weatherLowTemperatureArrowSmallDarkBgUrl)
	Q_PROPERTY(QUrl weatherLowTemperatureArrowMediumUrl READ weatherLowTemperatureArrowMediumUrl)
	Q_PROPERTY(QUrl weatherLowTemperatureArrowMediumDarkBgUrl READ weatherLowTemperatureArrowMediumDarkBgUrl)
	Q_PROPERTY(QUrl weatherLowTemperatureArrowLargeUrl READ weatherLowTemperatureArrowLargeUrl)
	Q_PROPERTY(QUrl weatherLowTemperatureArrowLargeDarkBgUrl READ weatherLowTemperatureArrowLargeDarkBgUrl)

	Q_PROPERTY(QString weatherHighTemperature READ weatherHighTemperature)
	Q_PROPERTY(QUrl weatherHighTemperatureArrowSmallUrl READ weatherHighTemperatureArrowSmallUrl)
	Q_PROPERTY(QUrl weatherHighTemperatureArrowSmallDarkBgUrl READ weatherHighTemperatureArrowSmallDarkBgUrl)
	Q_PROPERTY(QUrl weatherHighTemperatureArrowMediumUrl READ weatherHighTemperatureArrowMediumUrl)
	Q_PROPERTY(QUrl weatherHighTemperatureArrowMediumDarkBgUrl READ weatherHighTemperatureArrowMediumDarkBgUrl)
	Q_PROPERTY(QUrl weatherHighTemperatureArrowLargeUrl READ weatherHighTemperatureArrowLargeUrl)
	Q_PROPERTY(QUrl weatherHighTemperatureArrowLargeDarkBgUrl READ weatherHighTemperatureArrowLargeDarkBgUrl)
	// weatherForecast array ignored for now

public:
	explicit GuiMetadata(QUrl &url, QQmlContext *context, QObject * parent = Q_NULLPTR);
	virtual ~GuiMetadata();

	QString type() { return m_type; }
	QString title() { return m_title; }
	QString subtitle() { return m_subtitle; }

	// BodyTemplate1/2
	QString bodyText() { return m_bodyText; }

	// BodyTemplate2
	QUrl bodyImageSmallUrl() { return m_bodyImageSmallUrl; }
	QUrl bodyImageMediumUrl() { return m_bodyImageMediumUrl; }
	QUrl bodyImageLargeUrl() { return m_bodyImageLargeUrl; }

	// WeatherTemplate
	QString weatherCurrentTemperature() { return m_weatherCurrentTemperature; }
	QUrl weatherCurrentWeatherIconSmallUrl() { return m_weatherCurrentWeatherIconSmallUrl; }
	QUrl weatherCurrentWeatherIconSmallDarkBgUrl() { return m_weatherCurrentWeatherIconSmallDarkBgUrl; }
	QUrl weatherCurrentWeatherIconMediumUrl() { return m_weatherCurrentWeatherIconMediumUrl; }
	QUrl weatherCurrentWeatherIconMediumDarkBgUrl() { return m_weatherCurrentWeatherIconMediumDarkBgUrl; }
	QUrl weatherCurrentWeatherIconLargeUrl() { return m_weatherCurrentWeatherIconLargeUrl; }
	QUrl weatherCurrentWeatherIconLargeDarkBgUrl() { return m_weatherCurrentWeatherIconLargeDarkBgUrl; }

	QString weatherLowTemperature() { return m_weatherLowTemperature; }
	QUrl weatherLowTemperatureArrowSmallUrl() { return m_weatherLowTemperatureArrowSmallUrl; }
	QUrl weatherLowTemperatureArrowSmallDarkBgUrl() { return m_weatherLowTemperatureArrowSmallDarkBgUrl; }
	QUrl weatherLowTemperatureArrowMediumUrl() { return m_weatherLowTemperatureArrowMediumUrl; }
	QUrl weatherLowTemperatureArrowMediumDarkBgUrl() { return m_weatherLowTemperatureArrowMediumDarkBgUrl; }
	QUrl weatherLowTemperatureArrowLargeUrl() { return m_weatherLowTemperatureArrowLargeUrl; }
	QUrl weatherLowTemperatureArrowLargeDarkBgUrl() { return m_weatherLowTemperatureArrowLargeDarkBgUrl; }

	QString weatherHighTemperature() { return m_weatherHighTemperature; }
	QUrl weatherHighTemperatureArrowSmallUrl() { return m_weatherHighTemperatureArrowSmallUrl; }
	QUrl weatherHighTemperatureArrowSmallDarkBgUrl() { return m_weatherHighTemperatureArrowSmallDarkBgUrl; }
	QUrl weatherHighTemperatureArrowMediumUrl() { return m_weatherHighTemperatureArrowMediumUrl; }
	QUrl weatherHighTemperatureArrowMediumDarkBgUrl() { return m_weatherHighTemperatureArrowMediumDarkBgUrl; }
	QUrl weatherHighTemperatureArrowLargeUrl() { return m_weatherHighTemperatureArrowLargeUrl; }
	QUrl weatherHighTemperatureArrowLargeDarkBgUrl() { return m_weatherHighTemperatureArrowLargeDarkBgUrl; }

signals:
	void renderTemplate();
	void clearTemplate();

private:
	MessageEngine *m_mloop;
	QQmlContext *m_context;

	void clearMetadata();
	bool parseImageMetadata(QJsonObject &imageObj,
				QUrl &smallUrl, QUrl &mediumUrl, QUrl &largeUrl,
				QUrl *pSmallDarkBgUrl = NULL, QUrl *pMediumDarkBgUrl = NULL, QUrl *pLargeDarkBgUrl = NULL);

	bool updateMetadata(QJsonObject data);
	bool updateBodyMetadata(QJsonObject &data);
	bool updateWeatherMetadata(QJsonObject &data);

	void onConnected();
	void onDisconnected();
	void onMessageReceived(MessageType, Message*);

	const QStringList events {
		"render_template",
		"clear_template",
	};

	QString m_type = "";
	QString m_title = "";
	QString m_subtitle = "";

	// BodyTemplate1/2
	QString m_bodyText = "";

	// BodyTemplate2
	QUrl m_bodyImageSmallUrl = QString("");
	QUrl m_bodyImageMediumUrl = QString("");
	QUrl m_bodyImageLargeUrl = QString("");

	// WeatherTemplate
	QString m_weatherCurrentTemperature = "";
	QUrl m_weatherCurrentWeatherIconSmallUrl = QString("");
	QUrl m_weatherCurrentWeatherIconSmallDarkBgUrl = QString("");
	QUrl m_weatherCurrentWeatherIconMediumUrl = QString("");
	QUrl m_weatherCurrentWeatherIconMediumDarkBgUrl = QString("");
	QUrl m_weatherCurrentWeatherIconLargeUrl = QString("");
	QUrl m_weatherCurrentWeatherIconLargeDarkBgUrl = QString("");

	QString m_weatherLowTemperature = "";
	QUrl m_weatherLowTemperatureArrowSmallUrl = QString("");
	QUrl m_weatherLowTemperatureArrowSmallDarkBgUrl = QString("");
	QUrl m_weatherLowTemperatureArrowMediumUrl = QString("");
	QUrl m_weatherLowTemperatureArrowMediumDarkBgUrl = QString("");
	QUrl m_weatherLowTemperatureArrowLargeUrl = QString("");
	QUrl m_weatherLowTemperatureArrowLargeDarkBgUrl = QString("");

	QString m_weatherHighTemperature = "";
	QUrl m_weatherHighTemperatureArrowSmallUrl = QString("");
	QUrl m_weatherHighTemperatureArrowSmallDarkBgUrl = QString("");
	QUrl m_weatherHighTemperatureArrowMediumUrl = QString("");
	QUrl m_weatherHighTemperatureArrowMediumDarkBgUrl = QString("");
	QUrl m_weatherHighTemperatureArrowLargeUrl = QString("");
	QUrl m_weatherHighTemperatureArrowLargeDarkBgUrl = QString("");
};

#endif // GUIMETADATA_H
