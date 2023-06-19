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

    // Check if the file format is valid
    ERR_ASSERT_CLN(header.chunkID[0] == 'R' && header.chunkID[1] == 'I' && header.chunkID[2] == 'F' && header.chunkID[3] == 'F' && header.format[0] == 'W' && header.format[1] == 'A' && header.format[2] == 'V' && header.format[3] == 'E', 
            fclose(file), "Invalid WAV file format");

    file_read(&(wav->data), sizeof(wav->data) - sizeof(u8 *), file);
    while(!(wav->data.ID[0] == 'd' && wav->data.ID[1] == 'a' && wav->data.ID[2] == 't'&& wav->data.ID[3] == 'a')) {
        fseek(file, wav->data.size, SEEK_CUR);
        file_read(&(wav->data), sizeof(wav->data) - sizeof(u8 *), file);
    }

    // Allocate memory to store audio data
    u32 dataSize = wav->data.size;

    wav->header = header;
    u8* buffer = (uint8_t*)malloc(sizeof(u8) * dataSize);

    ERR_ASSERT_CLN(buffer != NULL, fclose(file), "Failed to allocate memory");
    wav->data.buffer = buffer;

    // Read the audio data into the buffer
    file_read(buffer, dataSize, file);
    fclose(file);

    return wav;
}

void wav_destroy(wav_t **wav) {
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
