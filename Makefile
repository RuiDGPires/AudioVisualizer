C_FILES := $(wildcard src/*.c) $(wildcard kiss_fft/*.c)
H_FILES := $(wildcard src/*.h) $(wildcard src/include/*.h)
TARGET := vis
CC := gcc

LIBFLAGS := -lm -lpng -ljpeg

$(TARGET): $(C_FILES) $(H_FILES)
	gcc -Wall $(C_FILES) -o $@ $(LIBFLAGS)

.PHONY: release
release: $(C_FILES) $(H_FILES)
	gcc -O3 $(C_FILES) -o $@ $(LIBFLAGS)

.PHONY: clean
clean:
	rm $(TARGET)
