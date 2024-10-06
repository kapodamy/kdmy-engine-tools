/*
PL_MPEG - MPEG1 Video decoder, MPEG-PS demuxer

Dominic Szablewski - https://phoboslab.org


-- LICENSE: The MIT License(MIT)

Copyright(c) 2019 Dominic Szablewski

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files(the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and / or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions :
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.




-- Synopsis

// Define `PL_MPEG_IMPLEMENTATION` in *one* C/C++ file before including this
// library to create the implementation.

#define PL_MPEG_IMPLEMENTATION
#include "plmpeg.h"

// This function gets called for each decoded video frame
void my_video_callback(plm_t *plm, plm_frame_t *frame, void *user) {
	// Do something with frame->y.data, frame->cr.data, frame->cb.data
}

// Load a .mpg (MPEG Program Stream) file
plm_t *plm = plm_create_with_filename("some-file.mpg");

// Install the video decode callback
plm_set_video_decode_callback(plm, my_video_callback, my_data);


// Decode
do {
	plm_decode(plm, time_since_last_call);
} while (!plm_has_ended(plm));

// All done
plm_destroy(plm);



-- Documentation

This library provides several interfaces to load, demux and decode MPEG video.
A high-level API combines the demuxer & video decoder
in an easy to use wrapper.

Lower-level APIs for accessing the demuxer, video decoder, 
as well as providing different data sources are also available.

Interfaces are written in an object oriented style, meaning you create object 
instances via various different constructor functions (plm_*create()),
do some work on them and later dispose them via plm_*destroy().

plm_* ......... the high-level interface, combining demuxer and decoders
plm_buffer_* .. the data source used by all interfaces
plm_demux_* ... the MPEG-PS demuxer
plm_video_* ... the MPEG1 Video ("mpeg1") decoder


With the high-level interface you have two options to decode video:

 1. Use plm_decode() and just hand over the delta time since the last call.
    It will decode everything needed and call your callbacks (specified through
    plm_set_video_decode_callback()) any number of times.

 2. Use plm_decode_video() to decode exactly one
    frame of video at a time. How you handle the synchronization 
    of both streams is up to you.

Video data is decoded into a struct with all 3 planes (Y, Cr, Cb) stored in
separate buffers. You can either convert this to RGB on the CPU (slow) via the
plm_frame_to_rgb() function or do it on the GPU with the following matrix:

mat4 bt601 = mat4(
	1.16438,  0.00000,  1.59603, -0.87079,
	1.16438, -0.39176, -0.81297,  0.52959,
	1.16438,  2.01723,  0.00000, -1.08139,
	0, 0, 0, 1
);
gl_FragColor = vec4(y, cb, cr, 1.0) * bt601;


Data can be supplied to the high level interface, the demuxer and the decoders
in three different ways:

 1. Using plm_create_from_filename() or with a file handle with 
    plm_create_from_file().

 2. Using plm_create_with_memory() and supplying a pointer to memory that
    contains the whole file.

 3. Using plm_create_with_buffer(), supplying your own plm_buffer_t instance and
    periodically writing to this buffer.

When using your own plm_buffer_t instance, you can fill this buffer using 
plm_buffer_write(). You can either monitor plm_buffer_get_remaining() and push 
data when appropriate, or install a callback on the buffer with 
plm_buffer_set_load_callback() that gets called whenever the buffer needs more 
data.

A buffer created with plm_buffer_create_with_capacity() is treated as a ring
buffer, meaning that data that has already been read, will be discarded. In
contrast, a buffer created with plm_buffer_create_for_appending() will keep all
data written to it in memory. This enables seeking in the already loaded data.


There should be no need to use the lower level plm_demux_* and plm_video_* functions, 
if all you want to do is read/decode an MPEG-PS file.
However, if you get raw mpeg1video data from a different
source, these functions can be used to decode the raw data directly. Similarly, 
if you only want to analyze an MPEG-PS file or extract raw video
packets from it, you can use the plm_demux_* functions.


This library uses malloc(), realloc() and free() to manage memory. Typically 
all allocation happens up-front when creating the interface. However, the
default buffer size may be too small for certain inputs. In these cases plmpeg
will realloc() the buffer with a larger size whenever needed. You can configure
the default buffer size by defining PLM_BUFFER_DEFAULT_SIZE *before* 
including this library.

You can also define PLM_MALLOC, PLM_REALLOC and PLM_FREE to provide your own
memory management functions.


See below for detailed the API documentation.

*/


