CFLAGS = -Wall -Wextra -Wfloat-conversion -Wno-sign-compare -Wsign-conversion
SOURCES = $(wildcard src/*.c) $(wildcard src/stubs/*.c)
HEADERS = -Iinclude -Isrc -Isrc/stubs
LIBS_X64 = lib/libxml2-2.dll lib/libjansson.dll


build-x64-debug-windows:
	$(CC) $(SOURCES) $(CFLAGS) -DDEBUG $(HEADERS) -O0 -g3 -std=c11 $(LIBS_X64) -o output/bno.exe

build-x64-release-windows:
	windres src/info.rc output/info_rc.o
	$(CC) $(SOURCES) output/info_rc.o $(CFLAGS) $(HEADERS) -O3 -std=c11 $(LIBS_X64) -o output/bno.exe
