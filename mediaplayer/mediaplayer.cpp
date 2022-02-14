/*
 * Copyright (C) 2018-2020,2022 Konsulko Group
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
#include <QJsonObject>

#include "mediaplayer.h"
#include "MediaplayerMpdBackend.h"


Playlist::Playlist(QVariantMap &item)
{
	m_index = item["index"].toInt();
	m_duration = item["duration"].toInt();
	m_genre = item["genre"].toString();
	m_path = item["path"].toString();
	m_title = item["title"].toString();
	m_album = item["album"].toString();
	m_artist = item["artist"].toString();
}

Playlist::~Playlist() {}

Mediaplayer::Mediaplayer (QQmlContext *context, QObject * parent) :
	QObject(parent)
{
	m_context = context;
	m_context->setContextProperty("MediaplayerModel", QVariant::fromValue(m_playlist));

	m_backend = new MediaplayerMpdBackend(this, context);
	if (!m_backend)
		qFatal("Could not create MediaPlayerBackend");
}

Mediaplayer::~Mediaplayer()
{
}

// Qt UI Context

void Mediaplayer::updatePlaylist(QVariantMap playlist)
{
	QVariantList list = playlist["list"].toList();

	m_playlist.clear();

	for (auto i : list) {
		QVariantMap item = qvariant_cast<QVariantMap>(i);
		m_playlist.append(new Playlist(item));
	}

	if (m_playlist.count() == 0) {
		QVariantMap tmp, track;

		track.insert("title", "");
		track.insert("artist", "");
		track.insert("album", "");
		track.insert("duration", 0);

		tmp.insert("position", 0);
		tmp.insert("track", track);

		// clear metadata in UI
		m_context->setContextProperty("AlbumArt", "");
		emit metadataChanged(tmp);
	}

	// Refresh model
	m_context->setContextProperty("MediaplayerModel", QVariant::fromValue(m_playlist));
}

void Mediaplayer::updateMetadata(QVariantMap metadata)
{
	if (metadata.contains("track")) {
		QVariantMap track = metadata.value("track").toMap();

		if (track.contains("image")) {
			m_context->setContextProperty("AlbumArt",
						      QVariant::fromValue(track.value("image")));
		}
 
		if (!track.contains("artist")) {
			track.insert("artist", "");
			metadata["track"] = track;
		}
 
		if (!track.contains("album")) {
			track.insert("album", "");
			metadata["track"] = track;
		}
	}

	emit metadataChanged(metadata);
}

// Control methods

// For backwards compatibility
void Mediaplayer::disconnect()
{
	disconnectBluetooth();
}

// For backwards compatibility
void Mediaplayer::connect()
{
	connectBluetooth();
}

void Mediaplayer::disconnectBluetooth()
{
	// Disconnect from Bluetooth media
}

void Mediaplayer::connectBluetooth()
{
	// Connect to Bluetooth media
}

void Mediaplayer::play()
{
	m_backend->play();
}

void Mediaplayer::pause()
{
	m_backend->pause();
}

void Mediaplayer::previous()
{
	m_backend->previous();
}

void Mediaplayer::next()
{
	m_backend->next();
}

void Mediaplayer::seek(int milliseconds)
{
	m_backend->seek(milliseconds);
}

void Mediaplayer::fastforward(int milliseconds)
{
	m_backend->fastforward(milliseconds);
}

void Mediaplayer::rewind(int milliseconds)
{
	m_backend->rewind(milliseconds);
}

void Mediaplayer::picktrack(int track)
{
	m_backend->picktrack(track);
}

void Mediaplayer::volume(int volume)
{
	m_backend->volume(volume);
}

void Mediaplayer::loop(QString state)
{
	m_backend->loop(state);
}
