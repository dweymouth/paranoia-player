#ifndef _CD_TRANSPORT_H
#define _CD_TRANSPORT_H

#include "blocking_queue.h"
#include "common.h"
#include "deemph.h"
#include "disc_info.h"

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
		lsn_t lsn_cursor;

		bool deemph_active;
		bool stopped;
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
		void seek_lsn(lsn_t lsn);
		void pause();
		void stop();
		void eject();
		void set_deemph_mode(DeemphMode mode);

		// Set a callback that will be invoked on each sector read 
		void set_status_callback(std::function<void(TransportStatus)>);

		// read-only access outside of this class
		DiscInfo disc_info;

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

		// Playback state
		std::atomic<bool> playing;
		std::atomic<lsn_t> read_cursor;
		int paranoia_read_retries;

		// Private methods
		void adjust_retries();
		void do_status_callback(int cur_track);
};

#endif
