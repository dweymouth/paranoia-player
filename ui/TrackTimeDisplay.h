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
			time_lbl(x+150, y+40, 40, 50, "Time"),
			deemph_lbl(x+210, y+20, 80, 50, "")
		{
			deemph_lbl.labelsize(11);
		}

		Fl_SevenSeg2 track_disp;
		Fl_SevenSegTime time_disp;

		void set_colors(Fl_Color fg, Fl_Color bg)
		{
			this->bg = bg;
			track_disp.set_colors(fg, bg);
			time_disp.set_colors(fg, bg);
			track_lbl.labelcolor(fg);
			time_lbl.labelcolor(fg);
			deemph_lbl.labelcolor(fg);
		}

		void set_track_num(int tr)
		{
			track_disp.set_digits(tr / 10, tr % 10);
		}

		void set_time(int min, int sec)
		{
			time_disp.set_time(min, sec);
		}

		void set_no_disc()
		{
			track_disp.set_segments(SEG_C | SEG_E | SEG_G /*n*/, SEG_C | SEG_D | SEG_E | SEG_G/*o*/);
			time_disp.set_segments(
				SEG_B | SEG_C | SEG_D | SEG_E | SEG_G, //d
				SEG_B | SEG_C /*I*/, false /*no colon*/,
				SEG_A | SEG_C | SEG_D | SEG_F | SEG_G, //s 
				SEG_A | SEG_D | SEG_E | SEG_F); // C
		}

		void set_deemphasis(bool on)
		{
			deemph_lbl.label(on ? "De-Emphasis" : "");
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
		Fl_Box deemph_lbl;
		Fl_Color bg;
};

#endif
