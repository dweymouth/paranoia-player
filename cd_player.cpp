#include "cd_player.h"
#include <iostream>

CdPlayer::CdPlayer() :
	data_buf(SAMPLES_PER_CD_FRAME * 75 * 5),
	transport(&this->data_buf),
	audio_out(&this->data_buf)
{
	this->transport.set_status_callback(std::bind(&CdPlayer::transport_status_callback, this, std::placeholders::_1));
}

void CdPlayer::transport_status_callback(TransportStatus stat)
{
	this->cur_track = stat.track_num;
	this->track_min = stat.track_min;
	this->track_sec = stat.track_sec;
	this->deemph_active = stat.deemph_active;
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
	if (!this->have_disc) {
		return false;
	}

	if (!this->audio_out.init()) {
		std::cerr << "Unable to initialize audio output." << std::endl;
	}
	this->audio_out.start();
	this->transport.play();
	return true;
}

void CdPlayer::seek_prev()
{
	this->transport.seek_prev();
}

void CdPlayer::seek_next()
{
	this->transport.seek_next();
}

void CdPlayer::eject()
{
	this->audio_out.stop();
	this->data_buf.clear();
	this->transport.eject();
}

void CdPlayer::seek_track(int track_num)
{
	this->transport.seek_track(track_num);
}

void CdPlayer::pause()
{
	// when pausing, cd transport will stop reading when buffer fills
	this->paused = !this->paused;
	this->data_buf.set_read_paused(this->paused);
}

void CdPlayer::set_deemph_mode(DeemphMode mode)
{
	this->transport.set_deemph_mode(mode);
}
