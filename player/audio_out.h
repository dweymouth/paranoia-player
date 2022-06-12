#include "3rdparty/miniaudio.h"
#include "blocking_queue.h"

#ifndef _AUDIO_OUT_H_
#define _AUDIO_OUT_H_

class AudioOut
{
	public:
		AudioOut(CircularBlockingQueue<int16_t> *data_source);
		~AudioOut();
		bool init();
		void start();
		void stop();
	private:
		CircularBlockingQueue<int16_t>* data_source;
		ma_device device;
};

#endif
