#include "test_manager.h"

#include <containers/darray.h>
#include <core/logger.h>
#include <core/kstring.h>
#include <core/kclock.h>
#include <core/kmemory.h>

typedef struct test_entry {
    PFN_test func;
    char* desc;
} test_entry;

static test_entry* tests;

void test_manager_init(void) {
    // FIXED:DS 防止未初始化报错 在调用内存对齐之前进行初始化内存管理
    memory_system_configuration memory_system_config = {};
    memory_system_config.total_alloc_size = GIBIBYTES(2);  // 2G 大小
    if (!memory_system_initialize(memory_system_config)) {
        KERROR("Failed to initialize memory system; shutting down.");
    }
    tests = darray_create(test_entry);
}

void test_manager_register_test(u8 (*PFN_test)(void), char* desc) {
    test_entry e;
    e.func = PFN_test;
    e.desc = desc;
    darray_push(tests, e);
}

void test_manager_run_tests(void) {
    u32 passed = 0;
    u32 failed = 0;
    u32 skipped = 0;

    u32 count = darray_length(tests);

    kclock total_time;
    kclock_start(&total_time);

    for (u32 i = 0; i < count; ++i) {
        kclock test_time;
        kclock_start(&test_time);
        u8 result = tests[i].func();
        kclock_update(&test_time);

        if (result == true) {
            ++passed;
        } else if (result == BYPASS) {
            KWARN("[SKIPPED]: %s", tests[i].desc);
            ++skipped;
        } else {
            KERROR("[FAILED]: %s", tests[i].desc);
            ++failed;
        }
        char status[20];
        string_format_unsafe(status, failed ? "*** %d FAILED ***" : "SUCCESS", failed);
        kclock_update(&total_time);
        KINFO("Executed %d of %d (skipped %d) %s (%.6f sec / %.6f sec total", i + 1, count, skipped, status, test_time.elapsed, total_time.elapsed);
    }

    kclock_stop(&total_time);

    KINFO("Results: %d passed, %d failed, %d skipped.", passed, failed, skipped);
}
