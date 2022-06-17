#ifndef _CD_PLAYER_UI_H
#define _CD_PLAYER_UI_H

#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Slider.H>
#include "TrackTimeDisplay.h"
#include "../player/cd_player.h"

class CdPlayerUI : public Fl_Window
{
	public:
		CdPlayerUI(CdPlayer *player);
		CdPlayer *player;
		TrackTimeDisplay *disp;
		Fl_Slider *track_pos;
		Fl_Button *stop;
		Fl_Button *eject;
		Fl_Button *play;
		Fl_Button *prev;
		Fl_Button *next;
	private:
		void display_update_routine();
		void disable_seek_bar();
		void setup_menu_bar();
};

#endif
