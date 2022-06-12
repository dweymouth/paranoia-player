#include "cd_player.h"
#include <chrono>
#include <iostream>
#include <thread>

CdPlayer::CdPlayer() :
	data_buf(SAMPLES_PER_CD_FRAME * 75 * 5),
	transport(&this->data_buf),
	audio_out(&this->data_buf)
{
	this->state = STOPPED;
	this->transport.set_status_callback(std::bind(&CdPlayer::transport_status_callback, this, std::placeholders::_1));
}

void CdPlayer::transport_status_callback(TransportStatus stat)
{
	if (stat.stopped) {
		this->transport_running = false;
		if (this->state == STOPPING) {
			this->state = STOPPED;
		}
		// TODO: spin up thread to check data buf
		// and update state to stopped once buffer runs out
	} else {
		this->cur_track = stat.track_num;
		this->track_min = stat.track_min;
		this->track_sec = stat.track_sec;
		this->deemph_active = stat.deemph_active;
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
	this->transport.seek_prev();
}

void CdPlayer::seek_next()
{
	if (!this->have_disc)
		return;
	this->transport.seek_next();
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
}

void CdPlayer::seek_track(int track_num)
{
	if (!this->have_disc)
		return;
	this->transport.seek_track(track_num);
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

void CdPlayer::set_deemph_mode(DeemphMode mode)
{
	this->transport.set_deemph_mode(mode);
}