#ifndef PL_MPEG_H
#define PL_MPEG_H

#include <stdint.h>
#include <stdio.h>


// -----------------------------------------------------------------------------
// Public Data Types


// Object types for the various interfaces

typedef struct plm_t plm_t;
typedef struct plm_buffer_t plm_buffer_t;
typedef struct plm_demux_t plm_demux_t;
typedef struct plm_video_t plm_video_t;


// Demuxed MPEG PS packet
// The type maps directly to the various MPEG-PES start codes. PTS is the
// presentation time stamp of the packet in seconds. Note that not all packets
// have a PTS value, indicated by PLM_PACKET_INVALID_TS.

#define PLM_PACKET_INVALID_TS -1

typedef struct {
	int type;
	double pts;
	size_t length;
	uint8_t *data;
} plm_packet_t;


// Decoded Video Plane 
// The byte length of the data is width * height. Note that different planes
// have different sizes: the Luma plane (Y) is double the size of each of 
// the two Chroma planes (Cr, Cb) - i.e. 4 times the byte length.
// Also note that the size of the plane does *not* denote the size of the 
// displayed frame. The sizes of planes are always rounded up to the nearest
// macroblock (16px).

typedef struct {
	unsigned int width;
	unsigned int height;
	uint8_t *data;
} plm_plane_t;


// Decoded Video Frame
// width and height denote the desired display size of the frame. This may be
// different from the internal size of the 3 planes.

typedef struct {
	double time;
	unsigned int width;
	unsigned int height;
	plm_plane_t y;
	plm_plane_t cr;
	plm_plane_t cb;
} plm_frame_t;


// Callback function type for decoded video frames used by the high-level
// plm_* interface

typedef void(*plm_video_decode_callback)
	(plm_t *self, plm_frame_t *frame, void *user);


// Callback function for plm_buffer when it needs more data

typedef void(*plm_buffer_load_callback)(plm_buffer_t *self, void *user);



// -----------------------------------------------------------------------------
// plm_* public API
// High-Level API for loading/demuxing/decoding MPEG-PS data


// Create a plmpeg instance with a filename. Returns NULL if the file could not
// be opened.

plm_t *plm_create_with_filename(const char *filename);


// Create a plmpeg instance with a file handle. Pass TRUE to close_when_done to
// let plmpeg call fclose() on the handle when plm_destroy() is called.

plm_t *plm_create_with_file(FILE *fh, int close_when_done);


// Create a plmpeg instance with a pointer to memory as source. This assumes the
// whole file is in memory. The memory is not copied. Pass TRUE to 
// free_when_done to let plmpeg call free() on the pointer when plm_destroy() 
// is called.

plm_t *plm_create_with_memory(uint8_t *bytes, size_t length, int free_when_done);


// Create a plmpeg instance with a plm_buffer as source. Pass TRUE to
// destroy_when_done to let plmpeg call plm_buffer_destroy() on the buffer when
// plm_destroy() is called.

plm_t *plm_create_with_buffer(plm_buffer_t *buffer, int destroy_when_done);


// Destroy a plmpeg instance and free all data.

void plm_destroy(plm_t *self);


// Get whether we have headers on all available streams and we can report the 
// number of video streams, video dimensions, framerate.
// This returns FALSE if the file is not an MPEG-PS file or - when not using a
// file as source - when not enough data is available yet.

int plm_has_headers(plm_t *self);


// Probe the MPEG-PS data to find the actual number of video streams
// within the buffer. For certain files (e.g. VideoCD) this can be more accurate
// than just reading the number of streams from the headers.
// This should only be used when the underlying plm_buffer is seekable, i.e. for 
// files, fixed memory buffers or _for_appending buffers. If used with dynamic
// memory buffers it will skip decoding the probesize!
// The necessary probesize is dependent on the files you expect to read. Usually
// a few hundred KB should be enough to find all streams.
// Use plm_get_num_video_streams() afterwards to get the number of 
// streams in the file.
// Returns TRUE if any streams were found within the probesize.

int plm_probe(plm_t *self, size_t probesize);


// Get the number of video streams (0--1) reported in the system header.

int plm_get_num_video_streams(plm_t *self);


// Get the display width/height of the video stream.

int plm_get_width(plm_t *self);
int plm_get_height(plm_t *self);


// Get the framerate of the video stream in frames per second.

double plm_get_framerate(plm_t *self);


// Get the current internal time in seconds.

double plm_get_time(plm_t *self);


// Get the video duration of the underlying source in seconds.

double plm_get_duration(plm_t *self);


