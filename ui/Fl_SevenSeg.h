#ifndef _FL_SEVENSEG_H_
#define _FL_SEVENSEG_H_

#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>

const char SEG_A = 0x01;
const char SEG_B = 0x02;
const char SEG_C = 0x04;
const char SEG_D = 0x08;
const char SEG_E = 0x10;
const char SEG_F = 0x20;
const char SEG_G = 0x40;

class Fl_SevenSeg : public Fl_Widget
{
	public:
		Fl_SevenSeg(int x, int y, int w, int h) :
			Fl_Widget(x, y, w, h) { }

		// set which segments are lit, using bitfield
		void set_segments(char segs) {
			if (this->segs != segs) {
				this->segs = segs;
				damage(0xff);
			}
		}

		void set_digit(int digit);

		void set_colors(Fl_Color lit_seg, Fl_Color bg) {
			this->lit_seg_color = lit_seg;
			this->bg_color = bg;
			damage(0xff);
		}

		void draw();

	private:
		char segs;
		Fl_Color lit_seg_color;
		Fl_Color bg_color;
};

class Fl_SevenSeg2 : public Fl_Widget
{
	public:
		Fl_SevenSeg2(int x, int y, int w, int h) :
			Fl_Widget(x, y, w, h),
			digit_a(x, y, w*7/16, h),
			digit_b(x + w*9/16, y, w*7/16, h)
		{
		}

		void set_segments(char digita_segs, char digitb_segs)
		{
			digit_a.set_segments(digita_segs);
			digit_b.set_segments(digitb_segs);
		}

		void set_colors(Fl_Color lit_seg, Fl_Color bg) {
			this->bg = bg;
			digit_a.set_colors(lit_seg, bg);
			digit_b.set_colors(lit_seg, bg);
		}

		void set_digits(int digita, int digitb)
		{
			digit_a.set_digit(digita);
			digit_b.set_digit(digitb);
		}

		void draw()
		{
			digit_a.draw();
			digit_b.draw();
		}

	private:
		Fl_SevenSeg digit_a;
		Fl_SevenSeg digit_b;
		Fl_Color bg;
};

class Fl_SevenSegTime : public Fl_Widget
{
	public:
		Fl_SevenSegTime(int x, int y, int w, int h) :
			Fl_Widget(x, y, w, h),
			min_digits(x, y, w*16/37, h),
			sec_digits(x + w*21/37, y, w*16/37, h)
		{
		}

		void set_segments(char a_segs, char b_segs, bool colon, char c_segs, char d_segs);

		void set_colors(Fl_Color lit_seg, Fl_Color bg) {
			this->lit_seg = lit_seg;
			this->bg = bg;
			min_digits.set_colors(lit_seg, bg);
			sec_digits.set_colors(lit_seg, bg);
		}

		void set_time(int min, int sec)
		{
			min_digits.set_digits(min / 10, min % 10);
			sec_digits.set_digits(sec / 10, sec % 10);
			damage(0xff);
			colon = true;
		}	

		void draw()
		{
			min_digits.draw();
			sec_digits.draw();
			draw_colon();
		}
	private:
		Fl_SevenSeg2 min_digits;
		Fl_SevenSeg2 sec_digits;
		Fl_Color lit_seg;
		Fl_Color bg;
		bool colon;

		void draw_colon()
		{
			int grid_x = w() / 37;
			int grid_y = h() / 13;
			fl_rectf(x()+grid_x*18, y()+grid_y*3, grid_x, grid_y, colon ? lit_seg : bg);
			fl_rectf(x()+grid_x*18, y()+grid_y*8, grid_x, grid_y, colon ? lit_seg : bg);
		}

};

#endif
