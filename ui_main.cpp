#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>

#include "player/cd_player.h"
#include "ui/CdPlayerUI.h"

#include <unistd.h>
#include <thread>

CdPlayerUI *playerUi;
CdPlayer player(true);

// test
void update_digits()
{
	for (int i = 0; i < 99; i++)
	{
		Fl::lock();
		playerUi->disp->track_disp.set_digits(i / 10, i % 10);
		playerUi->disp->time_disp.set_time(i, i);
		playerUi->track_pos->value((float)i/100);
		Fl::unlock();
		Fl::awake();
		sleep(1);
	}
}

int main(int argc, char **argv) {
	Fl::lock();

	playerUi = new CdPlayerUI(&player);
	playerUi->show(argc, argv);
	Fl::unlock();
	//std::thread update_digits_thr(update_digits);
	//update_digits_thr.detach();
	return Fl::run();
}
