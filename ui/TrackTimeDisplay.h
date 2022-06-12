#ifndef _TRACK_TIME_DISPLAY_H
#define _TRACK_TIME_DISPLAY_H

#include <FL/Fl_Widget.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>
#include "Fl_SevenSeg.h"

class TrackTimeDisplay : public Fl_Widget
{
	public:
		TrackTimeDisplay(int x, int y, int w, int h) :
			Fl_Widget(x, y, w, h),
			track_disp(x+15, y+10, 3*16, 3*13),
			time_disp(x+90, y+10, 3*37, 3*13),
			track_lbl(x+15, y+40, 50, 50, "Track"),
			time_lbl(x+150, y+40, 40, 50, "Time")
		{ }

		Fl_SevenSeg2 track_disp;
		Fl_SevenSegTime time_disp;

		void set_colors(Fl_Color fg, Fl_Color bg)
		{
			this->bg = bg;
			track_disp.set_colors(fg, bg);
			time_disp.set_colors(fg, bg);
			track_lbl.labelcolor(fg);
			time_lbl.labelcolor(fg);
		}

		void draw()
		{
			fl_rectf(x(), y(), w(), h(), bg);
			track_disp.draw();
			time_disp.draw();
		}
	private:
		Fl_Box track_lbl;
		Fl_Box time_lbl;
		Fl_Color bg;
};

#endif
