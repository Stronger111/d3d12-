#include "engine.h"

// Version reporting
#include "kohi.runtime_version.h"

#include "application_types.h"
#include "console.h"
#include "containers/darray.h"
#include "core/event.h"
#include "core/kvar.h"
#include "core/metrics.h"
#include "frame_data.h"
//#include "core/input.h"
#include "kclock.h"
#include "khandle.h"
#include "kmemory.h"
#include "kstring.h"
#include "logger.h"
#include "memory/linear_allocator.h"
#include "platform/platform.h"
#include "renderer/renderer_frontend.h"
#include "uuid.h"

// systems
#include "systems/audio_system.h"
#include "systems/camera_system.h"
#include "systems/font_system.h"
#include "systems/geometry_system.h"
#include "systems/job_system.h"
#include "systems/light_system.h"
#include "systems/material_system.h"
#include "systems/resource_system.h"
#include "systems/shader_system.h"
#include "systems/texture_system.h"
#include "systems/timeline_system.h"
#include "systems/xform_system.h"

typedef struct engine_state_t {
    application* game_inst;
    b8 is_running;
    b8 is_suspended;
    i16 width;
    i16 height;
    kclock clock;
    f64 last_time;

    // Indicates if the window is currently being resized.
    b8 resizing;
    // The current number of frames since the last resize operation.
    // Only set if resizing =true. Otherwise 0.
    u8 frames_since_resize;

    // An allocator used for per-frame allocations, that is reset every frame.
    linear_allocator frame_allocator;

    frame_data p_frame_data;

    u64 platform_memory_requirement;
    struct platform_state* platform;

    u64  console_memory_requirement;
    struct console_state* console;
    u8 platform_consumer_id;

    //Log file.
    file_handle log_file_handle;
    u8 logfile_consumer_id;

    u64 kvar_system_memory_requirement;
    struct kvar_state* kvar;

    u64 event_system_memory_requirement;
    struct event_state* event;

    u64 input_system_memory_requirement;
    struct input_state* input;

    u64 timeline_system_memory_requirement;
    struct timeline_state* timeline;

    u64 resource_system_memory_requirement;
    struct resource_state* resource;

    u64 shader_system_memory_requirement;
    struct shader_system_state* shader;

    u64 renderer_system_memory_requirement;
    struct renderer_system_state* renderer;

    u64 job_system_memory_requirement;
    struct job_system_state* job;

    u64 audio_system_memory_requirement;
    struct audio_system_state* audio;

    u64 xform_system_memory_requirement;
    struct xform_system_state* xform_system;

    u64 texture_system_memory_requirement;
    struct texture_system_state* texture_system;

    u64 font_system_memory_requirement;
    struct font_system_state* font_system;

    u64 material_system_memory_requirement;
    struct material_system_state* material_system;

    u64 geometry_system_memory_requirement;
    struct geometry_system_state* geometry_system;

    u64 light_system_memory_requirement;
    struct light_system_state* light_system;

    u64 camera_system_memory_requirement;
    struct camera_system_state* camera_system;
} engine_state_t;

static engine_state_t* engine_state;

// frame allocator functions.
static void* frame_allocator_allocate(u64 size) {
    if (!engine_state) {
        return 0;
    }
    return linear_allocator_allocate(&engine_state->frame_allocator, size);
}

static void frame_allocator_free(void* block, u64 size) {
    // NOTE: Linear allocator doesn't free, so this is a no-op.
    /* if (engine_state) {
   } */
}

static void frame_allocator_free_all(void) {
    if (engine_state) {
        // Don't wipe the memory each time,to save on performance 擦除内存会浪费性能
        linear_allocator_free_all(&engine_state->frame_allocator, false);
    }
}

// Event handlers
static b8 engine_on_event(u16 code, void* sender, void* listener_inst, event_context context);
static b8 engine_on_resized(u16 code, void* sender, void* listener_inst, event_context context);
static void engine_on_filewatcher_file_deleted(u32 watcher_id);
static void engine_on_filewatcher_file_written(u32 watcher_id);
static void engine_on_window_closed(const struct k_window* window);
static void engine_on_window_resized(const struct k_window* window, u16 width, u16 height);
static void engine_on_process_key(keys key, b8 pressed);
static void engine_on_process_mouse_button(mouse_buttons button, b8 pressed);
static void engine_on_process_mouse_move(i16 x, i16 y);
static void engine_on_process_mouse_wheel(i8 z_delta);
static b8 engine_log_file_write(void* engine, log_level level, const char* message);
static b8 engine_platform_console_write(void* platform, log_level level, const char* message);

