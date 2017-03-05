#pragma once

#define MAX_DELAY_BUF_SIZE 524288 // 4096 * 128

#ifdef __cplusplus
extern "C" {
#endif

	void delay_process(signed __int16* outputbfr, signed __int16* inputbfr, size_t numBytes);

#ifdef __cplusplus
}
#endif