#pragma once

#include "defines.h"

typedef union vec2_u {
    // An array of x,y
    f32 elements[2];
    struct
    {
        union {
            // The first element
            f32 x, r, s, u;
        };

        union {
            // The second element
            f32 y, g, t, v;
        };
    };
} vec2;

typedef struct vec3_u {
    union {
        // An array of x,y,z
        f32 elements[3];

        struct
        {
            union {
                // The first element
                f32 x, r, s, u;
            };

            union {
                // The second element
                f32 y, g, t, v;
            };

            union {
                // The third element
                f32 z, b, p, w;
            };
        };
    };
} vec3;

typedef union vec4_u {
    //    #if defined(KUSE_SIMD)
    //       //Used for SIMD operations
    //       alignas(16) __m128 data;
    //    #endif
    // An array of x,y,z,w alignas(16)
    f32 elements[4];
    union {
        struct
        {
            union {
                // The first element
                f32 x, r, s;
            };

            union {
                // The second element
                f32 y, g, t;
            };
            union {
                // The third element
                f32 z, b, p;
            };
            union {
                // The fourth element
                f32 w, a, q;
            };
        };
    };
} vec4;

typedef vec4 quat;

/** @brief A 3x3 matrix */
typedef union mat3_u {
    /** @brief The matrix elements. */
    f32 data[12];
}mat3;

typedef union mat4_u {
    f32 data[16];
} mat4;

/**
 * @brief Represents the extents of a 2d object.
 */
typedef struct extents_2d {
    /** @brief The minimum extents of the object. */
    vec2 min;
    /** @brief The maximum extents of the object. */
    vec2 max;
} extents_2d;

/**
 * @brief Represents the extents of a 3d object.
 */
typedef struct extents_3d {
    /** @brief The minimum extents of the object. */
    vec3 min;
    /** @brief The maximum extents of the object. */
    vec3 max;
} extents_3d;

/**
 * @brief Represents a single vertex in 3D space.
 */
typedef struct vertex_3d {
    vec3 position;
    /** @brief The normal of the vertex. */
    vec3 normal;
    vec2 texcoord;
    /** @brief The colour of the vertex. */
    vec4 colour;
    /** @brief The tangent of the vertex. */
    vec3 tangent;
} vertex_3d;

typedef struct vertex_2d {
    vec2 position;
    vec2 texcoord;
} vertex_2d;

/**
 * @brief Represents a single vertex in 3D space with position and colour data only.
 */
typedef struct colour_vertex_3d {
    /** @brief The position of the vertex. w is ignored. */
    vec4 position;
    /** @brief The colour of the vertex. */
    vec4 colour;
} colour_vertex_3d;

/**
 * @brief Represents the transform of an object in the world.
 * Transforms can have a parent whose own transform is then
 * taken into account. NOTE: The properties of this should not
 * be edited directly, but done via the functions in transform.h
 * to ensure proper matrix generation.
 */
typedef struct transform {
    /** @brief The position in the world. */
    vec3 position;
    /** @brief The rotation in the world. */
    quat rotation;
    /** @brief The scale in the world. */
    vec3 scale;
    /**
     * @brief Indicates if the position, rotation or scale have changed,
     * indicating that the local matrix needs to be recalculated.
     */
    b8 is_dirty;
    /**
     * @brief The local transformation matrix, updated whenever
     * the position, rotation or scale have changed.
     */
    mat4 local;
    //行列式
    f32 determinant;

    /** @brief A pointer to a parent transform if one is assigned. Can also be null. */
    struct transform* parent;
} transform;

typedef struct plane_3d {
    vec3 normal;
    f32 distance;
} plane_3d;

typedef struct frustum {
    // Top,bottom,right,left,far,near
    plane_3d sides[6];
} frustum;
