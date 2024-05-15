#pragma once

#include "defines.h"

#define KASSERTIONS_ENABLED

#ifdef KASSERTIONS_ENABLED
  #if _MSC_VER
     #include <intrin.h>
     #define debugBreak() __debugbreak()
  #else
     #define debugBreak() __builtin_trap()
  #endif

KAPI void report_assertion_failure(const char* expression,const char* message,const char* file,i32 line);

//文件名和行号 #转字符串
#define KASSERT(expr)     \
{                         \
   if(expr){              \
   }else{                 \
      report_assertion_failure(#expr,"",__FILE__,__LINE__);\
      debugBreak(); \
   }                                                       \
}                                     \

#define KASSERT_MSG(expr,message)                \
{\
   if(expr)\
   {}else{\
          report_assertion_failure(#expr,message,__FILE__,__LINE__);     \
          debugBreak();\
   } \
}\

#ifdef _DEBUG
/**
 * @brief Asserts the provided expression to be true, and logs a failure if not.
 * Also triggers a breakpoint if debugging. 
 * NOTE: Only included in debug builds; otherwise this is preprocessed out.
 * @param expr The expression to be evaluated.
 */
#define KASSERT_DEBUG(expr)                                          \
    {                                                                \
        if (expr) {                                                  \
        } else {                                                     \
            report_assertion_failure(#expr, "", __FILE__, __LINE__); \
            debugBreak();                                            \
        }                                                            \
    }
#else
#define KASSERT_DEBUG(expr)  // Does nothing at all
#endif

#else
#define KASSERT(expr)               // Does nothing at all
#define KASSERT_MSG(expr, message)  // Does nothing at all
#define KASSERT_DEBUG(expr)         // Does nothing at all
#endif