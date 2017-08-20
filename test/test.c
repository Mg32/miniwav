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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "miniwav.h"

// error messages
static char *error_messages[256];
static void init_error_messages()
{
    error_messages[(int)ERROR_OK]             = "OK";
    error_messages[(int)ERROR_UNKNOWN]        = "Error: unknown";
    error_messages[(int)ERROR_MEMORY_ALLOC]   = "Error: memory allocation";
    error_messages[(int)ERROR_UNSUPPORTED]    = "Error: unsupported";
    error_messages[(int)ERROR_UNABLE_TO_OPEN] = "Error: unable to open";
    error_messages[(int)ERROR_NOT_WAVE]       = "Error: not wave file";
    error_messages[(int)ERROR_BROKEN]         = "Error: broken";
    error_messages[(int)ERROR_BAD_DATA]       = "Error: bad data";
    error_messages[(int)ERROR_WRITE_FAULT]    = "Error: write fault";
}

static void show_result(waverror_t result)
{
    printf("%s\n", error_messages[(int)result]);
    fflush(stdout);
}

// read
static wavdata_t readfile(const char *filename)
{
    wavdata_t wave;
    waverror_t result;
    assert(filename != NULL);

    result = wav_read_file(&wave, filename);

    printf("read:\t%s: ", filename); show_result(result);
    assert(result == ERROR_OK);

#if 0
    // show information
    printf(
        "srate: %d, ch: %d, size: %d, data: %p, type: %d\n",
        wave.samplerate, (int)wave.channels,
        (int)wave.size, wave.data, (int)wave.type
    );
#endif

    return wave;
}

// verify
static double avoidDenormal(double v)
{
    return (v > 1e-20 || v < -1e-20) ? v : 0;
}
static double calculateError(size_t size, const double *a, const double *b)
{
    size_t i;
    double e = 0, maxb = 0, b_mag;
    assert(size > 0);

    for (i = 0; i < size; i++) {
        e += fabs(a[i] - b[i]);

        // find a maximum value
        b_mag = fabs(b[i]);
        if (b_mag > maxb) maxb = b_mag;
    }

    return avoidDenormal(e / (double)size) / maxb;
}

static void verifyfile(const char *filename,
                       size_t size, const double *orig,
                       double threshold)
{
    wavdata_t wave;
    double e;
    assert(filename != NULL);

    wave = readfile(filename);

    // "wave.size" must be equal to "size"
    if (wave.size != size) {
        printf(
            "verify:\t%s: read size = %d, original size = %d\n",
            filename, (int)wave.size, (int)size
        );
        assert(wave.size == size);
    }

    // error
    e = calculateError(size, wave.data, orig);
    printf(
        "verify:\t%s: %s (diff = %1.12lf)\n",
        filename,
        (e < threshold) ? "OK" : "NG",
        e
    );
    fflush(stdout);
}

static void verifyfiles(const char *prefix,
                        size_t size, const double *orig,
                        double threshold)
{
    int i;
    char filename[120];
    assert(prefix != NULL);

    for (i = (int)TYPE_UINT8; i <= (int)TYPE_FLOAT; i++) {
        sprintf(filename, "%s_%d.wav", prefix, i);
        verifyfile(filename, size, orig, threshold);
    }
}

// write
static void writefile(wavdata_t w, const char *filename, wavtype_t type)
{
    wavdata_t wave = w;
    waverror_t result;
    assert(filename != NULL);

    wave.type = type;
    result = wav_write_file(&wave, filename);

    printf("write:\t%s: ", filename); show_result(result);
    assert(result == ERROR_OK);
}

static void writefiles(wavdata_t w, const char *prefix)
{
    int i;
    char filename[120];
    assert(prefix != NULL);

    for (i = (int)TYPE_UINT8; i <= (int)TYPE_FLOAT; i++) {
        sprintf(filename, "%s_%d.wav", prefix, i);
        writefile(w, filename, (wavtype_t)i);
    }
}

// tests
#define TEST_START      printf("test %s: start\n", __func__)
#define TEST_DONE       printf("test %s: done\n\n", __func__)
#define ERROR_THRESHOLD 1

void test_write_sine()
{
    int i;
    wavdata_t wave;
    waverror_t result;
    TEST_START;

    result = wav_create(&wave, 44100/*Hz*/, 1/*ch*/, 44100 * 2/*frames*/);
    assert(result == ERROR_OK);
    for (i = 0; i < wave.size; i++) {
        wave.data[i] = sin(i * M_PI / 180.0) * 0.5;
    }
    writefiles(wave, "output/sine");
    verifyfiles("output/sine", wave.size, wave.data, ERROR_THRESHOLD);

    wav_destroy(&wave);
    TEST_DONE;
}

void test_read_write_verify()
{
    wavdata_t wave;
    TEST_START;

    wave = readfile("sa40.wav");
    writefiles(wave, "output/sa40");
    verifyfiles("output/sa40", wave.size, wave.data, ERROR_THRESHOLD);

    wav_destroy(&wave);
    TEST_DONE;
}

// entry
int main(int argc, char **argv)
{
    init_error_messages();
    test_write_sine();
    test_read_write_verify();

    return 0;
}
