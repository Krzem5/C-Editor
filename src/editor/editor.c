#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS 1
#endif
#include <editor.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>



#define STR_LEN(x) (sizeof((x))/sizeof(char)-1)
#define _EDITOR_LINE_FLAG_UPDATE_CURSOR_POS 2
#define _EDITOR_LINE_FLAG_HAS_CURSOR 4
#define _EDITOR_LINE_FLAG_IN_WHITESPACE 8



uint32_t _copy_str(char* d,const char* s){
	uint32_t o=0;
	while (*(s+o)){
		*(d+o)=*(s+o);
		o++;
	}
	return o;
}



void init_editor(editor_t* e){
	e->fp[0]=0;
	e->fpl=0;
	e->fl=0;
	e->st.cx=0;
	e->st.cy=0;
	e->st.l_off=0;
	e->w=0;
	e->h=0;
	e->ll=0;
	e->l=NULL;
}



void set_window_size(editor_t* e,uint32_t w,uint32_t h){
	e->w=w;
	e->h=h;
}



void open_file(editor_t* e,const char* fp){
	FILE* f=fopen(fp,"rb");
	e->fpl=0;
	while (*fp){
		e->fp[e->fpl]=*fp;
		e->fpl++;
		fp++;
	}
	e->fp[e->fpl]=0;
	e->fl|=EDITOR_FLAG_OPEN;
	fseek(f,0,SEEK_END);
	uint32_t sz=ftell(f);
	fseek(f,0,SEEK_SET);
	e->ll=1;
	e->l=malloc(sizeof(editor_line_t*));
	editor_line_t* l=malloc(sizeof(editor_line_t));
	l->l=0;
	l->dt=NULL;
	l->_bf=NULL;
	*(e->l)=l;
	for (uint32_t i=0;i<sz;i++){
		uint8_t c=fgetc(f);
		if (c=='\r'){
			c=fgetc(f);
			i++;
			if (c!='\n'){
				l->l++;
				l->dt=realloc(l->dt,l->l*sizeof(char));
				*(l->dt+l->l-1)='\r';
			}
			else{
				goto _write_newline;
			}
		}
		else if (c=='\n'){
_write_newline:
			e->ll++;
			e->l=realloc(e->l,e->ll*sizeof(editor_line_t*));
			l=malloc(sizeof(editor_line_t));
			l->l=0;
			l->dt=NULL;
			l->_bf=NULL;
			*(e->l+e->ll-1)=l;
		}
		else{
			l->l++;
			l->dt=realloc(l->dt,l->l*sizeof(char));
			*(l->dt+l->l-1)=c;
		}
	}
	fclose(f);
	e->_ll_sz=0;
	uint32_t v=e->ll;
	while (v){
		e->_ll_sz++;
		v/=10;
	}
	for (uint32_t i=0;i<e->ll;i++){
		render_line(e,i,1);
	}
	printf("\x1b]0;%s - Editor\x07",e->fp);
}



