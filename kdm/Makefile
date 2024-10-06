CFLAGS = -Wall -Wextra
EMCC = emcc
SOURCES_ENC = $(wildcard src/*.c) $(wildcard src/ffgraph/*.c) $(wildcard src/kdm/*.c) $(wildcard src/kdm/**/*.c)
SOURCES_DEC = $(wildcard src_dec/*.c) $(wildcard src/ffgraph/*.c) $(wildcard src/kdm/*.c) $(wildcard src/kdm/**/*.c) $(wildcard src_dec/pl_mpeg/*.c)
HEADERS = $(wildcard src/*.h)
LIBS = lib/avcodec-61.dll lib/avformat-61.dll lib/avutil-59.dll lib/swresample-5.dll lib/swscale-8.dll

# Important: do not use "-03" with "build-release-windows" does not work

build-debug-windows:
	$(CC) $(SOURCES_ENC) $(CFLAGS) -Iinclude -O0 -g3 -DDEBUG -std=c11 $(LIBS) -o output/kdm_enc.exe

build-release-windows:
	windres src/info.rc output/info_rc.o
	$(CC) $(SOURCES_ENC) output/info_rc.o $(CFLAGS) -Iinclude -O2 -std=c11 $(LIBS) -o output/kdm_enc.exe


build-decoder-debug-windows:
	$(CC) $(SOURCES_DEC) $(CFLAGS) -Iinclude -O0 -g3 -DDEBUG -std=c11 $(LIBS) -o output/kdm_dec.exe

build-decoder-release-windows:
	windres src_dec/info.rc output/info_rc.o
	$(CC) $(SOURCES_DEC) output/info_rc.o $(CFLAGS) -Iinclude -O2 -std=c11 $(LIBS) -o output/kdm_dec.exe
