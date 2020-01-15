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

#include "message.h"
#include "messageengine.h"
#include "guimetadata.h"
#include "guimetadatamessage.h"

#include <QJsonArray>

GuiMetadata::GuiMetadata(QUrl &url, QQmlContext *context, QObject * parent) :
	QObject(parent),
	m_mloop(nullptr)
{
	m_mloop = new MessageEngine(url);
	m_context = context;
	QObject::connect(m_mloop, &MessageEngine::connected, this, &GuiMetadata::onConnected);
	QObject::connect(m_mloop, &MessageEngine::disconnected, this, &GuiMetadata::onDisconnected);
	QObject::connect(m_mloop, &MessageEngine::messageReceived, this, &GuiMetadata::onMessageReceived);
}

GuiMetadata::~GuiMetadata()
{
	delete m_mloop;
}

// Qt UI Context

void GuiMetadata::clearMetadata()
{
	m_type = "";
	m_title = "";
	m_subtitle = "";

	m_bodyText = "";
	m_bodyImageSmallUrl = "";
	m_bodyImageMediumUrl = "";
	m_bodyImageLargeUrl = "";

	m_weatherCurrentTemperature = "";
	m_weatherCurrentWeatherIconSmallUrl = "";
	m_weatherCurrentWeatherIconSmallDarkBgUrl = "";
	m_weatherCurrentWeatherIconMediumUrl = "";
	m_weatherCurrentWeatherIconMediumDarkBgUrl = "";
	m_weatherCurrentWeatherIconLargeUrl = "";
	m_weatherCurrentWeatherIconLargeDarkBgUrl = "";

	m_weatherLowTemperature = "";
	m_weatherLowTemperatureArrowSmallUrl = "";
	m_weatherLowTemperatureArrowSmallDarkBgUrl = "";
	m_weatherLowTemperatureArrowMediumUrl = "";
	m_weatherLowTemperatureArrowMediumDarkBgUrl = "";
	m_weatherLowTemperatureArrowLargeUrl = "";
	m_weatherLowTemperatureArrowLargeDarkBgUrl = "";

	m_weatherHighTemperature = "";
	m_weatherHighTemperatureArrowSmallUrl = "";
	m_weatherHighTemperatureArrowSmallDarkBgUrl = "";
	m_weatherHighTemperatureArrowMediumUrl = "";
	m_weatherHighTemperatureArrowMediumDarkBgUrl = "";
	m_weatherHighTemperatureArrowLargeUrl = "";
	m_weatherHighTemperatureArrowLargeDarkBgUrl = "";
}

bool GuiMetadata::parseImageMetadata(QJsonObject &imageObj,
				     QUrl &smallUrl, QUrl &mediumUrl, QUrl &largeUrl,
				     QUrl *pSmallDarkBgUrl, QUrl *pMediumDarkBgUrl, QUrl *pLargeDarkBgUrl)
{
	if(!(imageObj.contains("sources") && imageObj["sources"].isArray())) {
		// error
		qWarning() << "Could not find image.sources parameter!";
		return false;
	}
	QJsonArray sourcesObj = imageObj["sources"].toArray();
	bool found = false;
	for(QJsonArray::iterator it = sourcesObj.begin(); it != sourcesObj.end(); it++) {
		if(!it->isObject()) {
			// unexpected, skip
			continue;
		}
		QJsonObject sourceObj = it->toObject();
		if(!(sourceObj.contains("url") && sourceObj["url"].isString())) {
			// error
			qWarning() << "Missing image.sources.url parameter!";
			continue;
		}
		QString url = sourceObj["url"].toString();
		QString darkBackgroundUrl = "";
		if(sourceObj.contains("darkBackgroundUrl") && sourceObj["darkBackgroundUrl"].isString()) {
			darkBackgroundUrl = sourceObj["darkBackgroundUrl"].toString();
		}
		if(sourceObj.contains("size") && sourceObj["size"].isString()) {
			QString size = sourceObj["size"].toString();
			if(size == "SMALL") {
				smallUrl = url;
				if(pSmallDarkBgUrl)
					pSmallDarkBgUrl->setUrl(darkBackgroundUrl);
				found = true;
			} else if(size == "MEDIUM") {
				mediumUrl = url;
				if(pMediumDarkBgUrl)
					pMediumDarkBgUrl->setUrl(darkBackgroundUrl);
				found = true;
			} else if(size == "LARGE") {
				largeUrl = url;
				if(pLargeDarkBgUrl)
					pLargeDarkBgUrl->setUrl(darkBackgroundUrl);
				found = true;
			}
			// else ignore (X-SMALL, X-LARGE)
		}
		// FIXME: Should handle image sources w/o size fields,
		//        parse width/height if present
	}
	return found;
}

