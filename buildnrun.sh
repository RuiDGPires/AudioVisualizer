#!/bin/sh

VIDEO_FILE=output.mp4

make
./vis ${VIDEO_FILE}
vlc ${VIDEO_FILE} >/dev/null 2>&1
