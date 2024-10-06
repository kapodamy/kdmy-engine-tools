#include "pl_mpeg.h"

static inline uint8_t plm_clamp(int n) {
	if (n > 255) {
		n = 255;
	}
	else if (n < 0) {
		n = 0;
	}
	return n;
}

// YCbCr conversion following the BT.601 standard:
// https://infogalactic.com/info/YCbCr#ITU-R_BT.601_conversion

#define PLM_PUT_PIXEL(RI, GI, BI, Y_OFFSET, DEST_OFFSET) \
	y = ((frame->y.data[y_index + Y_OFFSET]-16) * 76309) >> 16; \
	dest[d_index + DEST_OFFSET + RI] = plm_clamp(y + r); \
	dest[d_index + DEST_OFFSET + GI] = plm_clamp(y - g); \
	dest[d_index + DEST_OFFSET + BI] = plm_clamp(y + b);

#define PLM_DEFINE_FRAME_CONVERT_FUNCTION(NAME, BYTES_PER_PIXEL, RI, GI, BI) \
	void NAME(plm_frame_t *frame, uint8_t *dest, int stride) { \
		int cols = frame->width >> 1; \
		int rows = frame->height >> 1; \
		int yw = frame->y.width; \
		int cw = frame->cb.width; \
		for (int row = 0; row < rows; row++) { \
			int c_index = row * cw; \
			int y_index = row * 2 * yw; \
			int d_index = row * 2 * stride; \
			for (int col = 0; col < cols; col++) { \
				int y; \
				int cr = frame->cr.data[c_index] - 128; \
				int cb = frame->cb.data[c_index] - 128; \
				int r = (cr * 104597) >> 16; \
				int g = (cb * 25674 + cr * 53278) >> 16; \
				int b = (cb * 132201) >> 16; \
				PLM_PUT_PIXEL(RI, GI, BI, 0,      0); \
				PLM_PUT_PIXEL(RI, GI, BI, 1,      BYTES_PER_PIXEL); \
				PLM_PUT_PIXEL(RI, GI, BI, yw,     stride); \
				PLM_PUT_PIXEL(RI, GI, BI, yw + 1, stride + BYTES_PER_PIXEL); \
				c_index += 1; \
				y_index += 2; \
				d_index += 2 * BYTES_PER_PIXEL; \
			} \
		} \
	}

PLM_DEFINE_FRAME_CONVERT_FUNCTION(plm_frame_to_rgb,  3, 0, 1, 2)
PLM_DEFINE_FRAME_CONVERT_FUNCTION(plm_frame_to_bgr,  3, 2, 1, 0)
PLM_DEFINE_FRAME_CONVERT_FUNCTION(plm_frame_to_rgba, 4, 0, 1, 2)
PLM_DEFINE_FRAME_CONVERT_FUNCTION(plm_frame_to_bgra, 4, 2, 1, 0)
PLM_DEFINE_FRAME_CONVERT_FUNCTION(plm_frame_to_argb, 4, 1, 2, 3)
PLM_DEFINE_FRAME_CONVERT_FUNCTION(plm_frame_to_abgr, 4, 3, 2, 1)


#undef PLM_PUT_PIXEL
#undef PLM_DEFINE_FRAME_CONVERT_FUNCTION



