CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2
TARGET = mempredictc
SOURCES = main.c memory.c predictor.c

all: $(TARGET)

$(TARGET): $(SOURCES) memory.h predictor.h
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

clean:
	$(RM) $(TARGET) $(TARGET).exe

.PHONY: all clean
