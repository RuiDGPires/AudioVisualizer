#include <stdlib.h>
#include "wav.h"
#include "err.h"
#include "util.h"

#define BUFFER_SIZE 4096

// Function to read audio file and retrieve its contents
wav_t *wav_from_file(const char* filename) {
    FILE* file = fopen(filename, "rb");
    ERR_ASSERT(file != NULL, "Failed to open file");

    wav_t *wav = (wav_t *) malloc(sizeof(wav_t));
    wav_header_t header;

    // Read the WAV file header
    file_read(&header, sizeof(wav_header_t), file);
    wav->header = header;

    // Check if the file format is valid
    ERR_ASSERT_CLN(header.chunkID[0] == 'R' && header.chunkID[1] == 'I' && header.chunkID[2] == 'F' && header.chunkID[3] == 'F' && header.format[0] == 'W' && header.format[1] == 'A' && header.format[2] == 'V' && header.format[3] == 'E', 
            fclose(file), "Invalid WAV file format");

    fseek(file, header.subchunk1Size + 20, SEEK_SET);
    file_read(&(wav->data), sizeof(wav->data) - sizeof(u8 *), file);

    while(!(wav->data.ID[0] == 'd' && wav->data.ID[1] == 'a' && wav->data.ID[2] == 't'&& wav->data.ID[3] == 'a')) {
        fseek(file, wav->data.size, SEEK_CUR);
        file_read(&(wav->data), sizeof(wav->data) - sizeof(u8 *), file);
    }

    // Allocate memory to store audio data
    u32 dataSize = wav->data.size;

    u8* buffer = (uint8_t*)malloc(sizeof(u8) * dataSize);

    ERR_ASSERT_CLN(buffer != NULL, fclose(file), "Failed to allocate memory");
    wav->data.buffer = buffer;

    // Read the audio data into the buffer
    file_read(buffer, dataSize, file);
    fclose(file);

    return wav;
}

void wav_destroy(wav_t **wav) {
    if (*wav == NULL) return;

    free((*wav)->data.buffer);
    free(*wav);
    *wav = NULL;
}

i64 wav_get_val64(wav_t *wav, usize n) {
    u8 bytes_per_sample = wav->header.bitsPerSample / 8;
    u64 ret = 0;

    for (u8 i = 0; i < bytes_per_sample; i++) 
        ret = ret | (((u64)wav->data.buffer[bytes_per_sample*n + i]) << 8 * i);

    return ret;
}

i32 wav_get_val32(wav_t *wav, usize n) {
    u8 bytes_per_sample = wav->header.bitsPerSample / 8;
    u32 ret = 0;

    for (u8 i = 0; i < bytes_per_sample; i++) 
        ret = ret | (((i32) wav->data.buffer[bytes_per_sample*n + i]) << 8 * i);

    if (wav->header.bitsPerSample == 24) {
        if ((ret & 0x800000 ) != 0)
            ret |= 0xFF000000;
    }

    return ret;
}


i32 wav_get_val32_channel(wav_t *wav, usize n, u8 channel) {
    return wav_get_val32(wav, n*(wav->header.numChannels) + channel);
}

void wav_set_val(wav_t *wav, usize n, u32 val) {
    u8 bytes_per_sample = wav->header.bitsPerSample / 8;
    for (u8 i = 0; i < bytes_per_sample; i++) {
        u32 mask = 0xFF << i * 8;
        wav->data.buffer[n*bytes_per_sample + i] = (val & mask) >> i * 8;
    }
}

void wav_to_mono_left(wav_t *wav) {
    u8 bytes_per_sample = wav->header.bitsPerSample / 8;
    for (usize i = 0; i < wav->data.size / (wav->header.numChannels * bytes_per_sample); i++)  {
        wav_set_val(wav, i, wav_get_val32_channel(wav, i, 0));
    }

    wav->data.size = wav->data.size / wav->header.numChannels;
    wav->header.numChannels = 1;
    wav->data.buffer = realloc(wav->data.buffer, sizeof(u8) * wav->data.size);
}

void wav_to_mono(wav_t *wav) {
    // (left + right) / 2
    u8 bytes_per_sample = wav->header.bitsPerSample / 8;
    for (usize i = 0; i < wav->data.size / (wav->header.numChannels * bytes_per_sample); i++)  {
        i32 sum = 0;
        for (u8 j = 0; j < wav->header.numChannels; j++) {
            sum += wav_get_val32(wav, i * wav->header.numChannels + j);
        }
        wav_set_val(wav, i, sum / wav->header.numChannels);
    }

    wav->data.size = wav->data.size / wav->header.numChannels;
    wav->header.numChannels = 1;
    wav->data.buffer = realloc(wav->data.buffer, sizeof(u8) * wav->data.size);
}

i32 *wav_to_32(wav_t *wav) {
    usize n_samples = wav_n_samples(wav);
    u32 new_size = sizeof(i32) * n_samples;
    wav->data.buffer = realloc(wav->data.buffer, new_size);

    
    for (usize i = 0; i < n_samples; i++) {
        i32 val = wav_get_val32(wav, n_samples - i - 1);
        i32 *tmp = (i32 *) wav->data.buffer;
        tmp[n_samples - i - 1] = val;
    }

    wav->data.size = new_size;
    wav->header.bitsPerSample = 32; 
    wav->header.blockAlign = 8;

    return (i32 *) wav->data.buffer;
}

void wav_normalize(wav_t *wav, usize new_max) {
    usize n_samples = wav_n_samples(wav);

    i32 max = 0;

    for (usize i = 0; i < n_samples; i++) {
        i32 val = wav_get_val32(wav, i);
        if (abs(val) > max) 
            max = val;
    }

    for (usize i = 0; i < n_samples; i++) {
        i32 val = wav_get_val32(wav, i);
        i32 s = sign(val);
        wav_set_val(wav, i, s * map(abs(val), 0, max, 0, new_max));
    }
}

usize wav_n_samples(wav_t *wav) {
    u8 bytes_per_sample = wav->header.bitsPerSample / 8;
    return wav->data.size / bytes_per_sample;
}

double wav_duration(wav_t *wav) {
    return (double) wav_n_samples(wav) / wav->header.sampleRate;
}
