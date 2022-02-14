/*
 * Copyright (C) 2022 Konsulko Group
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
#include "MediaplayerMpdBackend.h"
#include "MpdEventHandler.h"
#include "mediaplayer.h"

// Use a 60s timeout on our MPD connection
// NOTE: The connection is actively poked at a higher frequency than
//       this to ensure we don't hit the timeout.  The alternatives
//       are to either open and close a connection for every command,
//       or try to keep the connection in idle mode when not using it.
//       The latter is deemed too complicated for our purposes for now,
//       due to it likely requiring another thread.
#define MPD_CONNECTION_TIMEOUT	60000

MediaplayerMpdBackend::MediaplayerMpdBackend(Mediaplayer *player, QQmlContext *context, QObject *parent) :
	QObject(parent),
	m_player(player)
{
	struct mpd_connection *conn = mpd_connection_new(NULL, 0, MPD_CONNECTION_TIMEOUT);
	if (!conn) {
		qFatal("Could not create MPD connection");
	}
	if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS) {
		qFatal("%s", mpd_connection_get_error_message(conn));
	}
	m_mpd_conn = conn;

	// Set up connection keepalive timer
	m_mpd_conn_timer = new QTimer(this);
	connect(m_mpd_conn_timer, &QTimer::timeout, this, &MediaplayerMpdBackend::connectionKeepaliveTimeout);
	m_mpd_conn_timer->start(MPD_CONNECTION_TIMEOUT / 2);

	m_song_pos_timer = new QTimer(this);
	connect(m_song_pos_timer, &QTimer::timeout, this, &MediaplayerMpdBackend::songPositionTimeout);

	MpdEventHandler *handler = new MpdEventHandler();
	handler->moveToThread(&m_handlerThread);
	connect(&m_handlerThread, &QThread::finished, handler, &QObject::deleteLater);
	connect(this, &MediaplayerMpdBackend::start, handler, &MpdEventHandler::handleEvents);

	// Connect playback state updates from the backend handler thread
	// so our view of the state is kept in sync with MPD.
	connect(handler,
		&MpdEventHandler::playbackStateUpdate,
		this,
		&MediaplayerMpdBackend::updatePlaybackState);

	// Connect updates from backend handler thread to parent.
	// Note that we should not have to explicitly specify the
	// Qt::QueuedConnection option here since the handler is constructed
	// by adding a worker object to a QThread and it should be automatic
	// in that case.
	connect(handler,
		&MpdEventHandler::playlistUpdate,
		player,
		&Mediaplayer::updatePlaylist);
	connect(handler,
		&MpdEventHandler::metadataUpdate,
		player,
		&Mediaplayer::updateMetadata);

	m_handlerThread.start();

	// Start event handler worker loop
	emit start();
}

MediaplayerMpdBackend::~MediaplayerMpdBackend()
{
	m_handlerThread.quit();
	m_handlerThread.wait();

	m_mpd_conn_timer->stop();
	delete m_mpd_conn_timer;

	mpd_connection_free(m_mpd_conn);
}

void MediaplayerMpdBackend::connectionKeepaliveTimeout(void)
{
	m_mpd_conn_mutex.lock();

	// Clear any lingering non-fatal errors
	if (!mpd_connection_clear_error(m_mpd_conn)) {
		// NOTE: There should likely be an attempt to reconnect here,
		//       but it definitely would complicate things for all the
		//       other users.
		qWarning() << "MPD connection in error state!";
		m_mpd_conn_mutex.unlock();
		return;
	}

	struct mpd_status *status = mpd_run_status(m_mpd_conn);
	if (!status) {
		qWarning() << "MPD connection status check failed";
	} else {
		mpd_status_free(status);
	}

	m_mpd_conn_mutex.unlock();
}

void MediaplayerMpdBackend::songPositionTimeout(void)
{
	m_state_mutex.lock();

	if (m_playing) {
		// Instead of the expense of repeatedly calling mpd_run_status,
		// provide our own elapsed time.  In practice this seems
		// sufficient for reasonable behavior in the application UI, and
		// it is what seems recommended for MPD client implementations.
		m_song_pos_ms += 250;
		QVariantMap metadata;
		metadata["position"] = m_song_pos_ms;
		m_player->updateMetadata(metadata);
	}

	m_state_mutex.unlock();
}

void MediaplayerMpdBackend::updatePlaybackState(int queue_pos, int song_pos_ms, bool state)
{
	m_state_mutex.lock();

	m_queue_pos = queue_pos;
	m_song_pos_ms = song_pos_ms;
	if (m_playing != state) {
		if (state) {
			// Start position timer
			m_song_pos_timer->start(250);
		} else {
			// Stop position timer
			m_song_pos_timer->stop();
			//m_song_pos_ms = 0;
		}
	}
	m_playing = state;

	m_state_mutex.unlock();
}

// Control methods

void MediaplayerMpdBackend::play()
{
	m_mpd_conn_mutex.lock();

	m_state_mutex.lock();
	if (!m_playing) {
		mpd_run_play(m_mpd_conn);
	}
	m_state_mutex.unlock();

	m_mpd_conn_mutex.unlock();
}

void MediaplayerMpdBackend::pause()
{
	m_mpd_conn_mutex.lock();

	m_state_mutex.lock();
	if (m_playing) {
		mpd_run_pause(m_mpd_conn, true);
	}
	m_state_mutex.unlock();

	m_mpd_conn_mutex.unlock();
}

void MediaplayerMpdBackend::previous()
{
	m_mpd_conn_mutex.lock();

	// MPD only allows next/previous if playing
	m_state_mutex.lock();
	if (m_playing) {
		mpd_run_previous(m_mpd_conn);
	}
	m_state_mutex.unlock();

	m_mpd_conn_mutex.unlock();
}

void MediaplayerMpdBackend::next()
{
	m_mpd_conn_mutex.lock();

	// MPD only allows next/previous if playing
	m_state_mutex.lock();
	if (m_playing) {
		mpd_run_next(m_mpd_conn);
	}
	m_state_mutex.unlock();

	m_mpd_conn_mutex.unlock();
}

void MediaplayerMpdBackend::seek(int milliseconds)
{
	m_mpd_conn_mutex.lock();

	float t = milliseconds;
	t /= 1000.0;
	mpd_run_seek_current(m_mpd_conn, t, false);

	m_mpd_conn_mutex.unlock();
}

// Relative to current position
void MediaplayerMpdBackend::fastforward(int milliseconds)
{
	m_mpd_conn_mutex.lock();

	float t = milliseconds;
	t /= 1000.0;
	mpd_run_seek_current(m_mpd_conn, t, true);

	m_mpd_conn_mutex.unlock();
}

// Relative to current position
void MediaplayerMpdBackend::rewind(int milliseconds)
{
	m_mpd_conn_mutex.lock();

	float t = -milliseconds;
	t /= 1000.0;
	mpd_run_seek_current(m_mpd_conn, t, true);

	m_mpd_conn_mutex.unlock();
}

void MediaplayerMpdBackend::picktrack(int track)
{
	m_mpd_conn_mutex.lock();

	if (track >= 0) {
		mpd_run_play_pos(m_mpd_conn, track);
	}

	m_mpd_conn_mutex.unlock();
}

void MediaplayerMpdBackend::volume(int volume)
{
	// Not implemented
}

void MediaplayerMpdBackend::loop(QString state)
{
	m_mpd_conn_mutex.lock();

	// Song:
	// mpd_run_single_state(m_mpd_conn, MPD_SINGLE_ON)
	// mpd_run_repeat(m_mpd_conn, true) to loop
	//
	// Playlist:
	// mpd_run_single_state(m_mpd_conn, MPD_SINGLE_OFF) (default)
	// mpd_run_repeat(m_mpd_conn, true) to loop

	if (state == "playlist") {
		mpd_run_repeat(m_mpd_conn, true);
	} else {
		mpd_run_repeat(m_mpd_conn, false);
	}

	m_mpd_conn_mutex.unlock();
}
