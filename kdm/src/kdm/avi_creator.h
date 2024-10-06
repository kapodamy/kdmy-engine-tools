#ifndef _avi_creator_h
#define _avi_creator_h

#include "../ffgraph/ffgraph.h"

typedef void CB_Progress(void* userdata, double time);

char* avicreator_process_video(FFGraph* ffgraph, CB_Progress progress, void* userdata, const char* filename_to_suffix, int bitrate_kbps, bool volatile* has_int_signal);

#endif
