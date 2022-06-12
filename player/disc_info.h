#ifndef _DISC_INFO_H
#define _DISC_INFO_H

#include <cdio/paranoia/cdda.h>
#include <cdio/cd_types.h>

class DiscInfo
{
	public:
		lsn_t disc_first_lsn;
		lsn_t disc_last_lsn;
		int num_tracks;
		// CDs only have up to 99 tracks (+ track 0?) so just reserve 100
		lsn_t track_first_lsns[100];
		lsn_t track_last_lsns[100];
		bool track_has_preemph[100];

		int track_duration_secs(int track_num)
		{
			if (track_num < 1 || track_num > num_tracks) {
				return -1;
			}
			return (track_last_lsns[track_num] - track_first_lsns[track_num]) / 75;
		}

		int disc_duration_secs()
		{
			return (disc_last_lsn - disc_first_lsn) / 75;
		}
};	

#endif
