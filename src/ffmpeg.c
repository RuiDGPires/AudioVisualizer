#include <stdlib.h>
#include <unistd.h>
#include "ffmpeg.h"
#include "err.h"

#define PIPE_READ  0
#define PIPE_WRITE 1

#define MAX_CHARS_DIM 100
#define MAX_CHARS_FPS 4

int open_ffmpeg(const char *filename, const char *audio, u32 width, u32 height, u32 fps) {
    int pipefd[2];
    ERR_ASSERT(pipe(pipefd) >= 0, "Unable to open pipe");
    
    int child = fork();

    char dims_s[MAX_CHARS_DIM];
    char fps_s[MAX_CHARS_FPS];
    snprintf(dims_s, MAX_CHARS_DIM, "%ux%u", width, height);
    snprintf(fps_s, MAX_CHARS_FPS, "%u", fps);

    ERR_ASSERT(child >= 0, "Unable to fork");

    if (child == 0) {
        ERR_ASSERT(dup2(pipefd[PIPE_READ], STDIN_FILENO) >= 0, "Unable to reopen read pipe as stdin");
        close(pipefd[PIPE_WRITE]);

        ERR_ASSERT(execlp("ffmpeg",
            "ffmpeg",
            "-loglevel", "verbose",
            "-y",
            "-f", "rawvideo",
            "-pix_fmt", "rgba",
            "-s", dims_s, 
            "-r", fps_s,
            "-an",
            "-i", "-",
            "-c:v", "libx264",

            filename,
#ifdef AUDIO
            "-i", audio,
#endif
            // ...
            NULL
        ) >= 0, "Unable to open FFMPEG: %s", strerror(errno));
        assert(0 && "Unreachable");
    }

    close(pipefd[PIPE_READ]);
    return pipefd[PIPE_WRITE];     
}

