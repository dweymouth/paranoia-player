#include "cd_player.h"
#include <chrono>
#include <iostream>
#include <thread>

CdPlayer::CdPlayer(bool auto_await_disc) :
	data_buf(SAMPLES_PER_CD_FRAME * 75 * 5),
	transport(&this->data_buf),
	audio_out(&this->data_buf)
{
	this->state = STOPPED;
	this->auto_await_disc = auto_await_disc;
	this->transport.set_status_callback(std::bind(&CdPlayer::transport_status_callback, this, std::placeholders::_1));
	if (auto_await_disc) {
		std::thread await_disc_th(std::bind(&CdPlayer::wait_and_load_disc, this));
		await_disc_th.detach();
	}
}

void CdPlayer::transport_status_callback(TransportStatus stat)
{
	if (stat.stopped) {
		this->transport_running = false;
		if (this->state == STOPPING) {
			this->state = STOPPED;
		}
		// transport stopped but we still have the last few seconds in the buffer
		std::thread poll_playbk_th(std::bind(&CdPlayer::poll_end_of_playback, this));
		poll_playbk_th.detach();
	} else {
		this->transport_cursor = stat.lsn_cursor;
		calculate_track_time(stat.lsn_cursor, stat.track_num);
		this->deemph_active = stat.deemph_active;
	}
}

void CdPlayer::calculate_track_time(lsn_t transport_cursor, int transport_tr)
{
	// figure out the lsn pointing to the data being sent to the sound card now
	lsn_t playing_lsn = transport_cursor - (this->data_buf.get_count() / SAMPLES_PER_CD_FRAME);
	this->cur_track = transport_tr;
	DiscInfo info = this->transport.disc_info;
	if (playing_lsn < info.track_first_lsns[transport_tr]) {
		// transport is reading track n but we're still playing track n-1
		this->cur_track = transport_tr - 1;
	}
	this->track_time_sec = (playing_lsn - info.track_first_lsns[this->cur_track]) / 75.0;
}

void CdPlayer::poll_end_of_playback()
{
	while (this->state == PLAYING) {
		if (this->data_buf.get_count() <= 0) {
			this->audio_out.stop();
			this->state = STOPPED;
			break;
		}
		DiscInfo info = this->transport.disc_info;
		calculate_track_time(info.disc_last_lsn, info.num_tracks);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

bool CdPlayer::wait_and_load_disc()
{
	this->transport.wait_for_disc();
	if (!transport.load_disc()) {
		return false;
	}
	have_disc = true;
	return true;
}

bool CdPlayer::play_disc()
{
	if (this->state == PLAYING) {
		return true;
	}
	if (!this->have_disc || this->state == STOPPING) {
		return false;
	}
	if (this->state == PAUSED) {
		this->pause(); // unpause
	}
	if (!this->is_audio_init) {
		if (!this->audio_out.init()) {
			std::cerr << "Unable to initialize audio output." << std::endl;
		} else {
			this->is_audio_init = true;
		}
	}
	this->data_buf.set_read_paused(false);
	this->data_buf.clear();
	this->audio_out.start();
	std::thread play_thread(&CdTransport::play, &this->transport);
	play_thread.detach();
	this->transport_running = true;
	this->state = PLAYING;
	return true;
}

void CdPlayer::seek_prev()
{
	if (!this->have_disc)
		return;
	if (this->cur_track <= 1)
		this->transport.seek_track(1);
	else if (this->track_time_sec > 2.)
		this->transport.seek_track(this->cur_track);
	else
		this->transport.seek_track(this->cur_track - 1);
}

void CdPlayer::seek_next()
{
	if (!this->have_disc)
		return;
	if (this->cur_track < this->transport.disc_info.num_tracks)
		this->transport.seek_track(this->cur_track + 1);
}

void CdPlayer::stop()
{
	if (!this->have_disc || this->state == STOPPING || this->state == STOPPED)
		return;
	if (!this->transport_running) {
		this->state = STOPPED;
	} else {
		this->state = STOPPING;
		this->transport.stop();
		// don't set state == STOPPED until
		// we get a callback from transport
	}
	this->data_buf.clear();
	this->audio_out.stop();
}

void CdPlayer::eject()
{
	if (!this->have_disc)
		return;
	// make sure the transport is safely stopped
	this->stop();
	while (this->state == STOPPING) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	this->transport.eject();
	this->have_disc = false;
	if (this->auto_await_disc) {
		std::thread await_disc_th(std::bind(&CdPlayer::wait_and_load_disc, this));
		await_disc_th.detach();
	}
}

void CdPlayer::seek_track(int track_num)
{
	if (!this->have_disc)
		return;
	this->transport.seek_track(track_num);
}

void CdPlayer::seek_trackpos(float pos)
{
	if (!this->have_disc)
		return;
	if (pos < 0.)
		pos = 0.;
	if (pos > 1.)
		pos = 1.;

	DiscInfo *info = get_disc_info();
	lsn_t first = info->track_first_lsns[cur_track];
	lsn_t seek = first + ((lsn_t)((info->track_last_lsns[cur_track] - first) * pos));
	this->transport.seek_lsn(seek);
}

void CdPlayer::pause()
{
	// when pausing, cd transport will stop reading when buffer fills
	if (!this->have_disc || this->state == STOPPING || this->state == STOPPED) {
		return;
	}
	if (this->state == PAUSED) {
		this->state = PLAYING;
	} else if (this->state == PLAYING) {
		this->state = PAUSED;
	}
	this->data_buf.set_read_paused(this->state == PAUSED);
}

void CdPlayer::play_pause()
{
	if (!this->have_disc || this->state == STOPPING)
		return;
	if (this->state == STOPPED)
		this->play_disc();
	else
		this->pause();
}

void CdPlayer::set_deemph_mode(DeemphMode mode)
{
	this->transport.set_deemph_mode(mode);
}
