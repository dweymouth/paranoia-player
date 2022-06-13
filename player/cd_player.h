#ifndef _CD_PLAYER_H_
#define _CD_PLAYER_H_

#include "audio_out.h"
#include "blocking_queue.h"
#include "cd_transport.h"
#include "common.h"

#include <atomic>
#include <functional>

enum PlayerState
{
	STOPPED,
	// stop cmd has been dispatched but
	// transport not yet shut down
	STOPPING,
	PLAYING,
	PAUSED
};

struct PlayerStatus
{
	PlayerState state;
	// if state == STOPPED | STOPPING
	//   track_num is disc total tracks
	//   time_sec is disc total time
	int track_num;
	float time_sec;
	bool deemph;
};

class CdPlayer
{
	public:
		CdPlayer(bool auto_await_disc = false);
		bool wait_and_load_disc();
		bool play_disc();
		void seek_prev();
		void seek_next();
		void seek_track(int track_num);
		// seek to a position within the current track (pos in [0..1])
		void seek_trackpos(float pos);
		void pause();
		void play_pause();
		void stop();
		void eject();
		void set_deemph_mode(DeemphMode mode);
		void set_status_callback(std::function<void(PlayerStatus)>);
		DiscInfo *get_disc_info() { return &transport.disc_info; }

		// readonly
		std::atomic<bool> have_disc;
		std::atomic<PlayerState> state;
		std::atomic<int> cur_track;
		std::atomic<float> track_time_sec;
		bool deemph_active;

	private:
		CircularBlockingQueue<int16_t> data_buf;
		CdTransport transport;
		lsn_t transport_cursor;
		AudioOut audio_out;
		bool auto_await_disc;
		bool is_audio_init;
		bool is_audio_active;
		std::atomic<bool> transport_running;
		std::function<void(PlayerStatus)> status_callback;

		void transport_status_callback(TransportStatus stat);
		void calculate_track_time(lsn_t transport_cursor, int transport_tr);
		void poll_end_of_playback();
};

#endif
