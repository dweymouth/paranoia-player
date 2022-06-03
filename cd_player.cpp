#include "cd_player.h"
#include <iostream>

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
