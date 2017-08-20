/*
    miniwav : A quick & dirty C library for reading/writing WAV audio file.
    Copyright (C) 2017 mogesystem

    This software is provided 'as-is', without any express or implied
    warranty.  In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
       claim that you wrote the original software. If you use this software
       in a product, an acknowledgment in the product documentation would be
       appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be
       misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.
*/

#ifndef __MINIWAV_H__
#define __MINIWAV_H__

#include <stddef.h>

typedef enum waverror {
    ERROR_OK,
    // common
    ERROR_UNKNOWN,
    ERROR_MEMORY_ALLOC,
    ERROR_UNSUPPORTED,
    ERROR_UNABLE_TO_OPEN,
    // read
    ERROR_NOT_WAVE,
    ERROR_BROKEN,
    // write
    ERROR_BAD_DATA,
    ERROR_WRITE_FAULT,
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
    unsigned samplerate;
    unsigned channels;
    size_t size;
    double *data;
    wavtype_t type;
} wavdata_t;

waverror_t wav_create(wavdata_t *wave, unsigned samplerate, unsigned ch, size_t frames);
void wav_destroy(wavdata_t *wave);
waverror_t wav_read_file(wavdata_t *wave, const char *filename);
waverror_t wav_write_file(const wavdata_t *wave, const char *filename);

#endif
