
#include "audio_loader.h"

#include <audio/audio_types.h>
#include <platform/filesystem.h>
#include <resources/loaders/loader_utils.h>

#include "core/kmemory.h"
#include "core/kstring.h"
#include "core/logger.h"
#include "defines.h"
#include "math/kmath.h"
#include "resources/resource_types.h"
#include "systems/resource_system.h"

// Loading vorbis files.
#include "vendor/stb_vorbis.h"
// Loading mp3 files.
#define MINIMP3_IMPLEMENTATION
#include "vendor/minimp3_ex.h"

// MP3 decoder
static mp3dec_t decoder;

typedef struct audio_file_internal {
    // The internal ogg vorbis file handle, if the file is ogg. Otherwise null.
    stb_vorbis* vorbis;
    // The internal mp3 file handle.
    mp3dec_file_info_t mp3_info;
    // Pulse-code modulation buffer, or raw data to be fed into a buffer.
    // Only used for some formats
    i16* pcm;
    u64 pcm_size;
} audio_file_internal;

static u64 audio_file_load_samples(struct audio_file* audio, u32 chunk_size, i32 count) {
    if (audio->internal_data->vorbis) {
        i64 samples = stb_vorbis_get_samples_short_interleaved(audio->internal_data->vorbis, audio->channels, audio->internal_data->pcm, chunk_size);
        // Sample here does not include channels, so factor them in.
        return samples * audio->channels;
    } else if (audio->internal_data->mp3_info.buffer) {
        // Samples count includes channels.
        return KMIN(audio->total_samples_left, chunk_size);
    }
    KERROR("Error loading samples:Unknown file type.");
    return INVALID_ID_U64;
}

static void* audio_file_stream_buffer_data(struct audio_file* audio) {
    if (audio->internal_data->vorbis) {
        return audio->internal_data->pcm;
    } else if (audio->internal_data->mp3_info.buffer) {
        u64 pos = audio->internal_data->mp3_info.samples - audio->total_samples_left;
        return audio->internal_data->mp3_info.buffer + pos;
    } else {
        KERROR("Error streaming audio dta:Unknown file type. Null is returned.");
        return 0;
    }
}

void audio_file_rewind(struct audio_file* audio) {
    if (audio) {
        if (audio->internal_data->vorbis) {
            stb_vorbis_seek_start(audio->internal_data->vorbis);
            // Reset sample counter.
            audio->total_samples_left = stb_vorbis_stream_length_in_samples(audio->internal_data->vorbis) * audio->channels;
        } else if (audio->internal_data->mp3_info.buffer) {
            // Reset sample counter.
            audio->total_samples_left = audio->internal_data->mp3_info.samples;
        } else {
            KERROR("Error rewinding audio file: unknown type.");
            return;
        }
    }
}

static b8 audio_loader_load(struct resource_loader* self, const char* name, void* params, resource* out_resource) {
    if (!self || !name || !out_resource) {
        return false;
    }

    audio_resource_loader_params* typed_params = params;

    char* format_str = "%s/%s/%s%s";
    char full_file_path[512];
    string_format(full_file_path, format_str, resource_system_base_path(), self->type_path, name, "");

    out_resource->full_path = string_duplicate(full_file_path);

    audio_file* resource_data = kallocate(sizeof(audio_file), MEMORY_TAG_RESOURCE);
    // Keep a pointer to this to free it later.
    resource_data->audio_resource = out_resource;
    resource_data->type = typed_params->type;
    resource_data->internal_data = kallocate(sizeof(audio_file_internal), MEMORY_TAG_RESOURCE);
    if (string_index_of_str(".ogg", full_file_path) != -1) {
        KTRACE("Processing OGG music file '%s'...", full_file_path);
        i32 ogg_error = 0;
        resource_data->internal_data->vorbis = stb_vorbis_open_filename(full_file_path, &ogg_error, 0);
        if (!resource_data->internal_data->vorbis) {
            enum STBVorbisError error = ogg_error;
            // TODO: error reporting.
            switch (error) {
                default:
                    break;
            }
            KERROR("Failed to load vorbis file with error:%u", ogg_error);
            return false;
        }
        
    }
}

KAPI resource_loader audio_resource_loader_create(void) {
    return resource_loader();
}