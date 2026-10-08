CC = gcc
RC = windres
CFLAGS = -O2 -Wall -Wextra -Iinclude
LDFLAGS = -mwindows -lgdi32 -lmsimg32 -lwinmm

SRCS = src/main.c src/renderer.c src/dat_loader.c src/game_logic.c src/audio.c
OBJS = $(SRCS:.c=.o) src/resources.o
TARGET = chips_win10.exe

all: $(TARGET) play_original.exe

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

src/resources.o: src/resources.rc assets/chips.ico
	$(RC) $< -O coff -o $@

play_original.exe: src/launcher.c src/resources.o
	$(CC) -O2 -mwindows src/launcher.c src/resources.o -o $@

clean:
	rm -f src/*.o $(TARGET) play_original.exe setup_requirements.exe

.PHONY: all clean
