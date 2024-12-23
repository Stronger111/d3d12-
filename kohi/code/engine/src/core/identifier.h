/**
 * @file identifier.h
 * @author Travis Vroman (travis@kohiengine.com)
 * @brief Contains a system for creating numeric identifiers.
 * @version 2.0
 * @date 2023-10-22
 *
 * @copyright Kohi Game Engine is Copyright (c) Travis Vroman 2021-2023
 *
 */


#pragma once

#include "defines.h"

/**
 * A Globally/Universally Unique identifier in 64-bit unsigned integer format.
 * To be used primarily as an identifier for resources. (De)serialization friendly.
 */
typedef struct identifier{
   //The actual internal identifier.
   u64 uniqueid;
}identifier;

/**
 * @brief Acquires a new identifier for the given owner.
 *
 * @param owner The owner of the identifier.
 * @return The new identifier.
 */
KAPI u32 identifier_aquire_new_id(void* owner);

/**
 * @brief Releases the given identifier, which can then be used
 * again.
 *
 * @param id The identifier to be released.
 */
void identifier_release_id(u32 id);