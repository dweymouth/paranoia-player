#include "cd_transport.h"
#include "common.h"

#include <chrono>
#include <iostream>
#include <thread>


CdTransport::CdTransport(CircularBlockingQueue<int16_t> *data_out)
{
	this->data_out = data_out;
	this->paranoia_read_retries = 3;
}

void CdTransport::set_status_callback(std::function<void(TransportStatus)> callbk)
{
	this->status_callback = callbk;
}

void CdTransport::wait_for_disc()
{
	bool have_cd = false;
	char **ppsz_cd_drives = NULL;
	while (!have_cd) {
		ppsz_cd_drives = cdio_get_devices_with_cap(NULL, CDIO_FS_AUDIO, false);
		if (ppsz_cd_drives && *ppsz_cd_drives) {
			// found a CD ROM drive with an audio CD loaded
			// use the first drive in the list
			this->cdio = cdio_open(*ppsz_cd_drives, DRIVER_UNKNOWN);
			this->drive = cdio_cddap_identify_cdio(cdio, CDDA_MESSAGE_PRINTIT, NULL);
		}
		if (!this->drive) {
			std::cerr << "Unable to identify audio CD disc." << std::endl;
		} else {
			have_cd = true;
			break;
		}
		std::this_thread::sleep_for(std::chrono::seconds(1));
		cdio_free_device_list(ppsz_cd_drives);
		ppsz_cd_drives = NULL;
	}
	if (ppsz_cd_drives) {
		cdio_free_device_list(ppsz_cd_drives);
	}
}

bool CdTransport::load_disc()
{
	if (cdio_cddap_open(drive) != 0) {
		std::cerr << "Unable to open disc." << std::endl;
		return false;
	}
	this->disc_info.disc_first_lsn = cdda_disc_firstsector(this->drive);
	this->disc_info.disc_last_lsn = cdda_disc_lastsector(this->drive);
	this->disc_info.num_tracks = cdio_cddap_tracks(this->drive);
	for (int t = 0; t <= this->disc_info.num_tracks; t++) {
		this->disc_info.track_first_lsns[t] = cdio_cddap_track_firstsector(drive, t);
		this->disc_info.track_last_lsns[t] = cdio_cddap_track_lastsector(drive, t);
		this->disc_info.track_has_preemph[t] =
			(cdio_get_track_preemphasis(cdio, t) == CDIO_TRACK_FLAG_TRUE);
	}
	this->read_cursor = this->disc_info.track_first_lsns[1];

	this->paranoia = paranoia_init(drive);
	paranoia_modeset(this->paranoia, PARANOIA_MODE_SCRATCH&PARANOIA_MODE_REPAIR);

	return true;
}

void CdTransport::play()
{
	if (this->playing)
		return;
	if (!this->paranoia)
		return;
	this->seek_lsn(this->read_cursor);
	int16_t *p_readbuf;
	int16_t cdio_readbuf[SAMPLES_PER_CD_FRAME];
	int16_t deemph_buf[SAMPLES_PER_CD_FRAME];
	this->playing = true;
	while (this->playing && this->read_cursor < this->disc_info.disc_last_lsn) {
		int tr = cdio_cddap_sector_gettrack(drive, this->read_cursor);
		if (this->deemph_mode == AUTO) {
			this->deemph.enabled = this->disc_info.track_has_preemph[tr];
		}
		this->do_status_callback(tr);
		this->adjust_retries();
		{
			// atomic section for device access
			std::lock_guard<std::mutex> lk(this->drive_mut);
			if (!this->playing) {
				break;
			}
			cdio_read_audio_sector(this->cdio, &cdio_readbuf, this->read_cursor);
			p_readbuf = cdio_readbuf;
			//p_readbuf = cdio_paranoia_read_limited(
			//	paranoia, NULL, this->paranoia_read_retries);
		}
		if (!p_readbuf) {
			std::cerr << "Paranoia read err. Stopping." << std::endl;
			break;
		}
		if (this->deemph.enabled) {
			deemph.process_samples(deemph_buf, p_readbuf, SAMPLES_PER_CD_FRAME);
			this->data_out->blocking_write(deemph_buf, SAMPLES_PER_CD_FRAME);
		} else {
			this->data_out->blocking_write(p_readbuf, SAMPLES_PER_CD_FRAME);
		}
		this->read_cursor++;
	}
	std::cout << "Exiting playback loop" << std::endl;
	// reset cursor to beginning
	this->read_cursor = this->disc_info.track_first_lsns[1];
	this->playing = false;
	// notify that we've stopped
	do_status_callback(-1);
}

void CdTransport::do_status_callback(int cur_track)
{
	if (this->status_callback) {
		TransportStatus status;
		if (cur_track == -1) {
			status.stopped = true;
		} else {
			status.track_num = cur_track;
			status.lsn_cursor = this->read_cursor;
			status.deemph_active = this->deemph.enabled;
		}
		this->status_callback(status);
	}
}


void CdTransport::adjust_retries()
{
	float fill = this->data_out->get_fill_ratio();
	if (fill < 0.1) {
		this->paranoia_read_retries = 0;
	} else if (fill < 0.25) {
		this->paranoia_read_retries = 1;
	} else if (fill < 0.75) {
		this->paranoia_read_retries = 2;
	} else {
		this->paranoia_read_retries = 3;
	}
}

void CdTransport::seek_track(int track_num)
{
	if (track_num < 1 || track_num > this->disc_info.num_tracks) {
		return;
	}
	this->seek_lsn(this->disc_info.track_first_lsns[track_num]);
}

void CdTransport::seek_prev()
{
	int cur_track = cdio_cddap_sector_gettrack(drive, this->read_cursor);
	if (cur_track > 1) {
		this->seek_lsn(this->disc_info.track_first_lsns[cur_track - 1]);
	} else if (cur_track == 1) {
		this->seek_lsn(this->disc_info.track_first_lsns[1]);
	}
}

void CdTransport::stop()
{
	this->playing = false;
}

void CdTransport::seek_next()
{
	int cur_track = cdio_cddap_sector_gettrack(drive, this->read_cursor);
	if (cur_track < this->disc_info.num_tracks) {
		this->seek_lsn(this->disc_info.track_first_lsns[cur_track + 1]);
	}
}

void CdTransport::seek_lsn(lsn_t lsn)
{
	std::lock_guard<std::mutex> lk(this->drive_mut);
	if (!this->paranoia) {
		return;
	}
	this->data_out->clear();
	this->read_cursor = lsn;
	cdio_paranoia_seek(this->paranoia, this->read_cursor, SEEK_SET);
}

void CdTransport::set_deemph_mode(DeemphMode mode)
{
	this->deemph_mode = mode;
	if (mode == ON) {
		this->deemph.enabled = true;
	} else if (mode == OFF) {
		this->deemph.enabled = false;
	}
}

void CdTransport::eject()
{
	std::lock_guard<std::mutex> lk(this->drive_mut);
	this->playing = false;
	paranoia_free(this->paranoia);
	this->paranoia = NULL;
	cdio_cddap_close_no_free_cdio(drive);
	this->drive = NULL;
	cdio_eject_media(&this->cdio);
}

CdTransport::~CdTransport()
{
	if (this->paranoia)
		paranoia_free(this->paranoia);
	if (this->drive)
		cdio_cddap_close_no_free_cdio(drive);
	if (this->cdio)
		cdio_destroy(cdio);
}
