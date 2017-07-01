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
#include "miniwav.h"

void test()
{
    wavdata_t wave;
    waverror_t result;
    int i;
    char filename[120];

    result = wav_read_file(&wave, "input.wav");
    printf("read: %d\n", (int)result); fflush(stdout);
    printf("srate: %d, ch: %d, size: %d, data: %p, type: %d\n",
        wave.samplerate, (int)wave.channels, (int)wave.size, wave.data, (int)wave.type);

    //wave.samplerate = 44100;
    //wave.channels = 1;
    //wave.size = wave.samplerate * wave.channels * 2;
    //wave.data = (double *) malloc(wave.size * sizeof(double));
    //for (i = 0; i < wave.size; i++) {
    //    wave.data[i] = sin(i * M_PI / 180.0) * 0.5;
    //}

    for (i = (int)TYPE_UINT8; i <= (int)TYPE_FLOAT; i++) {
        wave.type = (wavtype_t) i;
        sprintf(filename, "test/test_%d.wav", i);

        result = wav_write_file(&wave, filename);
        printf("%s: %d\n", filename, (int)result); fflush(stdout);
    }

    free(wave.data);
}

int main(int argc, char **argv)
{
    printf("hello\n"); fflush(stdout);

    test();

    return 0;
}
