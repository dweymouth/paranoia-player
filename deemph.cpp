#include "deemph.h"

#include <cstring>
#include <cstdlib>

const float Deemph::b0 = 0.46035078;
const float Deemph::b1 = -0.28440821;
const float Deemph::b2 = 0.03388877;
const float Deemph::a1 = -1.05429146;
const float Deemph::a2 = 0.26412280;

// for dithering
float Deemph::rand_flt()
{
	return float(rand()) / (float(RAND_MAX) + 1.0);
}

void Deemph::process_samples(int16_t *outbuf, int16_t *inbuf, int count)
{
	if (count < 2) {
		return;
	}
	if (!this->enabled) {
		memcpy(outbuf, inbuf, count * sizeof(int16_t));
	} else {
		// process first sample pair
		outbuf[0] = (int16_t)(b0*inbuf[0] + b1*xn1L + b2*xn2L - a1*yn1L - a2*yn2L + rand_flt());
		outbuf[1] = (int16_t)(b0*inbuf[1] + b1*xn1R + b2*xn2R - a1*yn1R - a2*yn2R + rand_flt());
		if (count >= 4) {
			// process second sample pair
			outbuf[2] = (int16_t)(b0*inbuf[2] + b1*inbuf[0] + b2*xn1L - a1*outbuf[0] - a2*yn1L + rand_flt());
			outbuf[3] = (int16_t)(b0*inbuf[3] + b1*inbuf[1] + b2*xn1R - a1*outbuf[1] - a2*yn1R + rand_flt());
		}
		for (int i = 4; i < count; i += 2) {
			outbuf[i] = (int16_t)(b0*inbuf[i] + b1*inbuf[i-2] + b2*inbuf[i-4]
			                    - a1*outbuf[i-2] - a2*outbuf[i-4] + rand_flt());
			outbuf[i+1] = (int16_t)(b0*inbuf[i+1] + b1*inbuf[i-1] + b2*inbuf[i-3]
			                    - a1*outbuf[i-1] - a2*outbuf[i-3] + rand_flt());
		}
	}

	// update filter memory
	if (count == 2) {
		xn2L = xn1L;
		xn2R = xn1R;
		xn1L = inbuf[0];
		xn1R = inbuf[1];
		yn2L = yn1L;
		yn2R = yn1R;
		yn1L = outbuf[0];
		yn1R = outbuf[1];
	} else {
		xn2L = inbuf[count - 4];
		xn2R = inbuf[count - 3];
		xn1L = inbuf[count - 2];
		xn1R = inbuf[count - 1];
		yn2L = outbuf[count - 4];
		yn2R = outbuf[count - 3];
		yn1L = outbuf[count - 2];
		yn1R = outbuf[count - 1];
	}
}