// Rewind all buffers back to the beginning.

void plm_rewind(plm_t *self);


// Get or set looping. Default FALSE.

int plm_get_loop(plm_t *self);
void plm_set_loop(plm_t *self, int loop);


// Get whether the file has ended. If looping is enabled, this will always
// return FALSE.

int plm_has_ended(plm_t *self);


// Set the callback for decoded video frames used with plm_decode(). If no 
// callback is set, video data will be ignored and not be decoded. The *user
// Parameter will be passed to your callback.

void plm_set_video_decode_callback(plm_t *self, plm_video_decode_callback fp, void *user);


// Advance the internal timer by seconds and decode video up to this time.
// This will call the video_decode_callback any number
// of times. A frame-skip is not implemented, i.e. everything up to current time
// will be decoded.

void plm_decode(plm_t *self, double seconds);


// Decode and return one video frame. Returns NULL if no frame could be decoded
// (either because the source ended or data is corrupt).
// The returned plm_frame_t is valid until the next call to plm_decode_video() 
// or until plm_destroy() is called.

plm_frame_t *plm_decode_video(plm_t *self);


// Seek to the specified time, clamped between 0 -- duration. This can only be 
// used when the underlying plm_buffer is seekable, i.e. for files, fixed 
// memory buffers or _for_appending buffers. 
// If seek_exact is TRUE this will seek to the exact time, otherwise it will 
// seek to the last intra frame just before the desired time. Exact seeking can 
// be slow, because all frames up to the seeked one have to be decoded on top of
// the previous intra frame.
// If seeking succeeds, this function will call the video_decode_callback 
// exactly once with the target frame.
// Returns TRUE if seeking succeeded or FALSE if no frame could be found.

int plm_seek(plm_t *self, double time, int seek_exact);


// Similar to plm_seek(), but will not call the video_decode_callback.
// Returns the found frame or NULL if no frame could be found.

plm_frame_t *plm_seek_frame(plm_t *self, double time, int seek_exact);



// -----------------------------------------------------------------------------
// plm_buffer public API
// Provides the data source for all other plm_* interfaces


// The default size for buffers created from files or by the high-level API

#ifndef PLM_BUFFER_DEFAULT_SIZE
#define PLM_BUFFER_DEFAULT_SIZE (128 * 1024)
#endif


// Create a buffer instance with a filename. Returns NULL if the file could not
// be opened.

plm_buffer_t *plm_buffer_create_with_filename(const char *filename);


// Create a buffer instance with a file handle. Pass TRUE to close_when_done
// to let plmpeg call fclose() on the handle when plm_destroy() is called.

plm_buffer_t *plm_buffer_create_with_file(FILE *fh, int close_when_done);


// Create a buffer instance with a pointer to memory as source. This assumes
// the whole file is in memory. The bytes are not copied. Pass 1 to 
// free_when_done to let plmpeg call free() on the pointer when plm_destroy() 
// is called.

plm_buffer_t *plm_buffer_create_with_memory(uint8_t *bytes, size_t length, int free_when_done);


// Create an empty buffer with an initial capacity. The buffer will grow
// as needed. Data that has already been read, will be discarded.

plm_buffer_t *plm_buffer_create_with_capacity(size_t capacity);


// Create an empty buffer with an initial capacity. The buffer will grow
// as needed. Decoded data will *not* be discarded. This can be used when
// loading a file over the network, without needing to throttle the download. 
// It also allows for seeking in the already loaded data.

plm_buffer_t *plm_buffer_create_for_appending(size_t initial_capacity);


// Destroy a buffer instance and free all data

void plm_buffer_destroy(plm_buffer_t *self);


// Copy data into the buffer. If the data to be written is larger than the 
// available space, the buffer will realloc() with a larger capacity. 
// Returns the number of bytes written. This will always be the same as the
// passed in length, except when the buffer was created _with_memory() for
// which _write() is forbidden.

size_t plm_buffer_write(plm_buffer_t *self, uint8_t *bytes, size_t length);


// Mark the current byte length as the end of this buffer and signal that no 
// more data is expected to be written to it. This function should be called
// just after the last plm_buffer_write().
// For _with_capacity buffers, this is cleared on a plm_buffer_rewind().

void plm_buffer_signal_end(plm_buffer_t *self);


// Set a callback that is called whenever the buffer needs more data

void plm_buffer_set_load_callback(plm_buffer_t *self, plm_buffer_load_callback fp, void *user);


// Rewind the buffer back to the beginning. When loading from a file handle,
// this also seeks to the beginning of the file.