b8 engine_create(application* game_inst) {
    if (game_inst->engine_state) {
        KERROR("engine_create called more than once.");
        return false;
    }

    // Memory system must be the first thing to be stood up.
    memory_system_configuration memory_system_config = {};
    memory_system_config.total_alloc_size = GIBIBYTES(2);  // 2G 大小
    if (!memory_system_initialize(memory_system_config)) {
        KERROR("Failed to initialize memory system; shutting down.");
        return false;
    }

    // Seed the uuid generator
    // TODO: a better seed here
    uuid_seed(101);

    // Metrics
    metrics_initialize();

    // Stand up the application state.
    game_inst->engine_state = kallocate(sizeof(engine_state_t), MEMORY_TAG_ENGINE);
    engine_state = game_inst->engine_state;
    engine_state->game_inst = game_inst;
    engine_state->is_running = false;
    engine_state->is_suspended = false;
    engine_state->resizing = false;
    engine_state->frames_since_resize = 0;

    game_inst->app_config.renderer_plugin = game_inst->render_plugin;
    game_inst->app_config.audio_plugin = game_inst->audio_plugin;

    // TODO: Reworking the systems manager to be single-phase. This means _all_ systems should be initialized
    // and ready before game boot. Any systems requiring game config for boot (i.e. renderer) should probably
    // be refactored or have a separate "post-boot" interface entry point.

    // Thinking this order should be something of the following:
    // Platform initialization first (NOTE: NOT window creation - that should happen much later).
    {
        platform_system_config plat_config = { 0 };
        plat_config.application_name = game_inst->app_config.name;
        engine_state->platform_memory_requirement = 0;
        platform_system_startup(&engine_state->platform_memory_requirement, 0, &plat_config);
        engine_state->platform = kallocate(engine_state->platform_memory_requirement, MEMORY_TAG_ENGINE);
        if (!platform_system_startup(&engine_state->platform_memory_requirement, engine_state->platform, &plat_config)) {
            KERROR("Failed to initialize platform layer.");
            return false;
        }
    }

    // Console system
    {
        console_initialize(&engine_state->console_memory_requirement, 0, 0);
        engine_state->console = kallocate(engine_state->console_memory_requirement, MEMORY_TAG_ENGINE);
        if (!console_initialize(&engine_state->console_memory_requirement, engine_state->console, 0)) {
            KERROR("Failed to initialize console.");
            return false;
        }

        //Platform should then register as a console consumer
        console_consumer_register(engine_state->platform, engine_platform_console_write, &engine_state->platform_consumer_id);
        // Setup the engine as another console consumer, which now owns the "console.log" file.
       // Create new/wipe existing log file, then open it.
        if (!filesystem_open("console.log", FILE_MODE_WRITE, false, &engine_state->log_file_handle)) {
            KFATAL("Unable to open console.log for writing.");
            return false;
        }
        console_consumer_register(engine_state, engine_log_file_write, &engine_state->logfile_consumer_id);
    }

    // Report runtime version
#if KRELEASE
    const char* build_type = "Release";
#else
    const char* build_type = "Debug";
#endif
    KINFO("Kohi Runtime v. %s (%s)", KVERSION, build_type);

    //KVar system
    {
        kvar_system_initialize(&engine_state->kvar_system_memory_requirement, 0, 0);
        engine_state->kvar = kallocate(engine_state->kvar_system_memory_requirement, MEMORY_TAG_ENGINE);
        if (!kvar_system_initialize(&engine_state->kvar_system_memory_requirement, engine_state->kvar, 0)) {
            KERROR("Failed to initialize KVar system.");
            return false;
        }
    }

    // Event system.
    {
        event_system_initialize(&engine_state->event_system_memory_requirement, 0, 0);
        engine_state->event = kallocate(engine_state->event_system_memory_requirement, MEMORY_TAG_ENGINE);
        if (!event_system_initialize(&engine_state->event_system_memory_requirement, engine_state->event, 0)) {
            KERROR("Failed to initialize event system.");
            return false;
        }

        //TODO:After event system, register watcher and input callbacks
    }

    // Perform the games boot sequence
    game_inst->stage = APPLICATION_STAGE_BOOTING;
    if (!game_inst->boot(game_inst)) {
        KFATAL("Game boot sequence failed; aborting application.");
        return false;
    }

    // Setup the frame allocator.
    linear_allocator_create(game_inst->app_config.frame_allocator_size, 0, &engine_state->frame_allocator);
    engine_state->p_frame_data.allocator.allocate = frame_allocator_allocate;
    engine_state->p_frame_data.allocator.free = frame_allocator_free;
    engine_state->p_frame_data.allocator.free_all = frame_allocator_free_all;

    // Allocate for the  application's frame data.
    if (game_inst->app_config.app_frame_data_size > 0) {
        engine_state->p_frame_data.application_frame_data = kallocate(game_inst->app_config.app_frame_data_size, MEMORY_TAG_GAME);
    }
    else {
        engine_state->p_frame_data.application_frame_data = 0;
    }

    game_inst->stage = APPLICATION_STAGE_BOOT_COMPLETE;

    if (!systems_manager_post_boot_initialize(&engine_state->sys_manager_state, &game_inst->app_config)) {
        KFATAL("Post-boot system manager initialization failed!");
        return false;
    }

    // Initialize the game
    game_inst->stage = APPLICATION_STAGE_INITIALIZING;
    if (!engine_state->game_inst->initialize(engine_state->game_inst)) {
        KFATAL("Game failed to initialize.");
        return false;
    }
    game_inst->stage = APPLICATION_STAGE_INITIALIZED;

    return true;
}

