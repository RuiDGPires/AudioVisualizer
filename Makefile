C_FILES := $(wildcard src/*.c)
H_FILES := $(wildcard src/*.h) $(wildcard src/include/*.h)
TARGET := vis
CC := gcc

$(TARGET): $(C_FILES) $(H_FILES)
	gcc $(C_FILES) -o $@

.PHONY: clean
clean:
	rm $(TARGET)
