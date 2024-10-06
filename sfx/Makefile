CFLAGS = -Wall -Wextra -Wfloat-conversion -Wno-sign-compare
SOURCES_ENC = $(wildcard src/*.c) $(wildcard src/ffgraph/*.c) $(wildcard src/mvqa/**/*.c) $(wildcard src/sndbridge/*.c)
HEADERS = $(wildcard src/*.h)
LIBS = lib/avcodec-61.dll lib/avformat-61.dll lib/avutil-59.dll lib/swresample-5.dll

build-debug-windows:
	$(CC) $(SOURCES_ENC) $(CFLAGS) -DDEBUG -Iinclude -O0 -g3 -std=c11 $(LIBS) -o output/sfx.exe

build-release-windows:
	windres src/info.rc output/info_rc.o
	$(CC) $(SOURCES_ENC) output/info_rc.o $(CFLAGS) -Iinclude -O3 -std=c11 $(LIBS) -o output/sfx.exe
