#pragma once

#include "audio/audio_types.h"
#include "defines.h"

/**
 * The maximum number of individually-controlled channels of audio available, each
 * with separate volume control. These are all nested under a master audio volume.
 */
#define MAX_AUDIO_CHANNELS 16

struct frame_data;

