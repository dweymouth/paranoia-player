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
	this->disc_first_lsn = cdda_disc_firstsector(this->drive);
	this->disc_last_lsn = cdda_disc_lastsector(this->drive);
	this->num_tracks = cdio_cddap_tracks(this->drive);
	for (int t = 0; t <= num_tracks; t++) {
		this->track_first_lsns[t] = cdio_cddap_track_firstsector(drive, t);
		this->track_last_lsns[t] = cdio_cddap_track_lastsector(drive, t);
		this->track_has_preemph[t] =
			(cdio_get_track_preemphasis(cdio, t) == CDIO_TRACK_FLAG_TRUE);
	}
	this->read_cursor = this->disc_first_lsn;

	this->paranoia = paranoia_init(drive);
	paranoia_modeset(this->paranoia, PARANOIA_MODE_SCRATCH&PARANOIA_MODE_REPAIR);

	return true;
}

void CdTransport::play()
{
	int16_t deemph_buf[SAMPLES_PER_CD_FRAME];
	while (this->read_cursor < this->disc_last_lsn) {
		int tr = cdio_cddap_sector_gettrack(drive, this->read_cursor);
		if (this->deemph_mode == AUTO) {
			this->deemph.enabled = this->track_has_preemph[tr];
		}
		if (this->status_callback) {
			TransportStatus status;
			status.track_num = tr;
			this->get_track_min_sec(tr, &status.track_min, &status.track_sec);
			status.deemph_active = this->deemph.enabled;
			this->status_callback(status);
		}
		int16_t *p_readbuf = cdio_paranoia_read_limited(
				paranoia, NULL, this->paranoia_read_retries);
		if (!p_readbuf) {
			std::cerr << "Paranoia read err. Stopping." << std::endl;
			break;
		}
		deemph.process_samples(deemph_buf, p_readbuf, SAMPLES_PER_CD_FRAME);
		this->data_out->blocking_write(deemph_buf, SAMPLES_PER_CD_FRAME);
		this->read_cursor++;
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
	if (track_num > this->num_tracks) {
		return;
	}
	this->seek_lsn(this->track_first_lsns[track_num]);
}

void CdTransport::seek_prev()
{
	int cur_track = cdio_cddap_sector_gettrack(drive, this->read_cursor);
	if (cur_track > 1) {
		this->seek_lsn(this->track_first_lsns[cur_track - 1]);
	}
}

void CdTransport::seek_next()
{
	int cur_track = cdio_cddap_sector_gettrack(drive, this->read_cursor);
	if (cur_track < this->num_tracks) {
		this->seek_lsn(this->track_first_lsns[cur_track + 1]);
	}
}

void CdTransport::seek_lsn(lsn_t lsn)
{
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

void CdTransport::get_track_min_sec(int cur_tr, int *min, int *sec)
{
	lsn_t lsn_offs = this->read_cursor - this->track_first_lsns[cur_tr];
	*sec = (lsn_offs * SAMPLES_PER_CD_FRAME - this->data_out->get_count()) / 88200;
	*min = *sec / 60;
	*sec = *sec % 60;
}

CdTransport::~CdTransport()
{
	paranoia_free(this->paranoia);
	cdio_cddap_close_no_free_cdio(drive);
	cdio_destroy(cdio);
}
