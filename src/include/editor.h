#ifndef __EDITOR_H__
#define __EDITOR_H__ 1
#include <stdint.h>



#define EDITOR_CREATE_UPPER_KEY(k) ((k)<<8)
#define EDITOR_ERROR_HEIGHT 4
#define EDITOR_ERROR_WIDTH 48
#define EDITOR_FLAG_OPEN 1
#define EDITOR_KEY_BACKSPACE EDITOR_CREATE_UPPER_KEY(1)
#define EDITOR_KEY_CTRL_C EDITOR_CREATE_UPPER_KEY(2)
#define EDITOR_KEY_CTRL_DOWN EDITOR_CREATE_UPPER_KEY(3)
#define EDITOR_KEY_CTRL_END EDITOR_CREATE_UPPER_KEY(4)
#define EDITOR_KEY_CTRL_HOME EDITOR_CREATE_UPPER_KEY(5)
#define EDITOR_KEY_CTRL_LEFT EDITOR_CREATE_UPPER_KEY(6)
#define EDITOR_KEY_CTRL_PAGE_DOWN EDITOR_CREATE_UPPER_KEY(7)
#define EDITOR_KEY_CTRL_PAGE_UP EDITOR_CREATE_UPPER_KEY(8)
#define EDITOR_KEY_CTRL_RIGHT EDITOR_CREATE_UPPER_KEY(9)
#define EDITOR_KEY_CTRL_UP EDITOR_CREATE_UPPER_KEY(10)
#define EDITOR_KEY_DELETE EDITOR_CREATE_UPPER_KEY(11)
#define EDITOR_KEY_DOWN EDITOR_CREATE_UPPER_KEY(12)
#define EDITOR_KEY_END EDITOR_CREATE_UPPER_KEY(13)
#define EDITOR_KEY_ENTER EDITOR_CREATE_UPPER_KEY(14)
#define EDITOR_KEY_ESCAPE EDITOR_CREATE_UPPER_KEY(15)
#define EDITOR_KEY_HOME EDITOR_CREATE_UPPER_KEY(16)
#define EDITOR_KEY_LEFT EDITOR_CREATE_UPPER_KEY(17)
#define EDITOR_KEY_PAGE_DOWN EDITOR_CREATE_UPPER_KEY(18)
#define EDITOR_KEY_PAGE_UP EDITOR_CREATE_UPPER_KEY(19)
#define EDITOR_KEY_RIGHT EDITOR_CREATE_UPPER_KEY(20)
#define EDITOR_KEY_UP EDITOR_CREATE_UPPER_KEY(21)
#define EDITOR_LINE_FLAG_RENDER 1
#define EDITOR_SETTINGS_BG_COLOR "\x1b[48;2;24;25;21m"
#define EDITOR_SETTINGS_FG_COLOR "\x1b[38;2;191;191;189m"
#define EDITOR_TAB_SIZE 4
#define EDITOR_UI_BG_COLOR "\x1b[48;2;40;41;35m"
#define EDITOR_UI_BG_COLOR_HIGHLIGHT "\x1b[48;2;62;61;50m"
#define EDITOR_UI_DEFAULT_FG_COLOR "\x1b[38;2;255;255;255m"
#define EDITOR_UI_ERROR_BG_COLOR EDITOR_SETTINGS_BG_COLOR
#define EDITOR_UI_ERROR_BORDER_COLOR "\x1b[38;2;225;85;75m"
#define EDITOR_UI_ERROR_BUTTON_BG_COLOR "\x1b[48;2;44;45;41m"
#define EDITOR_UI_ERROR_BUTTON_FG_COLOR "\x1b[38;2;200;200;200m"
#define EDITOR_UI_ERROR_FG_COLOR "\x1b[38;2;230;230;225m"
#define EDITOR_UI_SELECT_SPACE "∙"
#define EDITOR_UI_START_STR "\n"EDITOR_UI_BG_COLOR"\x1b[0K\x1b[38;2;145;145;140m "
#define EDITOR_UI_START_STR_HIGHLIGHT "\n"EDITOR_UI_BG_COLOR_HIGHLIGHT"\x1b[0K\x1b[38;2;217;217;217m "



typedef struct __EDITOR_LINE{
	uint32_t l;
	char* dt;
	char* _bf;
	uint32_t _bfl;
	uint32_t _ws_c;
} editor_line_t;



typedef struct __EDITOR_STATE{
	uint32_t cx;
	uint32_t cy;
	uint32_t l_off;
	uint32_t w;
	uint32_t h;
	uint32_t _cx;
} editor_state_t;



typedef struct __EDITOR_SYNTAX{
	char nm[256];
	uint8_t nml;
} editor_syntax_t;



typedef struct __EDITOR_ERROR{
	char t[EDITOR_ERROR_WIDTH+1];
	uint8_t tl;
	char e[EDITOR_ERROR_HEIGHT][EDITOR_ERROR_WIDTH+1];
} editor_error_t;



typedef struct __EDITOR{
	char fp[4096];
	uint16_t fpl;
	uint8_t fl;
	editor_state_t st;
	uint16_t el;
	editor_error_t** e;
	uint16_t sl;
	editor_syntax_t** s;
	uint32_t ll;
	editor_line_t** l;
	uint8_t _ll_sz;
} editor_t;



void init_editor(editor_t* e);



void set_window_size(editor_t* e,uint32_t w,uint32_t h);



editor_syntax_t* create_syntax(editor_t* e);



editor_error_t* create_error(editor_t* e);



void load_syntax(editor_t* e,editor_syntax_t* s,const char* dt,uint32_t dtl);



void write_error_body(editor_error_t* e,const char* dt);



void open_file(editor_t* e,const char* fp);



void update_line(editor_t* e,uint32_t i,uint8_t fl);



void type_key(editor_t* e,uint16_t k);



void render_editor(editor_t* e);



void free_editor(editor_t* e);



#endif
