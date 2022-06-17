#include "CdPlayerUI.h"
#include "../player/cd_player.h"
#include "../player/disc_info.h"

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Sys_Menu_Bar.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>

#include <chrono>
#include <thread>

#include "Fl_SevenSeg.h"
#include "TrackTimeDisplay.h"

#ifdef __APPLE__
#define MENU_BAR_HEIGHT 0
#else
#define MENU_BAR_HEIGHT 25
#endif

void play_callback(Fl_Widget *w, void *data) {
	CdPlayerUI *ui = (CdPlayerUI*)data;
	if (!ui->player)
		return;
	ui->player->play_pause();
}

void stop_callback(Fl_Widget *w, void *data) {
	CdPlayerUI *ui = (CdPlayerUI*)data;
	if (!ui->player)
		return;
	ui->player->stop();
}

void eject_callback(Fl_Widget *w, void *data) {
	CdPlayerUI *ui = (CdPlayerUI*)data;
	if (!ui->player)
		return;
	ui->player->eject();
}

void prev_callback(Fl_Widget *w, void *data) {
	CdPlayerUI *ui = (CdPlayerUI*)data;
	if (ui->player)
		ui->player->seek_prev();
}

void next_callback(Fl_Widget *w, void *data) {
	CdPlayerUI *ui = (CdPlayerUI*)data;
	if (ui->player)
		ui->player->seek_next();
}

void track_pos_callback(Fl_Widget *w, void *data) {
	CdPlayerUI *ui = (CdPlayerUI*)data;
	if (ui->player)
		ui->player->seek_trackpos(((Fl_Slider*)w)->value());
}

void deemph_auto_callback(Fl_Widget *w, void *data) {
	CdPlayerUI *ui = (CdPlayerUI*)data;
	if (ui->player)
		ui->player->set_deemph_mode(AUTO);
}

void deemph_on_callback(Fl_Widget *w, void *data) {
	CdPlayerUI *ui = (CdPlayerUI*)data;
	if (ui->player)
		ui->player->set_deemph_mode(ON);
}

void deemph_off_callback(Fl_Widget *w, void *data) {
	CdPlayerUI *ui = (CdPlayerUI*)data;
	if (ui->player)
		ui->player->set_deemph_mode(OFF);
}

CdPlayerUI::CdPlayerUI(CdPlayer *player) :
	Fl_Window(310, 185 + MENU_BAR_HEIGHT)
{
	this->player = player;

	const int btn_row_y = 125 + MENU_BAR_HEIGHT;
	const int btn_pad = 10;
	const int btn_w = 50;

	this->color(FL_DARK2);

	disp = new TrackTimeDisplay(10, 10 + MENU_BAR_HEIGHT, 290, 80);
	disp->set_colors(FL_DARK_GREEN, fl_darker(FL_DARK3));
	disp->set_no_disc();

	track_pos = new Fl_Slider(10, 100 + MENU_BAR_HEIGHT, 290, 20);
	track_pos->type(FL_HOR_NICE_SLIDER);
	track_pos->color(FL_DARK2, FL_DARK2);
	track_pos->range(0., 1.);
	track_pos->when(FL_WHEN_RELEASE);
	track_pos->callback(track_pos_callback, this);

    stop = new Fl_Button(btn_pad, btn_row_y, 50, 50, "@+2square");
	stop->color(FL_DARK2);
	stop->callback(stop_callback, this);

	play = new Fl_Button(btn_pad*2+btn_w, btn_row_y, btn_w, 50, "@+3>");
	play->color(FL_DARK2);
	play->callback(play_callback, this);

	eject = new Fl_Button(btn_pad*3+btn_w*2, btn_row_y, btn_w, 50, "@+38|>");
	eject->color(FL_DARK2);
	eject->align(FL_ALIGN_BOTTOM & FL_ALIGN_INSIDE );
	eject->callback(eject_callback, this);

	prev = new Fl_Button(btn_pad*4+btn_w*3, btn_row_y, btn_w, 50, "@+2|<");
	prev->color(FL_DARK2);
	prev->callback(prev_callback, this);

	next = new Fl_Button(btn_pad*5+btn_w*4, btn_row_y, btn_w, 50, "@+2>|");
	next->color(FL_DARK2);
	next->callback(next_callback, this);

	setup_menu_bar();

	this->end();

	if (player) {
		std::thread update_thr(std::bind(&CdPlayerUI::display_update_routine, this));
		update_thr.detach();
	}
}

void CdPlayerUI::setup_menu_bar()
{
	Fl_Sys_Menu_Bar *menubar = new Fl_Sys_Menu_Bar(0, 0, w(), MENU_BAR_HEIGHT);
	menubar->add("Options/De-Emphasis/Auto", 0, deemph_auto_callback, this, FL_MENU_RADIO | FL_MENU_VALUE);
	menubar->add("Options/De-Emphasis/On", 0, deemph_on_callback, this, FL_MENU_RADIO);
	menubar->add("Options/De-Emphasis/Off", 0, deemph_off_callback, this, FL_MENU_RADIO);
}

void CdPlayerUI::disable_seek_bar()
{
	track_pos->value(0.);
	if (track_pos->active())
		track_pos->deactivate();
}

void CdPlayerUI::display_update_routine()
{
	while (true) {
		Fl::lock();
		if (!player->have_disc) {
			disp->set_no_disc();
			disable_seek_bar();
		} else {
			DiscInfo *info = player->get_disc_info();
			PlayerState state = player->state;
			switch (state) {
			case STOPPING:
			case STOPPED:
				disable_seek_bar();
				disp->set_track_num(info->num_tracks);
				disp->set_time((int)info->disc_duration_secs() / 60, (int)info->disc_duration_secs() % 60);
				disp->set_deemphasis(false);
				break;
			case PLAYING:
			case PAUSED:
				if (!track_pos->active())
					track_pos->activate();
				disp->set_track_num(player->cur_track);
				disp->set_time((int)player->track_time_sec / 60, (int)player->track_time_sec % 60);
				track_pos->value(player->track_time_sec / info->track_duration_secs(player->cur_track));
				disp->set_deemphasis(player->deemph_active);
			}
		}
		Fl::unlock();
		Fl::awake();
		std::this_thread::sleep_for(std::chrono::milliseconds(250));
	}
}