bool GuiMetadata::updateMetadata(QJsonObject data)
{
	if(!data.contains("type"))
		return false;

	clearMetadata();

	QString type = data["type"].toString();
	if(!(type == "BodyTemplate1" ||
	     type == "BodyTemplate2" ||
	     type == "WeatherTemplate")) {
		// Show unsupported type message
		m_type = "Unsupported";
		return true;
	}
	m_type = type;

	// All template types have title
	if(data.contains("title") && data["title"].isObject()) {
		QJsonObject titleObj = data["title"].toObject();
		if(titleObj.contains("mainTitle")) {
			m_title = titleObj["mainTitle"].toString();
		} else {
			qWarning() << "Could not find title.mainTitle parameter!";
			return false;
		}
		// subTitle is apparently optional
		if(titleObj.contains("subTitle"))
			m_subtitle = titleObj["subTitle"].toString();
	} else {
		// error
		qWarning() << "Could not find title parameter!";
		return false;
	}

	if(type == "BodyTemplate1" || type == "BodyTemplate2")
		return updateBodyMetadata(data);
	else
		return updateWeatherMetadata(data);
}

bool GuiMetadata::updateBodyMetadata(QJsonObject &data)
{
	if(!data.contains("type"))
		return false;

	QString type = data["type"].toString();
	if(!(type == "BodyTemplate1" || type == "BodyTemplate2"))
		return false;

	// BodyTemplate1/2 have text field
	if(data.contains("textField")) {
		m_bodyText = data["textField"].toString();
	} else {
		// error
		qWarning() << "Could not find textField parameter!";
		return false;
	}

	// BodyTemplate2 has image
	if(type == "BodyTemplate2") {
		if(!(data.contains("image") && data["image"].isObject())) {
			// error
			qWarning() << "Could not find image parameter!";
			return false;
		}
		QJsonObject imageObj = data["image"].toObject();
		if(!parseImageMetadata(imageObj,
				       m_bodyImageSmallUrl,
				       m_bodyImageMediumUrl,
				       m_bodyImageLargeUrl)) {
			qWarning() << "Could not parse image parameter!";
			return false;
		}
	}

	return true;
}

