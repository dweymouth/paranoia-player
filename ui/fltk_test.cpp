#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>

#include "Fl_SevenSeg.h"
#include "TrackTimeDisplay.h"

#include <unistd.h>
#include <thread>

void play_callback(Fl_Widget *w, void *data) {
}

void stop_callback(Fl_Widget *w, void *data) {
}

int btn_row_y = 125;
int btn_pad = 10;
int btn_w = 50;

Fl_Window *window;
Fl_SevenSeg2* digits;
Fl_SevenSegTime* time_disp;
Fl_Slider* track_pos;

TrackTimeDisplay* disp;

// test
void update_digits()
{
	for (int i = 0; i < 99; i++)
	{
		Fl::lock();
		disp->track_disp.set_digits(i / 10, i % 10);
		disp->time_disp.set_time(i, i);
		track_pos->value((float)i/100);
		Fl::unlock();
		Fl::awake();
		sleep(1);
	}
}

int main(int argc, char **argv) {
	Fl::lock();
	window = new Fl_Window(325, 185);
	window->color(FL_DARK2);
	
	disp = new TrackTimeDisplay(10, 10, 290, 80);
	disp->set_colors(FL_DARK_GREEN, fl_darker(FL_DARK3));

	track_pos = new Fl_Slider(10, 100, 290, 20);
	track_pos->type(FL_HOR_NICE_SLIDER);
	track_pos->color(FL_DARK2, FL_DARK2);
	//track_pos->bounds(0, 100);

       	Fl_Button *stop = new Fl_Button(btn_pad, btn_row_y, 50, 50, "@+2square");
	stop->color(FL_DARK2);
	stop->callback(stop_callback, NULL);

	Fl_Button *play = new Fl_Button(btn_pad*2+btn_w, btn_row_y, btn_w, 50, "@+3>");
	play->color(FL_DARK2);
	play->callback(play_callback, NULL);
	//play->box(FL_ROUND_UP_BOX);

	Fl_Button *eject = new Fl_Button(btn_pad*3+btn_w*2, btn_row_y, btn_w, 50, "@+38|>");
	eject->color(FL_DARK2);
	eject->align(FL_ALIGN_BOTTOM & FL_ALIGN_INSIDE );

	Fl_Button *prev = new Fl_Button(btn_pad*4+btn_w*3, btn_row_y, btn_w, 50, "@+2|<");
	prev->color(FL_DARK2);

	Fl_Button *next = new Fl_Button(btn_pad*5+btn_w*4, btn_row_y, btn_w, 50, "@+2>|");
	next->color(FL_DARK2);
	window->end();

	fl_rectf(10, 10, 280, 60, fl_darker(FL_DARK3));
	std::thread update_digits_thr(update_digits);
	update_digits_thr.detach();

	window->show(argc, argv);
	Fl::unlock();
	return Fl::run();
}
