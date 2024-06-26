#include "logger.h"
#include "assert.h"
#include "platform/platform.h"

#include<stdio.h>
#include<string.h>
#include<stdarg.h>

typedef struct logger_system_state
{
    b8 initialized;
}logger_system_state;

static logger_system_state* state_ptr;

b8 initialize_logging(u64* memory_requirement,void* state)
{
    *memory_requirement=sizeof(logger_system_state);
    if(state==0)
    {
        return true;
    }

    state_ptr=state;
    state_ptr->initialized=true;

    //TODO: Remove this
   KFATAL("Test Message:%f",3.14f);
   KERROR("Test Message:%f",3.14f);
   KWARN("Test Message:%f",3.14f);
   KINFO("Test Message:%f",3.14f);
   KDEBUG("Test Message:%f",3.14f);
   KTRACE("Test Message:%f",3.14f);

    //TODO: create log file
   return true;
}

void shutdown_logging(void* state)
{
    state_ptr=0;
    //TODO: cleanup logging/write queued entries

}

//可变参数同样是char*
KAPI void log_output(log_level level,const char* message,...)
{
    const char* level_strings[6]={"[FATAL]:", "[ERROR]: ", "[WARN]:  ", "[INFO]:  ", "[DEBUG]: ", "[TRACE]: "};
    b8 is_error=level<LOG_LEVEL_WARN;
    
    const i32 msg_length=320000;
    char out_message[msg_length];
    memset(out_message,0,sizeof(out_message)); //将一段内存设置为指定的值,不必进行动态内存分配
    // Format original message.
    // NOTE: Oddly enough, MS's headers override the GCC/Clang va_list type with a "typedef char* va_list" in some
    // cases, and as a result throws a strange error here. The workaround for now is to just use __builtin_va_list,
    // which is the type GCC/Clang's va_start expects.
    __builtin_va_list arg_ptr;
    va_start(arg_ptr,message);
    vsnprintf(out_message,msg_length,message,arg_ptr);
    va_end(arg_ptr);
    
    char out_message1[msg_length];
    sprintf(out_message1,"%s%s\n",level_strings[level],out_message);
    // Platform-specific output
    if(is_error)
    {
        platform_console_write_error(out_message1,level);
    }else
    {
        platform_console_write(out_message1,level);
    }
}

KAPI  void report_assertion_failure(const char* expression,const char* message,const char* file,i32 line)
{
    log_output(LOG_LEVEL_FATAL,"Assertion Failure:%s,message:'%s',in fail:%s,line:%d\n",expression,message,file,line);
}