void plm_buffer_rewind(plm_buffer_t *self);


// Get the total size. For files, this returns the file size. For all other 
// types it returns the number of bytes currently in the buffer.

size_t plm_buffer_get_size(plm_buffer_t *self);


// Get the number of remaining (yet unread) bytes in the buffer. This can be
// useful to throttle writing.

size_t plm_buffer_get_remaining(plm_buffer_t *self);


// Get whether the read position of the buffer is at the end and no more data 
// is expected.

int plm_buffer_has_ended(plm_buffer_t *self);



// -----------------------------------------------------------------------------
// plm_demux public API
// Demux an MPEG Program Stream (PS) data into separate packages


// Various Packet Types

static const int PLM_DEMUX_PACKET_PRIVATE = 0xBD;
static const int PLM_DEMUX_PACKET_AUDIO_1 = 0xC0;
static const int PLM_DEMUX_PACKET_AUDIO_2 = 0xC1;
static const int PLM_DEMUX_PACKET_AUDIO_3 = 0xC2;
static const int PLM_DEMUX_PACKET_AUDIO_4 = 0xC2;
static const int PLM_DEMUX_PACKET_VIDEO_1 = 0xE0;


// Create a demuxer with a plm_buffer as source. This will also attempt to read
// the pack and system headers from the buffer.

plm_demux_t *plm_demux_create(plm_buffer_t *buffer, int destroy_when_done);


// Destroy a demuxer and free all data.

void plm_demux_destroy(plm_demux_t *self);


// Returns TRUE/FALSE whether pack and system headers have been found. This will
// attempt to read the headers if non are present yet.

int plm_demux_has_headers(plm_demux_t *self);


// Probe the file for the actual number of video streams. See
// plm_probe() for the details.

int plm_demux_probe(plm_demux_t *self, size_t probesize);


// Returns the number of video streams found in the system header. This will
// attempt to read the system header if non is present yet.

int plm_demux_get_num_video_streams(plm_demux_t *self);


// Rewind the internal buffer. See plm_buffer_rewind().

void plm_demux_rewind(plm_demux_t *self);


// Get whether the file has ended. This will be cleared on seeking or rewind.

int plm_demux_has_ended(plm_demux_t *self);


// Seek to a packet of the specified type with a PTS just before specified time.
// If force_intra is TRUE, only packets containing an intra frame will be 
// considered - this only makes sense when the type is PLM_DEMUX_PACKET_VIDEO_1.
// Note that the specified time is considered 0-based, regardless of the first 
// PTS in the data source.

plm_packet_t *plm_demux_seek(plm_demux_t *self, double time, int type, int force_intra);


// Get the PTS of the first packet of this type. Returns PLM_PACKET_INVALID_TS
// if not packet of this packet type can be found.

double plm_demux_get_start_time(plm_demux_t *self, int type);


// Get the duration for the specified packet type - i.e. the span between the
// the first PTS and the last PTS in the data source. This only makes sense when
// the underlying data source is a file or fixed memory.

double plm_demux_get_duration(plm_demux_t *self, int type);


// Decode and return the next packet. The returned packet_t is valid until
// the next call to plm_demux_decode() or until the demuxer is destroyed.

plm_packet_t *plm_demux_decode(plm_demux_t *self);



// -----------------------------------------------------------------------------
// plm_video public API
// Decode MPEG1 Video ("mpeg1") data into raw YCrCb frames


// Create a video decoder with a plm_buffer as source.

plm_video_t *plm_video_create_with_buffer(plm_buffer_t *buffer, int destroy_when_done);


// Destroy a video decoder and free all data.

void plm_video_destroy(plm_video_t *self);


// Get whether a sequence header was found and we can accurately report on
// dimensions and framerate.

int plm_video_has_header(plm_video_t *self);


// Get the framerate in frames per second.

double plm_video_get_framerate(plm_video_t *self);


// Get the display width/height.

int plm_video_get_width(plm_video_t *self);
int plm_video_get_height(plm_video_t *self);


// Set "no delay" mode. When enabled, the decoder assumes that the video does
// *not* contain any B-Frames. This is useful for reducing lag when streaming.
// The default is FALSE.

void plm_video_set_no_delay(plm_video_t *self, int no_delay);


// Get the current internal time in seconds.

double plm_video_get_time(plm_video_t *self);


// Set the current internal time in seconds. This is only useful when you
// manipulate the underlying video buffer and want to enforce a correct
// timestamps.

