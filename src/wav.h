#ifndef __WAV_H__
#define __WAV_H__

#include "defs.h"

// WAV file header structure
typedef struct {
    char chunkID[4];
    u32 chunkSize;
    char format[4];
    char subchunk1ID[4];
    u32 subchunk1Size;
    u16 audioFormat;
    u16 numChannels;
    u32 sampleRate;
    u32 byteRate;
    u16 blockAlign;
    u16 bitsPerSample;
    char subchunk2ID[4];
    u32 subchunk2Size;
} wav_header_t;

typedef struct {
    wav_header_t header;
    u8 *buffer;
} wav_t;

wav_t *wav_from_file(const char *);
void wav_destroy(wav_t **);

#endif
