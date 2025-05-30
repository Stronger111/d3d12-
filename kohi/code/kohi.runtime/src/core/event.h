#pragma once

#include "defines.h"

typedef struct event_context {
    // 128bytes
    union {
        i64 i64[2];
        u64 u64[2];
        f64 f64[2];

        i32 i32[4];
        u32 u32[4];
        f32 f32[4];

        i16 i16[8];
        u16 u16[8];

        i8 i8[16];
        u8 u8[16];

        char c[16];
    } data;
} event_context;

// Should return true if handled. 函数指针
typedef b8 (*PFN_on_event)(u16 code, void* sender, void* listener_inst, event_context data);

b8 event_system_initialize(u64* memory_requirement, void* state, void* config);

/**
 * @brief Shuts the event system down.
 */
void event_system_shutdown(void* state);

KAPI b8 event_register(u16 code, void* listener, PFN_on_event on_event);

KAPI b8 event_unregister(u16 code, void* listener, PFN_on_event on_event);

KAPI b8 event_fire(u16 code, void* sender, event_context context);

// System internal event codes. Application should use codes beyond 255
typedef enum system_event_code {
    // Shuts the application down on the next frame.
    EVENT_CODE_APPLICATION_QUIT = 0x01,

    // keyboard key pressed
    /*
       Context usage:
      * u16 key_code=data.data.u16[0];
      * u16 repeat_count = data.data.u16[1];
    */
    EVENT_CODE_KEY_PRESSED = 0x02,
    // keyboard key released
    /*
       Context usage:
       u16 key_code=data.data.u16[0];
      * u16 repeat_count = data.data.u16[1];
    */
    EVENT_CODE_KEY_RELEASED = 0x03,
    /** @brief Mouse button pressed.
     * Context usage:
     * u16 button = data.data.u16[0];
     * u16 x = data.data.i16[1];
     * u16 y = data.data.i16[2];
     */
    EVENT_CODE_BUTTON_PRESSED = 0x04,

    /** @brief Mouse button released.
     * Context usage:
     * u16 button = data.data.u16[0];
     * u16 x = data.data.i16[1];
     * u16 y = data.data.i16[2];
     */
    EVENT_CODE_BUTTON_RELEASED = 0x05,

    /** @brief Mouse button pressed then released.
     * Context usage:
     * u16 button = data.data.u16[0];
     * u16 x = data.data.i16[1];
     * u16 y = data.data.i16[2];
     */
    EVENT_CODE_BUTTON_CLICKED = 0x06,
    // Mouse moved
    /*
       Context usage:
        u16 x=data.data.i16[0];
        u16 y=data.data.i16[1];
    */
    EVENT_CODE_MOUSE_MOVED = 0x07,
    // Mouse moved
    /*
       Context usage:
        ui z_delta=data.data.i8[0]
    */
    EVENT_CODE_MOUSE_WHEEL = 0x08,
    // Resized/resolution changed from the OS
    /*  0xFF 255
       Context usage:
         u16 width=data.data.u16[0];
         u16 height=data.data.u16[1];
    */
    EVENT_CODE_RESIZED = 0x09,
    // Change the render mode for debugging purposes.
    /* Context usage:
     * i32 mode = context.data.i32[0];
     */
    EVENT_CODE_SET_RENDER_MODE = 0x0A,
    EVENT_CODE_DEBUG0 = 0x10,
    EVENT_CODE_DEBUG1 = 0x11,
    EVENT_CODE_DEBUG2 = 0x12,
    EVENT_CODE_DEBUG3 = 0x13,
    EVENT_CODE_DEBUG4 = 0x14,

    EVENT_CODE_DEBUG5 = 0x15,
    EVENT_CODE_DEBUG6 = 0x16,
    EVENT_CODE_DEBUG7 = 0x17,
    EVENT_CODE_DEBUG8 = 0x18,
    EVENT_CODE_DEBUG9 = 0x19,
    EVENT_CODE_DEBUG10 = 0x1A,
    EVENT_CODE_DEBUG11 = 0x1B,
    EVENT_CODE_DEBUG12 = 0x1C,
    EVENT_CODE_DEBUG13 = 0x1D,
    EVENT_CODE_DEBUG14 = 0x1E,
    EVENT_CODE_DEBUG15 = 0x1F,

    /** @brief The hovered-over object id, if there is one.
     * Context usage:
     * i32 id = context.data.u32[0]; - will be INVALID ID if nothing is hovered over.
     */
    EVENT_CODE_OBJECT_HOVER_ID_CHANGED = 0x20,
    /**
     * @brief An event fired by the renderer backend to indicate when any render targets
     * associated with the default window resources need to be refreshed (i.e. a window resize)
     */
    EVENT_CODE_DEFAULT_RENDERTARGET_REFRESH_REQUIRED = 0x21,

    /**
     * @brief An event fired by the kvar system when a kvar has been updated.
     */
    EVENT_CODE_KVAR_CHANGED = 0x22,

    /**
     * @brief An event fired when a watched file has been written to.
     * Context usage:
     * u32 watch_id = context.data.u32[0];
     */
    EVENT_CODE_WATCHED_FILE_WRITTEN = 0x23,
    /**
     * @brief An event fired when a watched file has been removed.
     * Context usage:
     * u32 watch_id = context.data.u32[0];
     */
    EVENT_CODE_WATCHED_FILE_DELETED = 0x24,

    /**
     * @brief An event fired while a button is being held down and the
     * mouse is moved.
     *
     * Context usage:
     * i16 x = context.data.i16[0]
     * i16 y = context.data.i16[1]
     * u16 button = context.data.u16[2]
     */
    EVENT_CODE_MOUSE_DRAGGED = 0x30,

    /**
     * @brief An event fired when a button is pressed and a mouse movement
     * is done while it is pressed.
     *
     * Context usage:
     * i16 x = context.data.i16[0]
     * i16 y = context.data.i16[1]
     * u16 button = context.data.u16[2]
     */
    EVENT_CODE_MOUSE_DRAG_BEGIN = 0x31,

    /**
     * @brief An event fired when a button is released was previously dragging.
     *
     * Context usage:
     * i16 x = context.data.i16[0]
     * i16 y = context.data.i16[1]
     * u16 button = context.data.u16[2]
     */
    EVENT_CODE_MOUSE_DRAG_END = 0x32,

    MAX_EVENT_CODE = 0xFF
} system_event_code;