void plm_video_set_time(plm_video_t *self, double time);


// Rewind the internal buffer. See plm_buffer_rewind().

void plm_video_rewind(plm_video_t *self);


// Get whether the file has ended. This will be cleared on rewind.

int plm_video_has_ended(plm_video_t *self);


// Decode and return one frame of video and advance the internal time by 
// 1/framerate seconds. The returned frame_t is valid until the next call of
// plm_video_decode() or until the video decoder is destroyed.

plm_frame_t *plm_video_decode(plm_video_t *self);


// Convert the YCrCb data of a frame into interleaved R G B data. The stride
// specifies the width in bytes of the destination buffer. I.e. the number of
// bytes from one line to the next. The stride must be at least 
// (frame->width * bytes_per_pixel). The buffer pointed to by *dest must have a
// size of at least (stride * frame->height).
// Note that the alpha component of the dest buffer is always left untouched.

void plm_frame_to_rgb(plm_frame_t *frame, uint8_t *dest, int stride);
void plm_frame_to_bgr(plm_frame_t *frame, uint8_t *dest, int stride);
void plm_frame_to_rgba(plm_frame_t *frame, uint8_t *dest, int stride);
void plm_frame_to_bgra(plm_frame_t *frame, uint8_t *dest, int stride);
void plm_frame_to_argb(plm_frame_t *frame, uint8_t *dest, int stride);
void plm_frame_to_abgr(plm_frame_t *frame, uint8_t *dest, int stride);


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// IMPLEMENTATION

#include <string.h>
#include <stdlib.h>

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#ifndef PLM_MALLOC
	#define PLM_MALLOC(sz) malloc(sz)
	#define PLM_FREE(p) free(p)
	#define PLM_REALLOC(p, sz) realloc(p, sz)
#endif

#define PLM_UNUSED(expr) (void)(expr)


struct plm_t {
	plm_demux_t *demux;
	double time;
	int has_ended;
	int loop;
	int has_decoders;

	int video_packet_type;
	plm_buffer_t *video_buffer;
	plm_video_t *video_decoder;

	plm_video_decode_callback video_decode_callback;
	void *video_decode_callback_user_data;
};

plm_t *plm_create_with_filename(const char *filename);
plm_t *plm_create_with_file(FILE *fh, int close_when_done);
plm_t *plm_create_with_memory(uint8_t *bytes, size_t length, int free_when_done);
plm_t *plm_create_with_buffer(plm_buffer_t *buffer, int destroy_when_done);
int plm_init_decoders(plm_t *self);
void plm_destroy(plm_t *self);
int plm_has_headers(plm_t *self);
int plm_probe(plm_t *self, size_t probesize);
int plm_get_num_video_streams(plm_t *self);
int plm_get_width(plm_t *self);
int plm_get_height(plm_t *self);
double plm_get_framerate(plm_t *self);
double plm_get_time(plm_t *self);
double plm_get_duration(plm_t *self);
void plm_rewind(plm_t *self);
int plm_get_loop(plm_t *self);
void plm_set_loop(plm_t *self, int loop);
int plm_has_ended(plm_t *self);
void plm_set_video_decode_callback(plm_t *self, plm_video_decode_callback fp, void *user);
void plm_decode(plm_t *self, double tick);
plm_frame_t *plm_decode_video(plm_t *self);
void plm_handle_end(plm_t *self);
void plm_read_video_packet(plm_buffer_t *buffer, void *user);
void plm_read_packets(plm_t *self, int requested_type);
plm_frame_t *plm_seek_frame(plm_t *self, double time, int seek_exact);
int plm_seek(plm_t *self, double time, int seek_exact);


struct plm_demux_t {
	plm_buffer_t *buffer;
	int destroy_buffer_when_done;
	double system_clock_ref;

	size_t last_file_size;
	double last_decoded_pts;
	double start_time;
	double duration;

	int start_code;
	int has_pack_header;
	int has_system_header;
	int has_headers;

	int num_video_streams;
	plm_packet_t current_packet;
	plm_packet_t next_packet;
};