b8 engine_run(application* game_inst) {
    game_inst->stage = APPLICATION_STAGE_RUNNING;
    engine_state->is_running = true;
    kclock_start(&engine_state->clock);
    kclock_update(&engine_state->clock);
    engine_state->last_time = engine_state->clock.elapsed;
    // f64 running_time = 0;
    // TODO: frame rate lock
    // u8 frame_count = 0;
    f64 target_frame_seconds = 1.0f / 60;
    f64 frame_elapsed_time = 0;  // 帧过去的时间

    KINFO(get_memory_usage_str());

    void* timeline_state = systems_manager_get_state(K_SYSTEM_TYPE_TIMELINE);
    while (engine_state->is_running) {
        if (!platform_pump_messages()) {
            engine_state->is_running = false;
        }
        // 不是暂停状态
        if (!engine_state->is_suspended) {
            // Update clock and get delta time
            kclock_update(&engine_state->clock);
            f64 current_time = engine_state->clock.elapsed;
            f64 delta = (current_time - engine_state->last_time);
            f64 frame_start_time = platform_get_absolute_time();

            // Reset the frame allocator
            engine_state->p_frame_data.allocator.free_all();

            // Update  system.
            systems_manager_update(&engine_state->sys_manager_state, &engine_state->p_frame_data);
            // Update timelines. Note that this is not done by the systems manager.
            // because we don't want or have timeline data in the frame data struct any longer.
            timeline_system_update(timeline_state, delta);

            // update metrics
            metrics_update(frame_elapsed_time);

            // Make sure the window is not currently being resized by waiting a designed
            // number of frames after the last resize operation before performing the backend updates.
            if (engine_state->resizing) {
                engine_state->frames_since_resize++;

                // If the required number of frames have passed since the resize,go ahead and perform the actual updates.
                if (engine_state->frames_since_resize >= 30) {
                    renderer_on_resized(engine_state->width, engine_state->height);
                    // NOTE: Don't bother checking the result of this,since this will likely
                    // recreate the swapchain and boot to the next frame anyway.
                    renderer_frame_prepare(&engine_state->p_frame_data);

                    // Notify the application of the resize.
                    engine_state->game_inst->on_resize(engine_state->game_inst, engine_state->width, engine_state->height);

                    engine_state->frames_since_resize = 0;
                    engine_state->resizing = false;
                }
                else {
                    // Skip rendering the frame and try again next time.
                    // NOTE: Simulate a frame being "drawn" at 60 FPS.
                    platform_sleep(16);
                }
                // Either way, don't process this frame any further while resizing.
                // Try again next frame.
                continue;
            }

            if (!renderer_frame_prepare(&engine_state->p_frame_data)) {
                // This can also happen not just from a resize above, but also if a renderer flag
                // (such as VSync) changed, which may also require resource recreation. To handle this,
                // Notify the application of a resize event, which it can then pass on to its rendergraph(s)
                // as needed.
                engine_state->game_inst->on_resize(engine_state->game_inst, engine_state->width, engine_state->height);
                continue;
            }

            if (!engine_state->game_inst->update(engine_state->game_inst, &engine_state->p_frame_data)) {
                KFATAL("Game update failed,shutting down.");
                engine_state->is_running = false;
                break;
            }

            if (!renderer_begin(&engine_state->p_frame_data)) {
                KFATAL("Failed to begin renderer.Shutting down.");
                engine_state->is_running = false;
                break;
            }

            // Begin "prepare_frame" render event grouping.
            renderer_begin_debug_label("prepare_frame", (vec3) { 1.0f, 1.0f, 0.0f });

            systems_manager_renderer_frame_prepare(&engine_state->sys_manager_state, &engine_state->p_frame_data);

            // Have the application generate the render packet.
            b8 prepare_result = engine_state->game_inst->prepare_frame(engine_state->game_inst, &engine_state->p_frame_data);

            // End "prepare_frame" render event grouping.
            renderer_end_debug_label();

            if (!prepare_result) {
                continue;
            }

            // call the games render routine
            if (!engine_state->game_inst->render_frame(engine_state->game_inst, &engine_state->p_frame_data)) {
                KFATAL("Game update failed,shutting down");
                engine_state->is_running = false;
                break;
            }

            // End the frame.
            renderer_end(&engine_state->p_frame_data);

            // Present the frame.
            if (!renderer_present(&engine_state->p_frame_data)) {
                KERROR("The call to renderer_present failed. This is likely unrecoverable. Shutting down.");
                engine_state->is_running = false;
                break;
            }

            // Figure out how long the frame took and ,if below
            f64 frame_end_time = platform_get_absolute_time();
            frame_elapsed_time = frame_end_time - frame_start_time;
            // running_time += frame_elapsed_time;
            f64 remaining_seconds = target_frame_seconds - frame_elapsed_time;

            if (remaining_seconds > 0) {
                u64 remaining_ms = (remaining_seconds * 1000);

                // If there is time left. give it back to the os
                b8 limit_frames = false;
                if (remaining_ms > 0 && limit_frames) {
                    platform_sleep(remaining_ms - 1);
                }
                // TODO: frame rate lock
                // frame_count++;
            }

            // 渲染后是键盘鼠标事件系统
            // NOTE: Input updte/state copying should always be handled
            // after any input should be recorded.I.E before this line.
            // As is Safety.input is the last thing to be updated before
            // this frame ends.
            input_update(&engine_state->p_frame_data);

            // Update last time
            engine_state->last_time = current_time;
        }
    }
    engine_state->is_running = false;
    game_inst->stage = APPLICATION_STAGE_SHUTTING_DOWN;

    // Shut down the game.
    engine_state->game_inst->shutdown(engine_state->game_inst);

    // Unregister from events.
    event_unregister(EVENT_CODE_APPLICATION_QUIT, 0, engine_on_event);

    // Shut down all systems
    systems_manager_shutdown(&engine_state->sys_manager_state);
    game_inst->stage = APPLICATION_STAGE_UNINITIALIZED;
    return true;
}

