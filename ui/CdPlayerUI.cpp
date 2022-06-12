#include "CdPlayerUI.h"
#include "../player/cd_player.h"

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>

#include "Fl_SevenSeg.h"
#include "TrackTimeDisplay.h"

void play_callback(Fl_Widget *w, void *data) {
}

void stop_callback(Fl_Widget *w, void *data) {
}

CdPlayerUI::CdPlayerUI(CdPlayer *player) :
	Fl_Window(325, 185)
{
	const int btn_row_y = 125;
	const int btn_pad = 10;
	const int btn_w = 50;

	this->color(FL_DARK2);
	
	disp = new TrackTimeDisplay(10, 10, 290, 80);
	disp->set_colors(FL_DARK_GREEN, fl_darker(FL_DARK3));

	track_pos = new Fl_Slider(10, 100, 290, 20);
	track_pos->type(FL_HOR_NICE_SLIDER);
	track_pos->color(FL_DARK2, FL_DARK2);
	//track_pos->bounds(0, 100);

       	stop = new Fl_Button(btn_pad, btn_row_y, 50, 50, "@+2square");
	stop->color(FL_DARK2);
	stop->callback(stop_callback, NULL);

	play = new Fl_Button(btn_pad*2+btn_w, btn_row_y, btn_w, 50, "@+3>");
	play->color(FL_DARK2);
	play->callback(play_callback, NULL);
	//play->box(FL_ROUND_UP_BOX);

	eject = new Fl_Button(btn_pad*3+btn_w*2, btn_row_y, btn_w, 50, "@+38|>");
	eject->color(FL_DARK2);
	eject->align(FL_ALIGN_BOTTOM & FL_ALIGN_INSIDE );

	prev = new Fl_Button(btn_pad*4+btn_w*3, btn_row_y, btn_w, 50, "@+2|<");
	prev->color(FL_DARK2);

	next = new Fl_Button(btn_pad*5+btn_w*4, btn_row_y, btn_w, 50, "@+2>|");
	next->color(FL_DARK2);
	this->end();
}