plm_demux_t *plm_demux_create(plm_buffer_t *buffer, int destroy_when_done);
void plm_demux_destroy(plm_demux_t *self);
int plm_demux_has_headers(plm_demux_t *self);
int plm_demux_probe(plm_demux_t *self, size_t probesize);
int plm_demux_get_num_video_streams(plm_demux_t *self);
void plm_demux_rewind(plm_demux_t *self);
int plm_demux_has_ended(plm_demux_t *self);
void plm_demux_buffer_seek(plm_demux_t *self, size_t pos);
double plm_demux_get_start_time(plm_demux_t *self, int type);
double plm_demux_get_duration(plm_demux_t *self, int type);
plm_packet_t *plm_demux_seek(plm_demux_t *self, double seek_time, int type, int force_intra);
plm_packet_t *plm_demux_decode(plm_demux_t *self);
double plm_demux_decode_time(plm_demux_t *self);
plm_packet_t *plm_demux_decode_packet(plm_demux_t *self, int type);
plm_packet_t *plm_demux_get_packet(plm_demux_t *self);


enum plm_buffer_mode {
	PLM_BUFFER_MODE_FILE,
	PLM_BUFFER_MODE_FIXED_MEM,
	PLM_BUFFER_MODE_RING,
	PLM_BUFFER_MODE_APPEND
};

struct plm_buffer_t {
	size_t bit_index;
	size_t capacity;
	size_t length;
	size_t total_size;
	int discard_read_bytes;
	int has_ended;
	int free_when_done;
	int close_when_done;
	FILE *fh;
	plm_buffer_load_callback load_callback;
	void *load_callback_user_data;
	uint8_t *bytes;
	enum plm_buffer_mode mode;
};

typedef struct {
	int16_t index;
	int16_t value;
} plm_vlc_t;

typedef struct {
	int16_t index;
	uint16_t value;
} plm_vlc_uint_t;

plm_buffer_t *plm_buffer_create_with_filename(const char *filename);
plm_buffer_t *plm_buffer_create_with_file(FILE *fh, int close_when_done);
plm_buffer_t *plm_buffer_create_with_memory(uint8_t *bytes, size_t length, int free_when_done);
plm_buffer_t *plm_buffer_create_with_capacity(size_t capacity);
plm_buffer_t *plm_buffer_create_for_appending(size_t initial_capacity);
void plm_buffer_destroy(plm_buffer_t *self);
size_t plm_buffer_get_size(plm_buffer_t *self);
size_t plm_buffer_get_remaining(plm_buffer_t *self);
size_t plm_buffer_write(plm_buffer_t *self, uint8_t *bytes, size_t length);
void plm_buffer_signal_end(plm_buffer_t *self);
void plm_buffer_set_load_callback(plm_buffer_t *self, plm_buffer_load_callback fp, void *user);
void plm_buffer_rewind(plm_buffer_t *self);
void plm_buffer_seek(plm_buffer_t *self, size_t pos);
size_t plm_buffer_tell(plm_buffer_t *self);
void plm_buffer_discard_read_bytes(plm_buffer_t *self);
void plm_buffer_load_file_callback(plm_buffer_t *self, void *user);
int plm_buffer_has_ended(plm_buffer_t *self);
int plm_buffer_has(plm_buffer_t *self, size_t count);
int plm_buffer_read(plm_buffer_t *self, int count);
void plm_buffer_align(plm_buffer_t *self);
void plm_buffer_skip(plm_buffer_t *self, size_t count);
int plm_buffer_skip_bytes(plm_buffer_t *self, uint8_t v);
int plm_buffer_next_start_code(plm_buffer_t *self);
int plm_buffer_find_start_code(plm_buffer_t *self, int code);
int plm_buffer_has_start_code(plm_buffer_t *self, int code);
int plm_buffer_peek_non_zero(plm_buffer_t *self, int bit_count);
int16_t plm_buffer_read_vlc(plm_buffer_t *self, const plm_vlc_t *table);
uint16_t plm_buffer_read_vlc_uint(plm_buffer_t *self, const plm_vlc_uint_t *table);

#define PLM_DEFINE_FRAME_CONVERT_FUNCTION(NAME) \
	void NAME(plm_frame_t *frame, uint8_t *dest, int stride);

PLM_DEFINE_FRAME_CONVERT_FUNCTION(plm_frame_to_rgb)
PLM_DEFINE_FRAME_CONVERT_FUNCTION(plm_frame_to_bgr)
PLM_DEFINE_FRAME_CONVERT_FUNCTION(plm_frame_to_rgba)
PLM_DEFINE_FRAME_CONVERT_FUNCTION(plm_frame_to_bgra)
PLM_DEFINE_FRAME_CONVERT_FUNCTION(plm_frame_to_argb)
PLM_DEFINE_FRAME_CONVERT_FUNCTION(plm_frame_to_abgr)

#undef PLM_DEFINE_FRAME_CONVERT_FUNCTION

#endif // PL_MPEG_H

