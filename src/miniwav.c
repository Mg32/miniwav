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

#include "miniwav.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

///////////////////////////////////////////////////////////////////////////////
// Misc.
///////////////////////////////////////////////////////////////////////////////

static const uint8_t RIFF_HEADER[4] = { 0x52, 0x49, 0x46, 0x46 };
static const uint8_t WAVE_HEADER[4] = { 0x57, 0x41, 0x56, 0x45 };
static const uint8_t FMT__HEADER[4] = { 0x66, 0x6D, 0x74, 0x20 };
static const uint8_t DATA_HEADER[4] = { 0x64, 0x61, 0x74, 0x61 };

static double *buffer_new(size_t length);
static void buffer_delete(double *buffer);
static int wavdata_is_bad_data(const wavdata_t *wave);
static int wavdata_is_unsupported(const wavdata_t *wave);
static wavtype_t wavtype_create(int fmtid, int bits);
static int wavtype_get_bits(wavtype_t type);

static double *buffer_new(size_t length)
{
    return (double *)malloc(length * sizeof(double));
}

static void buffer_delete(double *buffer)
{
    if (buffer != NULL) free(buffer);
}

static int wavdata_is_bad_data(const wavdata_t *wave)
{
    return (
        wave->size <= 0 ||
        wave->data == NULL ||
        wave->type == TYPE_UNKNOWN
    );
}

static int wavdata_is_unsupported(const wavdata_t *wave)
{
    return (
        wave->type == TYPE_UNKNOWN
    );
}

static wavtype_t wavtype_create(int fmtid, int bits)
{
    switch (fmtid) {
    case 0x01:
        switch (bits) {
        case  8: return TYPE_UINT8;
        case 16: return TYPE_INT16;
        case 24: return TYPE_INT24;
        case 32: return TYPE_INT32;
        }
        break;

    case 0x03:
        if (bits == 32) {
            return TYPE_FLOAT;
        }
        break;

    default:
        break;
    }

    return TYPE_UNKNOWN;
}

static int wavtype_get_bits(wavtype_t type)
{
    switch (type) {
    case TYPE_UINT8: return 8;
    case TYPE_INT16: return 16;
    case TYPE_INT24: return 24;
    case TYPE_INT32: return 32;
    case TYPE_FLOAT: return 32;
    default:         return -1;
    }
    return -1;
}

///////////////////////////////////////////////////////////////////////////////
// Create and Destroy
///////////////////////////////////////////////////////////////////////////////

waverror_t wav_create(wavdata_t *wave, unsigned samplerate, unsigned ch, size_t frames)
{
    wave->samplerate = samplerate;
    wave->channels = ch;
    wave->size = frames * ch;
    wave->data = buffer_new(wave->size);
    wave->type = TYPE_INT16;
    if (wavdata_is_bad_data(wave)) return ERROR_BAD_DATA;
    if (wavdata_is_unsupported(wave)) return ERROR_UNSUPPORTED;
    return ERROR_OK;
}

