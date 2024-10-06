CFLAGS = -Wall -Wextra
SOURCES = $(wildcard src/*.c) $(wildcard src/ffgraph/*.c)
SOURCES_DEC = $(wildcard src_dec/*.c) src/morton2d.c src/lzss.c
HEADERS = $(wildcard src/*.h)
LIBS = lib/avcodec-61.dll lib/avformat-61.dll lib/avutil-59.dll lib/swscale-8.dll

build-debug-windows:
	$(CC) $(SOURCES) $(CFLAGS) -Iinclude -O0 -g3 -std=c11 $(LIBS) -o output/kdt_enc.exe

build-release-windows:
	windres src/info.rc output/info_rc.o
	$(CC) $(SOURCES) output/info_rc.o $(CFLAGS) -Iinclude -O3 -std=c11 $(LIBS) -o output/kdt_enc.exe

build-decoder-debug-windows:
	$(CC) $(SOURCES_DEC) $(CFLAGS) -Iinclude -O0 -g3 -std=c11 $(LIBS) -o output/kdt_dec.exe

build-decoder-release-windows:
	windres src/info.rc output/info_rc.o
	$(CC) $(SOURCES_DEC) output/info_rc.o $(CFLAGS) -Iinclude -O3 -std=c11 $(LIBS) -o output/kdt_dec.exe
