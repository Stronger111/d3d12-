#pragma once

#include "defines.h"

struct audio_plugin;

//plugin entry point
KAPI b8 plugin_create(struct audio_plugin* out_plugin);