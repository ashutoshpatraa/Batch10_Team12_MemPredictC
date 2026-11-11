CC = gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2
MKDIR_P ?= if not exist build mkdir build
TARGET = mempredictc
CLI_TARGET = build/MemPredictC_CLI.exe
GUI_TARGET = build/MemPredictC.exe
CORE_SOURCES = memory.c predictor.c
CLI_SOURCES = cli_main.c main.c $(CORE_SOURCES)
GUI_SOURCES = gui_main.c main.c ui.c $(CORE_SOURCES)

all: $(CLI_TARGET) $(GUI_TARGET)

$(CLI_TARGET): $(CLI_SOURCES) memory.h predictor.h
	$(MKDIR_P) build
	$(CC) $(CFLAGS) $(CLI_SOURCES) -o $(CLI_TARGET)

$(GUI_TARGET): $(GUI_SOURCES) cli.h ui.h memory.h predictor.h
	$(MKDIR_P) build
	$(CC) $(CFLAGS) $(GUI_SOURCES) -o $(GUI_TARGET) -luser32 -lgdi32

clean:
	$(RM) $(TARGET) $(TARGET).exe $(CLI_TARGET) $(GUI_TARGET)

.PHONY: all clean
