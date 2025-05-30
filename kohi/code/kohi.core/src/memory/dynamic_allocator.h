#pragma once

#include "defines.h"

typedef struct dynamic_allocator
{
   void* memory;
}dynamic_allocator;

KAPI b8 dynamic_allocator_create(u64 total_size,u64* memory_requirement,void* memory,dynamic_allocator* out_allocator);

KAPI b8 dynamic_allocator_destroy(dynamic_allocator* allocator);

KAPI void* dynamic_allocator_allocate(dynamic_allocator* allocator,u64 size);

/**
 * @brief Allocates the given amount of aligned memory from the provided allocator.
 *
 * @param allocator A pointer to the allocator to allocate from.
 * @param size The amount in bytes to be allocated.
 * @param alignment The alignment in bytes.
 * @return The aligned, allocated block of memory unless this operation fails, then 0.
 */
KAPI void* dynamic_allocator_allocate_aligned(dynamic_allocator* allocator,u64 size,u16 alignment);

KAPI b8 dynamic_allocator_free(dynamic_allocator* allocator,void* block,u64 size);

/**
 * @brief Frees the given block of aligned memory. Technically the same as calling
 * dynamic_allocator_free, but here for API consistency. No size is required.
 *
 * @param allocator A pointer to the allocator to free from.
 * @param block The block to be freed. Must have been allocated by the provided allocator.
 * @return True on success; otherwise false.
 */
KAPI b8 dynamic_allocator_free_aligned(dynamic_allocator* allocator,void* block);

/**
 * @brief Obtains the size and alignment of the given block of memory. Can fail if
 * invalid data is passed.
 *
 * @param block The block of memory.
 * @param out_size A pointer to hold the size.
 * @param out_alignment A pointer to hold the alignment.
 * @return True on success; otherwise false.
 */
KAPI b8 dynamic_allocator_get_size_alignment(void* block,u64* out_size,u16* out_alignment);

KAPI u64 dynamic_allocator_free_space(dynamic_allocator* allocator);

/**
 * @brief Obtains the amount of total space originally available in the provided allocator.
 *
 * @param allocator A pointer to the allocator to be examined.
 * @return The total amount of space originally available in bytes.
 */
KAPI u64 dynamic_allocator_total_space(dynamic_allocator* allocator);

/** Obtains the size of the internal allocation header. This is really only used for unit testing purposes. */
KAPI u64 dynamic_allocator_header_size(void);