bool GuiMetadata::updateWeatherMetadata(QJsonObject &data)
{
	if(!data.contains("type"))
		return false;

	QString type = data["type"].toString();
	if(type != "WeatherTemplate")
		return false;

	if(data.contains("currentWeather")) {
		m_weatherCurrentTemperature = data["currentWeather"].toString();
	} else {
		// error
		qWarning() << "Could not find currentWeather parameter!";
		return false;
	}

	if(!(data.contains("currentWeatherIcon") && data["currentWeatherIcon"].isObject())) {
		// error
		qWarning() << "Could not find currentWeatherIcon parameter!";
		return false;
	}
	QJsonObject imageObj = data["currentWeatherIcon"].toObject();
	if(!parseImageMetadata(imageObj,
			       m_weatherCurrentWeatherIconSmallUrl,
			       m_weatherCurrentWeatherIconMediumUrl,
			       m_weatherCurrentWeatherIconLargeUrl,
			       &m_weatherCurrentWeatherIconSmallDarkBgUrl,
			       &m_weatherCurrentWeatherIconMediumDarkBgUrl,
			       &m_weatherCurrentWeatherIconLargeDarkBgUrl)) {
		qWarning() << "Could not parse currentWeatherIcon.image parameter!";
		return false;
	}

	if(!(data.contains("lowTemperature") && data["lowTemperature"].isObject())) {
		// error
		qWarning() << "Could not find lowTemperature parameter!";
		return false;
	}
	QJsonObject tempObj = data["lowTemperature"].toObject();
	if(!(tempObj.contains("value") && tempObj["value"].isString())) {
		// error
		qWarning() << "Could not find lowTemperature.value parameter!";
		return false;
	}
	m_weatherLowTemperature = tempObj["value"].toString();

	if(!(tempObj.contains("arrow") && tempObj["arrow"].isObject())) {
		// error
		qWarning() << "Could not find lowTemperature.arrow parameter!";
		return false;
	}
	imageObj = tempObj["arrow"].toObject();
	if(!parseImageMetadata(imageObj,
			       m_weatherLowTemperatureArrowSmallUrl,
			       m_weatherLowTemperatureArrowMediumUrl,
			       m_weatherLowTemperatureArrowLargeUrl,
			       &m_weatherLowTemperatureArrowSmallDarkBgUrl,
			       &m_weatherLowTemperatureArrowMediumDarkBgUrl,
			       &m_weatherLowTemperatureArrowLargeDarkBgUrl)) {
		qWarning() << "Could not parse lowTemperature.arrow parameter!";
		return false;
	}

	if(!(data.contains("highTemperature") && data["highTemperature"].isObject())) {
		// error
		qWarning() << "Could not find highTemperature parameter!";
		return false;
	}
	tempObj = data["highTemperature"].toObject();
	if(!(tempObj.contains("value") && tempObj["value"].isString())) {
		// error
		qWarning() << "Could not find highTemperature.value parameter!";
		return false;
	}
	m_weatherHighTemperature = tempObj["value"].toString();

	if(!(tempObj.contains("arrow") && tempObj["arrow"].isObject())) {
		// error
		qWarning() << "Could not find highTemperature.arrow parameter!";
		return false;
	}
	imageObj = tempObj["arrow"].toObject();
	if(!parseImageMetadata(imageObj,
			       m_weatherHighTemperatureArrowSmallUrl,
			       m_weatherHighTemperatureArrowMediumUrl,
			       m_weatherHighTemperatureArrowLargeUrl,
			       &m_weatherHighTemperatureArrowSmallDarkBgUrl,
			       &m_weatherHighTemperatureArrowMediumDarkBgUrl,
			       &m_weatherHighTemperatureArrowLargeDarkBgUrl)) {
		qWarning() << "Could not parse highTemperature.arrow parameter!";
		return false;
	}

	return true;

}

void GuiMetadata::onConnected()
{
	QStringListIterator eventIterator(events);
	GuiMetadataCapabilityMessage *tmsg;

	tmsg = new GuiMetadataCapabilityMessage();
	QJsonObject parameter;
	QJsonArray actions;
	while (eventIterator.hasNext()) {
		actions.append(QJsonValue(eventIterator.next()));
	}
	parameter.insert("actions", actions);
	tmsg->createRequest("guimetadata/subscribe", parameter);
	m_mloop->sendMessage(tmsg);
	delete tmsg;
}

void GuiMetadata::onDisconnected()
{
	// vshl-capabilities currently has no unsubscribe verb...
}

void GuiMetadata::onMessageReceived(MessageType type, Message *message)
{
	if (type == GuiMetadataCapabilityEventMessage) {
		GuiMetadataCapabilityMessage *tmsg = qobject_cast<GuiMetadataCapabilityMessage*>(message);
		if (tmsg->isEvent()) {
			if (tmsg->isGuiMetadataRenderTemplateEvent()) {
				if(updateMetadata(tmsg->eventData()))
					emit renderTemplate();
			} else if (tmsg->isGuiMetadataClearTemplateEvent()) {
				emit clearTemplate();
			}
		}
	}
	message->deleteLater();
}
