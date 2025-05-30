#pragma once

#include "defines.h"

typedef struct linear_allocator{
    u64 total_size;
    u64 allocated;
    void* memory;
    b8 owns_memory;
}linear_allocator;


KAPI void linear_allocator_create(u64 total_size,void* memory,linear_allocator* out_allocator);
KAPI void linear_allocator_destroy(linear_allocator* allocator);

KAPI void* linear_allocator_allocate(linear_allocator* allocator,u64 size);

/**
 * @brief Frees everything in the allocator, effectively moving its pointer back to the beginning.
 * Does not free internal memory, if owned. Only resets the pointer.
 *
 * @param allocator A pointer to the allocator to free.
 * @param clear Indicates whether or not to clear/zero the memory. Enabling this obviously takes more processing power.
 */
KAPI void linear_allocator_free_all(linear_allocator* allocator,b8 clear);
