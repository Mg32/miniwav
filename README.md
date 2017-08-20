# miniwav
A quick & dirty C library for reading/writing WAV audio file.

# API
```C
typedef struct wavdata {
    unsigned samplerate;// sampling rate
    unsigned channels;  // channels
    size_t size;        // array size of data
    double * data;      // PCM audio
    wavtype_t type;     // file type (for writing)
} wavdata_t;

typedef enum wavtype {
    TYPE_UNKNOWN,
    TYPE_UINT8,     // unsigned int  8bit
    TYPE_INT16,     //   signed int 16bit
    TYPE_INT24,     //   signed int 24bit
    TYPE_INT32,     //   signed int 32bit
    TYPE_FLOAT      //        float 32bit
} wavtype_t;

waverror_t wav_create(wavdata_t *wave, unsigned samplerate, unsigned ch, size_t frames);
void wav_destroy(wavdata_t *wave);
waverror_t wav_read_file(wavdata_t *wave, const char *filename);
waverror_t wav_write_file(const wavdata_t *wave, const char *filename);
```

# Examples
## Read the PCM-data from .wav file
```C
wavdata_t wave;
waverror_t result;

result = wav_read_file(&wave, "IZ-US.wav");

// handle wave.data

wav_destroy(&wave);
```

## Write the PCM-data to .wav file
```C
wavdata_t wave;
waverror_t result;

// create new wavdata_t
result = wav_create(&wave, 44100, 1, 88200);

// generate sine wave
for (i = 0; i < wave.size; i++) {
    wave.data[i] = sin(i * M_PI / 180.0) * 0.5;
}

// write sine wave to a file
wave.type = TYPE_INT24;
result = wav_write_file(&wave, "sine.wav");

wav_destroy(&wave);
```

# License
zlib License.
