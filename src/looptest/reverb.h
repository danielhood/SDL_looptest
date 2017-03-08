#pragma once

#define MAX_DELAY_BUF_SIZE 524288 // 4096 * 128

#ifdef __cplusplus
extern "C" {
#endif

	void reverb_init();
	void reverb_process(short*  outputbfr, short* inputbfr, size_t numBytes);

#ifdef __cplusplus
}
#endif
