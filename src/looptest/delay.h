#pragma once

#define MAX_DELAY_BUF_SIZE 524288 // 4096 * 128

#ifdef __cplusplus
extern "C" {
#endif

	void delay_process(short*  outputbfr, short* inputbfr, size_t numBytes);

#ifdef __cplusplus
}
#endif
