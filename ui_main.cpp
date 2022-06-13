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

int main(int argc, char **argv) {
	Fl::lock();

	playerUi = new CdPlayerUI(&player);
	playerUi->show(argc, argv);
	Fl::unlock();
	return Fl::run();
}