void render_line(editor_t* e,uint32_t i,uint8_t fl){
	editor_line_t* l=*(e->l+i);
	if (fl&EDITOR_LINE_FLAG_RENDER){
		if (l->_bf){
			free(l->_bf);
		}
		l->_bfl=e->_ll_sz+l->l+(i==e->st.cy?STR_LEN(EDITOR_UI_START_STR_HIGHLIGHT):STR_LEN(EDITOR_UI_START_STR))+STR_LEN(" "EDITOR_UI_DEFAULT_FG_COLOR);
		l->_ws_c=0;
	}
	uint32_t j=0;
	uint32_t n=0;
	fl|=(e->st.cy==i&&!(fl&_EDITOR_LINE_FLAG_UPDATE_CURSOR_POS)?_EDITOR_LINE_FLAG_HAS_CURSOR:0)|(fl&EDITOR_LINE_FLAG_RENDER?_EDITOR_LINE_FLAG_IN_WHITESPACE:0);
	for (;j<l->l;j++){
		uint8_t c=*(l->dt+j);
		if ((fl&_EDITOR_LINE_FLAG_UPDATE_CURSOR_POS)&&n>=e->st._cx){
			fl&=~_EDITOR_LINE_FLAG_UPDATE_CURSOR_POS;
			e->st.cx=j;
			e->st._cx=n;
		}
		else if ((fl&_EDITOR_LINE_FLAG_HAS_CURSOR)&&e->st.cx==j){
			fl&=~_EDITOR_LINE_FLAG_HAS_CURSOR;
			e->st._cx=n;
		}
		if ((fl&_EDITOR_LINE_FLAG_IN_WHITESPACE)&&(c==' '||c=='\t'||c=='\f'||c=='\v')){
			l->_ws_c++;
		}
		else{
			fl&=~_EDITOR_LINE_FLAG_IN_WHITESPACE;
		}
		if (c=='\t'){
			if (fl&EDITOR_LINE_FLAG_RENDER){
				l->_bfl+=EDITOR_TAB_SIZE-(n%EDITOR_TAB_SIZE)-1;
			}
			n+=EDITOR_TAB_SIZE-(n%EDITOR_TAB_SIZE);
		}
		else if (c<31||c>126){
			if (fl&EDITOR_LINE_FLAG_RENDER){
				l->_bfl+=5;
			}
			n++;
		}
		else{
			n++;
		}
	}
	if (fl&_EDITOR_LINE_FLAG_UPDATE_CURSOR_POS){
		e->st.cx=l->l;
		e->st._cx=n;
	}
	if (fl&_EDITOR_LINE_FLAG_HAS_CURSOR){
		e->st._cx=n;
	}
	if (!(fl&EDITOR_LINE_FLAG_RENDER)){
		return;
	}
	l->_bf=malloc(l->_bfl*sizeof(char));
	uint32_t k=_copy_str(l->_bf,(i==e->st.cy?EDITOR_UI_START_STR_HIGHLIGHT:EDITOR_UI_START_STR));
	char bf[10];
	j=0;
	uint32_t v=i+1;
	while (v){
		bf[j]=(v%10)+48;
		j++;
		v/=10;
	}
	while (j<e->_ll_sz){
		bf[j]=' ';
		j++;
	}
	while (j){
		j--;
		*(l->_bf+k)=bf[j];
		k++;
	}
	k+=_copy_str(l->_bf+k," "EDITOR_UI_DEFAULT_FG_COLOR);
	n=0;
	for (j=0;j<l->l;j++){
		uint8_t c=*(l->dt+j);
		if (c>31&&c<127){
			*(l->_bf+k)=c;
			k++;
			n++;
		}
		else if (c=='\t'){
			for (uint8_t m=0;m<EDITOR_TAB_SIZE-(n%EDITOR_TAB_SIZE);m++){
				*(l->_bf+k)=' ';
				k++;
			}
			n+=EDITOR_TAB_SIZE-(n%EDITOR_TAB_SIZE);
		}
		else{
			*(l->_bf+k)='<';
			*(l->_bf+k+1)='0';
			*(l->_bf+k+2)='x';
			*(l->_bf+k+3)=(c>>4)+((c>>4)>9?87:48);
			*(l->_bf+k+4)=(c&15)+((c&15)>9?87:48);
			*(l->_bf+k+5)='>';
			k+=6;
			n++;
		}
	}
}



