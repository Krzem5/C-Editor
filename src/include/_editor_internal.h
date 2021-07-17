#ifndef ___EDITOR_INTERNAL_H__
#define ___EDITOR_INTERNAL_H__
#include <stdint.h>



#define STR_LEN(x) (sizeof((x))/sizeof(char)-1)
#define _EDITOR_LINE_FLAG_UPDATE_CURSOR_POS 2
#define _EDITOR_LINE_FLAG_HAS_CURSOR 4
#define _EDITOR_LINE_FLAG_IN_WHITESPACE 8



uint32_t _copy_str(char* d,const char* s);



#endif
