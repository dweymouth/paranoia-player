#include <iostream>
#include <math.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include <cdio/paranoia/cdda.h>
#include <cdio/paranoia/paranoia.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <thread>

#include "audio_out.h"
#include "blocking_queue.h"
#include "cd_player.h"
#include "cd_transport.h"
#include "deemph.h"

using namespace std;

CdPlayer player;

void print_time(int min, int sec)
{
	cout << min << ":" << (sec < 10 ? "0" : "") << sec;
}

void print_status_msg()
{
	if (player.state == STOPPED) {
		cout << "stopped" << endl;
		return;
	}
	cout << (player.state == PAUSED ? "paused " : "playing ");
	cout << "track " << player.cur_track << " ";
	print_time(player.track_min, player.track_sec);
	if (player.deemph_active) {
		cout << " deemph";
	}
	cout << endl;
}

void get_min_sec(int sec, int *pmin, int *psec)
{
	*pmin = sec / 60;
	*psec = sec % 60;
}

void wait_for_disc_then_print_info()
{
	if (!player.have_disc) {
		cout << "Waiting for disc..." << endl;
		if (!player.wait_and_load_disc()) {
			cerr << "Unable to load disc." << endl;
			exit(77);
		}
		// print disc info
		DiscInfo *info = player.get_disc_info();
		cout << "Disc has " << info->num_tracks << "tracks. Total time ";
		int min, sec;
		get_min_sec(info->disc_duration_secs(), &min, &sec);
		print_time(min, sec);
		cout << endl;
		for (int i = 1; i <= info->num_tracks; i++) {
			cout << "Track " << i << " (";
			get_min_sec(info->track_duration_secs(i), &min, &sec);
			print_time(min, sec);
			cout << ")" << endl;
		}
		cout << "Type 'play' to start playback, or 'help' to list available commands." << endl;
	}
}

void print_help()
{
	cout << "Available commands: " << endl;
	cout << "play" << endl << "pause" << endl << "exit" << endl << "status" << endl;
	cout << "seek-next" << endl << "seek-prev" << endl;
	cout << "seek-track <track-num>" << endl;
	cout << "deemph [on|off|auto]" << endl;
}


void do_main_loop()
{
	string line;
	while (true) {
		wait_for_disc_then_print_info();
		cout << "> ";
		// while we have a CD, enter cmd loop
		getline(cin, line);
		stringstream stream(line);
		string cmd;
		stream >> cmd;
		if (cmd == "play") {
			player.play_disc();
		} else if (cmd == "help") {
			print_help();
		}else if (cmd == "stop") {
			// stop isn't working right now
			cout << "Stop is currently unsupported. Use pause or exit instead." << endl;
			//player.stop();
		} else if (cmd == "seek-next") {
			player.seek_next();
		} else if (cmd == "seek-prev") {
			player.seek_prev();
		} else if (cmd == "pause") {
			player.pause();
		} else if (cmd == "exit") {
			exit(0);	
		} else if (cmd == "deemph") {
			string deemph_mode;
			stream >> deemph_mode;
			if (deemph_mode == "on") {
				player.set_deemph_mode(ON);
			} else if (deemph_mode == "off") {
				player.set_deemph_mode(OFF);
			} else if (deemph_mode == "auto") {
				player.set_deemph_mode(AUTO);
			}
		} else if (cmd == "seek-track") {
			string track_num_str;
			stream >> track_num_str;
			int track_num = -1;
			try {
				track_num = stoi(track_num_str);
			} catch (exception &e) { }
			if (track_num > 0) {
				player.seek_track(track_num);
			}
		} else if (cmd == "status") {
			print_status_msg();
		} else if (cmd == "eject") {
			player.eject();
		}
		line.clear();
	}
}

int main(int argc, const char *argv[])
{
	do_main_loop();
}
