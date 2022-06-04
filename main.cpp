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


CdPlayer player;

void get_min_sec(int time_sec, int *min, int *sec) {
	*min = time_sec / 60;
	*sec = time_sec - (*min * 60);
}

void read_cmds()
{
	std::string line;
	while (true) {
		std::getline(std::cin, line);
		std::stringstream stream(line);
		std::string cmd;
		stream >> cmd;
		if (cmd == "seek-next") {
			player.seek_next();
		} else if (cmd == "seek-prev") {
			player.seek_prev();
		} else if (cmd == "pause") {
			player.pause();
		} else if (cmd == "exit") {
			std::exit(0);	
		} else if (cmd == "deemph") {
			std::string deemph_mode;
			stream >> deemph_mode;
			if (deemph_mode == "on") {
				player.set_deemph_mode(ON);
			} else if (deemph_mode == "off") {
				player.set_deemph_mode(OFF);
			} else if (deemph_mode == "auto") {
				player.set_deemph_mode(AUTO);
			}
		} else if (cmd == "seek-track") {
			std::string track_num_str;
			stream >> track_num_str;
			int track_num = -1;
			try {
				track_num = stoi(track_num_str);
			} catch (std::exception &e) { }
			if (track_num > 0) {
				player.seek_track(track_num);
			}
		}
		line.clear();
	}
}

int main(int argc, const char *argv[])
{
	if (!player.wait_and_load_disc()) {
		std::cerr << "Unable to load disc." << std::endl;
		exit(77);
	}
	std::thread cmd_thread(read_cmds);
	if (!player.play_disc()) {
		std::cerr << "Unable to initialize audio output." << std::endl;
	}
	cmd_thread.join();
}