void type_key(editor_t* e,uint16_t k){
	if (k==EDITOR_KEY_CTRL_C){
		e->fl&=~EDITOR_FLAG_OPEN;
	}
	else if (k==EDITOR_KEY_UP){
		if (e->st.cy){
			e->st.cy--;
			if (e->st.l_off>e->st.cy){
				e->st.l_off--;
			}
			render_line(e,e->st.cy,EDITOR_LINE_FLAG_RENDER|_EDITOR_LINE_FLAG_UPDATE_CURSOR_POS);
			render_line(e,e->st.cy+1,EDITOR_LINE_FLAG_RENDER);
		}
	}
	else if (k==EDITOR_KEY_DOWN){
		if (e->st.cy<e->ll-1){
			e->st.cy++;
			if (e->st.l_off+e->h-2<=e->st.cy){
				e->st.l_off++;
			}
			render_line(e,e->st.cy-1,EDITOR_LINE_FLAG_RENDER);
			render_line(e,e->st.cy,EDITOR_LINE_FLAG_RENDER|_EDITOR_LINE_FLAG_UPDATE_CURSOR_POS);
		}
	}
	else if (k==EDITOR_KEY_RIGHT){
		if (e->st.cx<(*(e->l+e->st.cy))->l){
			e->st.cx++;
			render_line(e,e->st.cy,0);
		}
		else if (e->st.cy<e->ll-1){
			e->st.cx=0;
			e->st.cy++;
			if (e->st.l_off+e->h-2<=e->st.cy){
				e->st.l_off++;
			}
			render_line(e,e->st.cy-1,EDITOR_LINE_FLAG_RENDER);
			render_line(e,e->st.cy,EDITOR_LINE_FLAG_RENDER);
		}
	}
	else if (k==EDITOR_KEY_LEFT){
		if (e->st.cx){
			e->st.cx--;
			render_line(e,e->st.cy,0);
		}
		else if (e->st.cy){
			e->st.cy--;
			e->st.cx=(*(e->l+e->st.cy))->l;
			if (e->st.l_off>e->st.cy){
				e->st.l_off--;
			}
			render_line(e,e->st.cy,EDITOR_LINE_FLAG_RENDER);
			render_line(e,e->st.cy+1,EDITOR_LINE_FLAG_RENDER);
		}
	}
	else if (k==EDITOR_KEY_HOME){
		uint32_t ws_c=(*(e->l+e->st.cy))->_ws_c;
		if (!ws_c){
			if (e->st.cx){
				e->st.cx=0;
				render_line(e,e->st.cy,0);
			}
		}
		else{
			e->st.cx=(e->st.cx==ws_c?0:ws_c);
			render_line(e,e->st.cy,0);
		}
	}
	else if (k==EDITOR_KEY_END){
		uint32_t l=(*(e->l+e->st.cy))->l;
		if (e->st.cx<l){
			e->st.cx=l;
			render_line(e,e->st.cy,0);
		}
	}
	else if (k==EDITOR_KEY_CTRL_UP){
		if (e->st.l_off){
			e->st.l_off--;
			if (e->st.l_off<e->st.cy){
				e->st.cy--;
				render_line(e,e->st.cy,EDITOR_LINE_FLAG_RENDER);
				render_line(e,e->st.cy+1,EDITOR_LINE_FLAG_RENDER);
			}
		}
		else if (e->st.cy){
			e->st.cy--;
			render_line(e,e->st.cy,EDITOR_LINE_FLAG_RENDER);
			render_line(e,e->st.cy+1,EDITOR_LINE_FLAG_RENDER);
		}
	}
	else if (k==EDITOR_KEY_CTRL_DOWN){
		if (e->st.l_off+e->h-2<e->ll){
			e->st.l_off++;
			if (e->st.l_off+e->h-2>e->st.cy){
				e->st.cy++;
				render_line(e,e->st.cy-1,EDITOR_LINE_FLAG_RENDER);
				render_line(e,e->st.cy,EDITOR_LINE_FLAG_RENDER);
			}
		}
		else if (e->st.cy<e->ll-1){
			e->st.cy++;
			render_line(e,e->st.cy-1,EDITOR_LINE_FLAG_RENDER);
			render_line(e,e->st.cy,EDITOR_LINE_FLAG_RENDER);
		}
	}
	else if (k==EDITOR_KEY_CTRL_HOME){
		if (e->st.cy){
			uint32_t l_cy=e->st.cy;
			e->st.cy=0;
			e->st.l_off=0;
			render_line(e,0,EDITOR_LINE_FLAG_RENDER);
			render_line(e,l_cy,EDITOR_LINE_FLAG_RENDER);
		}
	}
	else if (k==EDITOR_KEY_CTRL_END){
		if (e->st.cy<e->ll-1){
			uint32_t l_cy=e->st.cy;
			e->st.cy=e->ll-1;
			e->st.l_off=e->ll-e->h+2;
			render_line(e,l_cy,EDITOR_LINE_FLAG_RENDER);
			render_line(e,e->st.cy,EDITOR_LINE_FLAG_RENDER);
		}
	}
}



