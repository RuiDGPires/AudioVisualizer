C_FILES := $(wildcard src/*.c) $(wildcard kiss_fft/*.c)
H_FILES := $(wildcard src/*.h) $(wildcard src/include/*.h)
TARGET := vis
CC := gcc

$(TARGET): $(C_FILES) $(H_FILES)
	gcc -Wall $(C_FILES) -o $@ -lm -lpng -ljpeg

.PHONY: release
release: $(C_FILES) $(H_FILES)
	gcc -O3 $(C_FILES) -o $@ -lm -lpng -ljpeg

.PHONY: clean
clean:
	rm $(TARGET)
