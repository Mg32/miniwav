
#ifndef __MINIWAV_H__
#define __MINIWAV_H__

#include <stddef.h>

typedef enum waverror {
    ERROR_OK,
    // common
    ERROR_UNKNOWN,
    ERROR_UNABLE_TO_OPEN,
    ERROR_MEMORY_ALLOC,
    // read
    ERROR_NOT_WAVE,
    ERROR_BROKEN,
    ERROR_UNSUPPORTED,
    // write
    ERROR_BAD_DATA,
    ERROR_WRITE_FALUT,
} waverror_t;

typedef enum wavtype {
    TYPE_UNKNOWN,
    TYPE_UINT8,
    TYPE_INT16,
    TYPE_INT24,
    TYPE_INT32,
    TYPE_FLOAT
} wavtype_t;

typedef struct wavdata {
    int samplerate;
    size_t channels;
    size_t size;
    double *data;
    wavtype_t type;
} wavdata_t;

waverror_t wav_read_file(wavdata_t *wave, const char *filename);
waverror_t wav_write_file(const wavdata_t *wave, const char *filename);

#endif
