# miniwav
A quick & dirty C library for reading/writing WAV audio file.

```C
typedef struct wavdata {
    int samplerate;   // sampling rate
    size_t channels;  // channels
    size_t size;      // array size of data
    double *data;     // audio signals
    wavtype_t type;   // (TYPE_UINT8, TYPE_INT16, TYPE_INT24, TYPE_INT32, TYPE_FLOAT)
} wavdata_t;

waverror_t wav_read_file(wavdata_t *wave, const char *filename);
waverror_t wav_write_file(const wavdata_t *wave, const char *filename);
```

## to read a wavefile
```C
wavdata_t wave;
waverror_t result;

result = wav_read_file(&wave, "IZ-US.wav");
```

## to write a wavefile
```C
wavdata_t wave;
waverror_t result;

wave.samplerate = 44100;
wave.channels = 1;
wave.size = wave.samplerate * wave.channels * 2;
wave.data = (double *) malloc(wave.size * sizeof(double));
for (i = 0; i < wave.size; i++) {
    wave.data[i] = sin(i * M_PI / 180.0) * 0.5;
}
wave.type = TYPE_INT24;

result = wav_write_file(&wave, "sine.wav");
free(wave.data);
```
