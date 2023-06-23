#ifndef __FFMPEG_H__
#define __FFMPEG_H__

#include "config.h"
#include "defs.h"

int open_ffmpeg(const char *, const char *, u32 width, u32 height, u32 fps);

#endif
