SOURCES="player/audio_out.cpp player/cd_transport.cpp player/cd_player.cpp player/deemph.cpp main.cpp"

g++ -std=c++11 -I /opt/local/include/ -L/opt/local/lib $SOURCES -lcdio -lcdio_cdda -lcdio_paranoia