void wav_destroy(wavdata_t *wave)
{
    wave->samplerate = 0;
    wave->channels = 0;
    wave->size = 0;
    buffer_delete(wave->data); wave->data = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// Write
///////////////////////////////////////////////////////////////////////////////

static waverror_t write_header_riff(const wavdata_t *wave, FILE *fp);
static waverror_t write_header_wave(const wavdata_t *wave, FILE *fp);
static waverror_t write_chunk_fmt(int sr, int ch, int fmt, int bits, FILE *fp);
static waverror_t write_chunk_data(const double *data, size_t size, wavtype_t type, FILE *fp);
static double limit(double sig);
static int sig_to_uint8(double sig);
static int sig_to_int16(double sig);
static int sig_to_int24(double sig);
static int sig_to_int32(double sig);
static float sig_to_float(double sig);
static int write_int(FILE *fp, size_t bytes, int value);
static int write_float(FILE *fp, float value);

waverror_t wav_write_file(const wavdata_t *wave, const char *filename)
{
    waverror_t err = ERROR_UNKNOWN;
    FILE *fp = NULL;

    assert(wave != NULL);
    assert(filename != NULL);

    if (wavdata_is_bad_data(wave)) {
        err = ERROR_BAD_DATA;
        goto l_error;
    }

    if ((fp = fopen(filename, "wb")) == NULL) {
        err = ERROR_UNABLE_TO_OPEN;
        goto l_error;
    }

    err = write_header_riff(wave, fp);
	if (err != ERROR_OK) { goto l_error; }

    err = write_header_wave(wave, fp);
    if (err != ERROR_OK) { goto l_error; }

    err = ERROR_OK;

l_error:
    if (fp != NULL) { fclose(fp); }
	return err;
}

static waverror_t write_header_riff(const wavdata_t *wave, FILE *fp)
{
    assert(wave != NULL);
    assert(wave->size > 0);
    assert(wave->type != TYPE_UNKNOWN);
    assert(fp != NULL);
    if (ferror(fp)) { return ERROR_WRITE_FAULT; }

	fwrite((char *) RIFF_HEADER, sizeof(RIFF_HEADER), 1, fp);
	write_int(fp, 4, 36 + wave->size * wavtype_get_bits(wave->type) / 8);

    return ERROR_OK;
}

static waverror_t write_header_wave(const wavdata_t *wave, FILE *fp)
{
    int bits, fmt;
    waverror_t err;
    assert(wave != NULL);
    assert(wave->type != TYPE_UNKNOWN);
    assert(fp != NULL);
    if (ferror(fp)) { return ERROR_WRITE_FAULT; }

    bits = wavtype_get_bits(wave->type);
    fmt = (wave->type == TYPE_FLOAT) ? (0x03) : (0x01);

    // "WAVE" header
    fwrite((char *) WAVE_HEADER, sizeof(WAVE_HEADER), 1, fp);

    // "fmt " chunk
    err = write_chunk_fmt(wave->samplerate, wave->channels, fmt, bits, fp);
    if (err != ERROR_OK) { return err; }

	// "data" chunk
    err = write_chunk_data(wave->data, wave->size, wave->type, fp);
    if (err != ERROR_OK) { return err; }

    return ERROR_OK;
}

static waverror_t write_chunk_fmt(int sr, int ch, int fmt, int bits, FILE *fp)
{
    assert(sr > 0);
    assert(ch > 0);
    assert(fmt != 0);
    assert(bits > 0 && (bits % 2) == 0);
    assert(fp != NULL);
    if (ferror(fp)) { return ERROR_WRITE_FAULT; }

    fwrite((char *) FMT__HEADER, sizeof(FMT__HEADER), 1, fp);
    write_int(fp, 4, 16);
    write_int(fp, 2, fmt);
    write_int(fp, 2, ch);
    write_int(fp, 4, sr);
    write_int(fp, 4, sr * ch * bits / 8);
    write_int(fp, 2, ch * bits / 8);
    write_int(fp, 2, bits);

    return ERROR_OK;
}

static waverror_t write_chunk_data(const double *data, size_t size, wavtype_t type, FILE *fp)
{
    int i;
    assert(data != NULL);
    assert(size > 0);
    assert(type != TYPE_UNKNOWN);
    assert(fp != NULL);
    if (ferror(fp)) { return 1; }

	fwrite((char *) DATA_HEADER, sizeof(DATA_HEADER), 1, fp);
	write_int(fp, 4, size * wavtype_get_bits(type) / 8);

    switch (type) {
    case TYPE_UINT8:
		for (i = 0; i < size; i++) { write_int(fp, 1, sig_to_uint8(data[i])); }
        break;
    case TYPE_INT16:
		for (i = 0; i < size; i++) { write_int(fp, 2, sig_to_int16(data[i])); }
        break;
    case TYPE_INT24:
		for (i = 0; i < size; i++) { write_int(fp, 3, sig_to_int24(data[i])); }
        break;
    case TYPE_INT32:
		for (i = 0; i < size; i++) { write_int(fp, 4, sig_to_int32(data[i])); }
        break;
    case TYPE_FLOAT:
        for (i = 0; i < size; i++) { write_float(fp, sig_to_float(data[i])); }
        break;
    default:
        assert("Unknown or unsupported format");
        return ERROR_BAD_DATA;
    }

    return ERROR_OK;
}

static double limit(double sig)
{
    if (sig > +1.0) { return +1.0; }
    if (sig < -1.0) { return -1.0; }
    return sig;
}

static int sig_to_uint8(double sig) { return (int)(limit(sig) * 127) + 128; }
static int sig_to_int16(double sig) { return (int)(limit(sig) * 32767); }
static int sig_to_int24(double sig) { return (int)(limit(sig) * 8388607); }
static int sig_to_int32(double sig) { return (int)(limit(sig) * 2147483647); }
static float sig_to_float(double sig) { return (float) sig; }

static int write_int(FILE *fp, size_t bytes, int value)
{
    while (bytes--) {
        fputc(value & 0xff, fp);
        value >>= 8;
    }
    return 1;
}

static int write_float(FILE *fp, float value)
{
    return fwrite(&value, sizeof(value), 1, fp) == sizeof(value);
}

///////////////////////////////////////////////////////////////////////////////
// Read
///////////////////////////////////////////////////////////////////////////////

static waverror_t read_header_riff(size_t *filesize, FILE *fp);
static waverror_t read_header_wave(wavdata_t *wave, size_t filesize, FILE *fp);
static waverror_t read_chunk_body(const char *sig, size_t size, wavdata_t *wave, FILE *fp);
static waverror_t read_chunk_fmt(wavdata_t *wave, FILE *fp);
static waverror_t read_chunk_data(wavdata_t *wave, FILE *fp);
static double uint8_to_sig(uint8_t data);
static double int16_to_sig(int16_t data);
static double int24_to_sig(int32_t data);
static double int32_to_sig(int32_t data);
static double float_to_sig(float data);
static int read_int(FILE *fp, size_t bytes);
static float read_float(FILE *fp);

waverror_t wav_read_file(wavdata_t *wave, const char *filename)
{
	waverror_t err = ERROR_BROKEN;
    FILE *fp = NULL;
    size_t filesize;

    assert(wave != NULL);
    assert(filename != NULL);

    wave->samplerate = 0;
    wave->channels = 0;
    wave->size = 0;
    wave->data = NULL;
    wave->type = TYPE_UNKNOWN;

    fp = fopen(filename, "rb");
    if (fp == NULL) {
        err = ERROR_UNABLE_TO_OPEN;
        goto l_error;
    }
    fseek(fp, 0, SEEK_SET);

	// RIFF header
    err = read_header_riff(&filesize, fp);
    if (err != ERROR_OK) { goto l_error; }

	// WAVE header
    err = read_header_wave(wave, filesize, fp);
    if (err != ERROR_OK) { goto l_error; }

	err = ERROR_OK;

l_error:
    if (fp != NULL) { fclose(fp); }
	return err;
}

static waverror_t read_header_riff(size_t *filesize, FILE *fp)
{
    char sig[4];
    assert(filesize != NULL);
    assert(fp != NULL);
    if (feof(fp) || ferror(fp)) { return ERROR_BROKEN; }

	if (fread(sig, 1, sizeof(sig), fp) != sizeof(sig) ||
        memcmp(sig, RIFF_HEADER, sizeof(RIFF_HEADER)) != 0) {
        return ERROR_NOT_WAVE;
	}
	*filesize = read_int(fp, 4) + 8;

    return ERROR_OK;
}

static waverror_t read_header_wave(wavdata_t *wave, size_t filesize, FILE *fp)
{
    char sig[4];

    assert(wave != NULL);
    assert(fp != NULL);
    if (feof(fp) || ferror(fp)) { return ERROR_BROKEN; }

	if (fread(sig, 1, sizeof(sig), fp) != sizeof(sig) ||
    	memcmp(sig, WAVE_HEADER, sizeof(WAVE_HEADER)) != 0) {
        return ERROR_NOT_WAVE;
	}

	// read chunks
	while (!feof(fp) && !ferror(fp) && ftell(fp) < filesize) {
		// read the chunk header
		if (fread(sig, 1, sizeof(sig), fp) != sizeof(sig)) { break; }
		size_t size = read_int(fp, 4);
		off_t pos = ftell(fp);

		// read the chunk body
        waverror_t err = read_chunk_body(sig, size, wave, fp);
        if (err != ERROR_OK) { return err; }

		// move to the next chunk
		fseek(fp, pos + size, SEEK_SET);
	}

    return ERROR_OK;
}

static waverror_t read_chunk_body(const char *sig, size_t size, wavdata_t *wave, FILE *fp)
{
    assert(sig != NULL);
    assert(wave != NULL);
    assert(fp != NULL);
    if (feof(fp) || ferror(fp)) { return ERROR_BROKEN; }

    if (memcmp(sig, FMT__HEADER, sizeof(FMT__HEADER)) == 0) {
        // "fmt " chunk
        waverror_t err = read_chunk_fmt(wave, fp);
        if (err != ERROR_OK) { return err; }
    }
    else if (memcmp(sig, DATA_HEADER, sizeof(DATA_HEADER)) == 0) {
        // "data" chunk
        wave->size = size * CHAR_BIT / wavtype_get_bits(wave->type);
        if (wave->size <= 0) {
            wave->data = NULL;
            return ERROR_OK;
        }

        wave->data = buffer_new(wave->size);
        if (wave->data == NULL) {
            return ERROR_MEMORY_ALLOC;
        }

        waverror_t err = read_chunk_data(wave, fp);
        if (err != ERROR_OK) { return err; }
    }

    return ERROR_OK;
}

static waverror_t read_chunk_fmt(wavdata_t *wave, FILE *fp)
{
    int fmt, bits;
    assert(wave != NULL);
    assert(fp != NULL);
    if (feof(fp) || ferror(fp)) { return ERROR_BROKEN; }

    fmt = read_int(fp, 2);                  // Format
    wave->channels = read_int(fp, 2);       // Channels
    wave->samplerate = read_int(fp, 4);     // SampleRate
    read_int(fp, 4);                        // BytePerSec
    read_int(fp, 2);                        // BlockAlign
    bits = read_int(fp, 2);                 // BitPerSample

    wave->type = wavtype_create(fmt, bits);
    if (wave->type == TYPE_UNKNOWN) {
        return ERROR_UNSUPPORTED;
    }

    return ERROR_OK;
}

static waverror_t read_chunk_data(wavdata_t *wave, FILE *fp)
{
    int i;
    assert(wave != NULL);
    assert(wave->size > 0);
    assert(wave->data != NULL);
    assert(fp != NULL);
    if (feof(fp) || ferror(fp)) { return ERROR_BROKEN; }

    switch (wave->type) {
    case TYPE_UINT8:
        for (i = 0; i < wave->size; i++) {
            wave->data[i] = uint8_to_sig(read_int(fp, 1));
        }
        break;
    case TYPE_INT16:
        for (i = 0; i < wave->size; i++) {
            wave->data[i] = int16_to_sig(read_int(fp, 2));
        }
        break;
    case TYPE_INT24:
        for (i = 0; i < wave->size; i++) {
            wave->data[i] = int24_to_sig(read_int(fp, 3));
        }
        break;
    case TYPE_INT32:
        for (i = 0; i < wave->size; i++) {
            wave->data[i] = int32_to_sig(read_int(fp, 4));
        }
        break;
    case TYPE_FLOAT:
        for (i = 0; i < wave->size; i++) {
            wave->data[i] = float_to_sig(read_float(fp));
        }
        break;
    default:
        return ERROR_UNSUPPORTED;
    }

    return ERROR_OK;
}

static double uint8_to_sig(uint8_t data) { return (data - 128.0) / 127.0; }
static double int16_to_sig(int16_t data) { return data / 32767.0; }
static double int24_to_sig(int32_t data) {
    data = (data & 0x7fffff) - (data & 0x800000); return data / 8388607.0;
}
static double int32_to_sig(int32_t data) { return data / 2147483647.0; }
static double float_to_sig(float data) { return (double)data; }

static int read_int(FILE *fp, size_t bytes)
{
    int i, value = 0;
    for (i = 0; i < bytes; i++) {
        value |= (fgetc(fp) & 0xff) << (i * 8);
    }
    return value;
}

static float read_float(FILE *fp)
{
    float data;
    return fread(&data, sizeof(data), 1, fp) == sizeof(data);
}
