#define MINIAUDIO_IMPLEMENTATION
#include "3rdparty/miniaudio.h"
#include "audio_out.h"
#include "blocking_queue.h"


AudioOut::AudioOut(CircularBlockingQueue<int16_t> *data_source)
{
	this->data_source = data_source;
}

// callback to supply next samples of audio
void ma_audio_callback(ma_device *p_device, void *p_output, const void *p_input, ma_uint32 frame_count)
{
	auto *p_data_source = (CircularBlockingQueue<int16_t>*)p_device->pUserData;
	int num_samples_written = p_data_source->non_blocking_read(frame_count * 2, (int16_t*)p_output);
	frame_count -= num_samples_written / 2;
	// in case of buffer underrun, copy zeros for rest of requested samples
	memset(((int32_t*)p_output) + num_samples_written, 0, frame_count * 4);
}

bool AudioOut::init()
{
	ma_device_config config = ma_device_config_init(ma_device_type_playback);
	config.playback.format = ma_format_s16;
	config.playback.channels = 2;
	config.sampleRate = 44100;
	config.dataCallback = ma_audio_callback;
	config.pUserData = this->data_source;

	if (ma_device_init(NULL, &config, &this->device) != MA_SUCCESS) {
		return false;
	}
	return true;
}

void AudioOut::start()
{
	ma_device_start(&this->device);
}

void AudioOut::stop()
{
	ma_device_stop(&this->device);
}

AudioOut::~AudioOut()
{
	ma_device_uninit(&this->device);
}
