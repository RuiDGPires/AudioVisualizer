#include <stdlib.h>

#include "wav.h"
#include "err.h"

#define BUFFER_SIZE 4096

// Function to read audio file and retrieve its contents
wav_t *wav_from_file(const char* filename) {
    FILE* file = fopen(filename, "rb");
    ERR_ASSERT(file != NULL, "Failed to open file");

    wav_t *wav = (wav_t *) malloc(sizeof(wav_t));
    wav_header_t header;

    // Read the WAV file header
    size_t headerSize = fread(&header, 1, sizeof(wav_header_t), file);

    ERR_ASSERT_CLN(headerSize == sizeof(wav_header_t), fclose(file), "Invalid WAV File");

    // Check if the file format is valid
    ERR_ASSERT_CLN(header.chunkID[0] == 'R' && header.chunkID[1] == 'I' && header.chunkID[2] == 'F' && header.chunkID[3] == 'F' && header.format[0] == 'W' && header.format[1] == 'A' && header.format[2] == 'V' && header.format[3] == 'E', 
            fclose(file), "Invalid WAV file format");

    // Allocate memory to store audio data
    u32 dataSize = header.subchunk2Size;

    wav->header = header;
    u8* buffer = (uint8_t*)malloc(sizeof(u8) * dataSize);

    ERR_ASSERT_CLN(buffer != NULL, fclose(file), "Failed to allocate memory");
    wav->buffer = buffer;

    // Read the audio data into the buffer
    usize bytesRead = fread(buffer, 1, dataSize, file);
    fclose(file);

    ERR_ASSERT_CLN(bytesRead == dataSize, free(buffer), "Error occurred while reading the audio data")

    return wav;
}

void wav_destroy(wav_t **wav) {
    free((*wav)->buffer);
    free(*wav);
    *wav = NULL;
}
