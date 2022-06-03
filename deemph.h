#ifndef _DEEMPH_H_
#define _DEEMPH_H_

#include <atomic>
#include <stdint.h>

// Red Book (15/50us) de-emphasis filter taken from SoX
// See 3rdparty/deemph.plt for more details
class Deemph
{
	public:
		bool enabled;
		void process_samples(int16_t *outbuf, int16_t *inbuf, int count);

	private:
		// filter coefficients (definition in .cpp)
		static const float b0;
		static const float b1;
		static const float b2;
		static const float a1;
		static const float a2;

		// filter memory: x(n-1) (left/right) etc
		int16_t xn1L;
		int16_t xn1R;
		int16_t xn2L;
		int16_t xn2R;
		int16_t yn1L;
		int16_t yn1R;
		int16_t yn2L;
		int16_t yn2R;

		// for dithering
		static float rand_flt();
};

#endif