void render_editor(editor_t* e){
	char* bf=malloc((STR_LEN("\x1b[?12l\x1b[H"EDITOR_SETTINGS_BG_COLOR"\x1b[0K")+e->fpl)*sizeof(char));
	uint32_t sz=_copy_str(bf,"\x1b[?12l\x1b[H"EDITOR_SETTINGS_BG_COLOR"\x1b[0K");
	sz+=_copy_str(bf+sz,e->fp);
	uint32_t i=0;
	for (;i<(e->ll<e->h-2?e->ll:e->h-2);i++){
		editor_line_t* l=*(e->l+e->st.l_off+i);
		bf=realloc(bf,(sz+l->_bfl)*sizeof(char));
		for (uint32_t j=0;j<l->_bfl;j++){
			*(bf+sz)=*(l->_bf+j);
			sz++;
		}
	}
	for (;i<e->h-2;i++){
		bf=realloc(bf,(sz+STR_LEN("\n"EDITOR_UI_BG_COLOR"\x1b[0K"))*sizeof(char));
		sz+=_copy_str(bf+sz,"\n"EDITOR_UI_BG_COLOR"\x1b[0K");
	}
	char cl_str[10];
	uint8_t cl_l=0;
	uint32_t v=e->st.cx+1;
	while (v){
		cl_str[cl_l]=(v%10)+48;
		cl_l++;
		v/=10;
	}
	char ln_str[10];
	uint8_t ln_l=0;
	v=e->st.cy+1;
	while (v){
		ln_str[ln_l]=(v%10)+48;
		ln_l++;
		v/=10;
	}
	char cx_str[10];
	uint8_t cx_l=0;
	v=e->st._cx+e->_ll_sz+3;
	while (v){
		cx_str[cx_l]=(v%10)+48;
		cx_l++;
		v/=10;
	}
	char cy_str[10];
	uint8_t cy_l=0;
	v=e->st.cy-e->st.l_off+2;
	while (v){
		cy_str[cy_l]=(v%10)+48;
		cy_l++;
		v/=10;
	}
	bf=realloc(bf,(sz+STR_LEN("\n"EDITOR_SETTINGS_FG_COLOR EDITOR_SETTINGS_BG_COLOR"\x1b[0KLine , Column \x1b[0m\x1b[;H")+cl_l+ln_l+cx_l+cy_l));
	sz+=_copy_str(bf+sz,"\n"EDITOR_SETTINGS_FG_COLOR EDITOR_SETTINGS_BG_COLOR"\x1b[0KLine ");
	while (ln_l){
		ln_l--;
		*(bf+sz)=ln_str[ln_l];
		sz++;
	}
	sz+=_copy_str(bf+sz,", Column ");
	while (cl_l){
		cl_l--;
		*(bf+sz)=cl_str[cl_l];
		sz++;
	}
	sz+=_copy_str(bf+sz,"\x1b[0m\x1b[");
	while (cy_l){
		cy_l--;
		*(bf+sz)=cy_str[cy_l];
		sz++;
	}
	*(bf+sz)=';';
	sz++;
	while (cx_l){
		cx_l--;
		*(bf+sz)=cx_str[cx_l];
		sz++;
	}
	*(bf+sz)='H';
	fwrite(bf,sizeof(char),sz+1,stdout);
	free(bf);
}


void free_editor(editor_t* e){
	for (uint32_t i=0;i<e->ll;i++){
		editor_line_t* l=*(e->l+i);
		if (l->dt){
			free(l->dt);
		}
		free(l);
	}
	if (e->l){
		free(e->l);
	}
}
