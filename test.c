
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
