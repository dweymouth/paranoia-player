#ifndef _CD_PLAYER_H_
#define _CD_PLAYER_H_

#include "audio_out.h"
#include "blocking_queue.h"
#include "cd_transport.h"
#include "common.h"

class CdPlayer
{
	public:
		CdPlayer() :
			data_buf(SAMPLES_PER_CD_FRAME * 75 * 3),
			transport(&this->data_buf), 
			audio_out(&this->data_buf)
		{ }
		bool wait_and_load_disc();
		bool play_disc();
		void seek_prev();
		void seek_next();
		void seek_track(int track_num);
		void pause();
		void set_deemph_mode(DeemphMode mode);
	private:
		bool have_disc;
		bool paused;
		CircularBlockingQueue<int16_t> data_buf;
		CdTransport transport;
		AudioOut audio_out;
};

#endif
