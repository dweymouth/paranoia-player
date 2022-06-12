
SOURCES="player/audio_out.cpp player/cd_transport.cpp player/cd_player.cpp player/deemph.cpp"

UI_SOURCES="ui/CdPlayerUI.cpp ui/Fl_SevenSeg.cpp ui_main.cpp"

g++ -std=c++11 -I/opt/local/include -L/opt/local/lib $SOURCES $UI_SOURCES -lcdio -lcdio_cdda -lcdio_paranoia -lfltk
