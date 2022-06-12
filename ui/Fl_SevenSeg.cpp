#include "Fl_SevenSeg.h"
#include <FL/fl_draw.H>

void Fl_SevenSeg::draw()
{
	fl_rectf(x(), y(), w(), h(), bg_color);
	int grid_x = w() / 7;
	int grid_y = h() / 13;
	fl_rectf(x()+grid_x, y(), grid_x*5, grid_y, (segs & SEG_A) ? lit_seg_color : bg_color);
	fl_rectf(x()+grid_x*6, y()+grid_y, grid_x, grid_y*5, (segs & SEG_B) ? lit_seg_color : bg_color);
	fl_rectf(x()+grid_x*6, y()+grid_y*7, grid_x, grid_y*5, (segs & SEG_C) ? lit_seg_color : bg_color);
	fl_rectf(x()+grid_x, y()+grid_y*12, grid_x*5, grid_y, (segs & SEG_D) ? lit_seg_color : bg_color);
	fl_rectf(x(), y()+grid_y*7, grid_x, grid_y*5, (segs & SEG_E) ? lit_seg_color : bg_color);
	fl_rectf(x(), y()+grid_y, grid_x, grid_y*5, (segs & SEG_F) ? lit_seg_color : bg_color);
	fl_rectf(x()+grid_x, y()+grid_y*6, grid_x*5, grid_y, (segs & SEG_G) ? lit_seg_color : bg_color);
}

void Fl_SevenSeg::set_digit(int digit)
{
	switch (digit) {
	case 0:
		set_segments(SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F);
		break;
	case 1:
		set_segments(SEG_B | SEG_C);
		break;
	case 2:
		set_segments(SEG_A | SEG_B | SEG_D | SEG_E | SEG_G);
		break;
	case 3:
		set_segments(SEG_A | SEG_B | SEG_C | SEG_D | SEG_G);
		break;
	case 4:
		set_segments(SEG_B | SEG_C | SEG_F | SEG_G);
		break;
	case 5:
		set_segments(SEG_A | SEG_C | SEG_D | SEG_F | SEG_G);
		break;
	case 6:
		set_segments(SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G);
		break;
	case 7:
		set_segments(SEG_A | SEG_B | SEG_C);
		break;
	case 8:
		set_segments(SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G);
		break;
	case 9:
		set_segments(SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G);
		break;
	}
	damage(0xff);
}
