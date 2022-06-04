#ifndef _CD_TRANSPORT_H
#define _CD_TRANSPORT_H

#include "blocking_queue.h"
#include "common.h"
#include "deemph.h"

#include <atomic>
#include <condition_variable>
#include <cdio/paranoia/cdda.h>
#include <cdio/paranoia/paranoia.h>
#include <cdio/cd_types.h>
#include <functional>
#include <mutex>

class TransportStatus
{
	public:
		// Current playback position
		int track_num;
		int track_min;
		int track_sec;

		bool deemph_active;
};

class CdTransport
{
	public:
		CdTransport(CircularBlockingQueue<int16_t> *data_out);
		~CdTransport();

		void wait_for_disc();
		bool load_disc();
		void play();
		void seek_track(int track_num);
		void seek_prev();
		void seek_next();
		void pause();
		void eject();
		void set_deemph_mode(DeemphMode mode);

		// Set a callback that will be invoked on each sector read 
		void set_status_callback(std::function<void(TransportStatus)>);


	private:
		// make sure we don't eject during a read or seek
		std::mutex drive_mut;
		CdIo_t *cdio;
		cdrom_drive_t *drive;
		cdrom_paranoia_t *paranoia;
		Deemph deemph;
		DeemphMode deemph_mode;
		CircularBlockingQueue<int16_t> *data_out;
		std::function<void(TransportStatus)> status_callback;

		// TOC info
		lsn_t disc_first_lsn;
		lsn_t disc_last_lsn;
		int num_tracks;
		// CDs only have up to 99 tracks (+ track 0?) so just reserve 100
		lsn_t track_first_lsns[100];
		lsn_t track_last_lsns[100];
		bool track_has_preemph[100];

		// Playback state
		bool playing;
		std::atomic<lsn_t> read_cursor;
		int paranoia_read_retries;

		void seek_lsn(lsn_t lsn);
		void get_track_min_sec(int tr, int *min, int *sec);
		void adjust_retries();
};

#endif
