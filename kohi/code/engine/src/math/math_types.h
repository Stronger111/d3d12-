#pragma once

#include "defines.h"

typedef union vec2_u
{
    //An array of x,y
    f32 elements[2];
    struct 
    {
        union 
        {
            //The first element
            f32 x,r,s,u;
        };

        union 
        {
            //The second element
            f32 y,g,t,v;
        };
    };
}vec2;

typedef struct vec3_u
{
    union 
    {
        //An array of x,y,z
        f32 elements[3];

        struct 
        {
            union 
            {
                //The first element
                f32 x,r,s,u;
            };

            union 
            {
                //The second element
                f32 y,g,t,v;
            };

            union 
            {
                //The third element
                f32 z,b,p,w;
            };
        };
        
    };
}vec3;

typedef union vec4_u
{
//    #if defined(KUSE_SIMD)
//       //Used for SIMD operations
//       alignas(16) __m128 data;
//    #endif
   //An array of x,y,z,w alignas(16)
   f32 elements[4];
   union 
   {
      struct 
      {
          union 
          {
             //The first element
             f32 x,r,s;
          };

          union 
          {
             //The second element
             f32 y,g,t;
          };
          union 
          {
             //The third element
             f32 z,b,p;
          };
          union 
          {
             //The fourth element
             f32 w,a,q;
          };
      };
   };
}vec4;

typedef vec4 quat;

typedef union mat4_u
{
    f32 data[16];
}mat4;

/**
 * @brief Represents a single vertex in 3D space.
 */
typedef struct vertex_3d
{
    vec3 position;
    /** @brief The normal of the vertex. */
    vec3 normal;
    vec2 texcoord;
    /** @brief The colour of the vertex. */
    vec4 colour;
    /** @brief The tangent of the vertex. */
    vec4 tangent;
}vertex_3d;

typedef struct vertex_2d
{
   vec2 position;
   vec2 texcoord;
}vertex_2d;