void engine_on_event_system_initialized(void) {
    // Register for engine-level events.
    event_register(EVENT_CODE_APPLICATION_QUIT, 0, engine_on_event);
    event_register(EVENT_CODE_RESIZED, 0, engine_on_resized);
}

const struct frame_data* engine_frame_data_get(struct application* game_inst) {
    return &((engine_state_t*)game_inst->engine_state)->p_frame_data;
}

systems_manager_state* engine_systems_manager_state_get(struct application* game_inst) {
    return &((engine_state_t*)game_inst->engine_state)->sys_manager_state;
}

static b8 engine_on_event(u16 code, void* sender, void* listener_inst, event_context context) {
    switch (code) {
    case EVENT_CODE_APPLICATION_QUIT: {
        KINFO("EVENT_CODE_APPLICATION_QUIT recieved.shutting down.\n");
        engine_state->is_running = false;
        return true;
    }
    }
    return false;
}

static b8 engine_on_resized(u16 code, void* sender, void* listener_inst, event_context context) {
    if (code == EVENT_CODE_RESIZED) {
        // Flag as resizing and store the change,but wait to regenerate.
        engine_state->resizing = true;
        // Also reset the frame count since the last resize operation.
        engine_state->frames_since_resize = 0;

        u16 width = context.data.u16[0];
        u16 height = context.data.u16[1];

        // check if different,If so, triggle a resize event
        if (width != engine_state->width || height != engine_state->height) {
            engine_state->width = width;
            engine_state->height = height;

            KDEBUG("Window resize:%i %i", width, height);

            // Handle minimization
            if (width == 0 || height == 0) {
                KINFO("Window minimized,suspending application.");
                engine_state->is_suspended = true;
                return true;
            }
            else {
                if (engine_state->is_suspended) {
                    KINFO("Window restored,resuming application.");
                    engine_state->is_suspended = false;
                }
            }
        }
    }
    // Event purposely not handled to allow other listeners to get this.
    return false;
}
