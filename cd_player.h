#ifndef _CD_PLAYER_H_
#define _CD_PLAYER_H_

#include "audio_out.h"
#include "blocking_queue.h"
#include "cd_transport.h"
#include "common.h"

#include <atomic>

enum PlayerState
{
	STOPPED,
	STOPPING,
	PLAYING,
	PAUSED
};

class CdPlayer
{
	public:
		CdPlayer();
		bool wait_and_load_disc();
		bool play_disc();
		void seek_prev();
		void seek_next();
		void seek_track(int track_num);
		void pause();
		void stop();
		void eject();
		void set_deemph_mode(DeemphMode mode);
		DiscInfo *get_disc_info() { return &transport.disc_info; }


		// readonly
		bool have_disc;
		std::atomic<PlayerState> state;
		int cur_track;
		int track_min;
		int track_sec;
		bool deemph_active;

	private:
		CircularBlockingQueue<int16_t> data_buf;
		CdTransport transport;
		AudioOut audio_out;
		bool is_audio_init;
		bool is_audio_active;

		void transport_status_callback(TransportStatus stat);
};

#endif
