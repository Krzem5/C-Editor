#ifndef __EDITOR_H__
#define __EDITOR_H__ 1
#include <stdint.h>



#define EDITOR_CREATE_UPPER_KEY(k) ((k)<<8)
#define EDITOR_FLAG_OPEN 1
#define EDITOR_KEY_BACKSPACE EDITOR_CREATE_UPPER_KEY(1)
#define EDITOR_KEY_CTRL_C EDITOR_CREATE_UPPER_KEY(2)
#define EDITOR_KEY_CTRL_DOWN EDITOR_CREATE_UPPER_KEY(3)
#define EDITOR_KEY_CTRL_LEFT EDITOR_CREATE_UPPER_KEY(4)
#define EDITOR_KEY_CTRL_RIGHT EDITOR_CREATE_UPPER_KEY(5)
#define EDITOR_KEY_CTRL_UP EDITOR_CREATE_UPPER_KEY(6)
#define EDITOR_KEY_DELETE EDITOR_CREATE_UPPER_KEY(7)
#define EDITOR_KEY_DOWN EDITOR_CREATE_UPPER_KEY(8)
#define EDITOR_KEY_END EDITOR_CREATE_UPPER_KEY(9)
#define EDITOR_KEY_ENTER EDITOR_CREATE_UPPER_KEY(10)
#define EDITOR_KEY_ESCAPE EDITOR_CREATE_UPPER_KEY(11)
#define EDITOR_KEY_HOME EDITOR_CREATE_UPPER_KEY(12)
#define EDITOR_KEY_LEFT EDITOR_CREATE_UPPER_KEY(13)
#define EDITOR_KEY_PAGE_CTRL_DOWN EDITOR_CREATE_UPPER_KEY(14)
#define EDITOR_KEY_PAGE_CTRL_UP EDITOR_CREATE_UPPER_KEY(15)
#define EDITOR_KEY_PAGE_DOWN EDITOR_CREATE_UPPER_KEY(16)
#define EDITOR_KEY_PAGE_UP EDITOR_CREATE_UPPER_KEY(17)
#define EDITOR_KEY_RIGHT EDITOR_CREATE_UPPER_KEY(18)
#define EDITOR_KEY_UP EDITOR_CREATE_UPPER_KEY(19)



typedef struct __EDITOR_LINE{
	uint32_t l;
	char* dt;
} editor_line_t;



typedef struct __EDITOR{
	char fp[4096];
	uint16_t fpl;
	uint8_t fl;
	uint32_t cx;
	uint32_t cy;
	uint32_t w;
	uint32_t h;
	uint32_t ll;
	editor_line_t** l;
} editor_t;



void init_editor(editor_t* e);



void set_window_size(editor_t* e,uint32_t w,uint32_t h);



void open_file(editor_t* e,const char* fp);



void type_key(editor_t* e,uint16_t k);



void render_editor(editor_t* e);



void free_editor(editor_t* e);



#endif
