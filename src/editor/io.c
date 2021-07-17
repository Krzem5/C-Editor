#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS 1
#endif
#include <editor.h>
#include <_editor_internal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>



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
		update_line(e,i,1);
	}
	printf("\x1b]0;%s - Editor\x07",e->fp);
}



void type_key(editor_t* e,uint16_t k){
	if (e->el){
		if (k==EDITOR_KEY_ENTER){
			free(*(e->e+e->el-1));
			e->el--;
			if (!e->el){
				free(e->e);
				e->e=NULL;
			}
			else{
				e->e=realloc(e->e,e->el*sizeof(editor_error_t*));
			}
		}
		return;
	}
	if (k==EDITOR_KEY_CTRL_C){
		e->fl&=~EDITOR_FLAG_OPEN;
	}
	else if (k==EDITOR_KEY_UP){
		if (e->st.cy){
			e->st.cy--;
			if (e->st.l_off>e->st.cy){
				e->st.l_off--;
			}
			update_line(e,e->st.cy,EDITOR_LINE_FLAG_RENDER|EDITOR_LINE_FLAG_UPDATE_CURSOR_POS);
			update_line(e,e->st.cy+1,EDITOR_LINE_FLAG_RENDER);
		}
	}
	else if (k==EDITOR_KEY_DOWN){
		if (e->st.cy<e->ll-1){
			e->st.cy++;
			if (e->st.l_off+e->st.h-2<=e->st.cy){
				e->st.l_off++;
			}
			update_line(e,e->st.cy-1,EDITOR_LINE_FLAG_RENDER);
			update_line(e,e->st.cy,EDITOR_LINE_FLAG_RENDER|EDITOR_LINE_FLAG_UPDATE_CURSOR_POS);
		}
	}
	else if (k==EDITOR_KEY_RIGHT){
		if (e->st.cx<(*(e->l+e->st.cy))->l){
			e->st.cx++;
			update_line(e,e->st.cy,0);
		}
		else if (e->st.cy<e->ll-1){
			e->st.cx=0;
			e->st.cy++;
			if (e->st.l_off+e->st.h-2<=e->st.cy){
				e->st.l_off++;
			}
			update_line(e,e->st.cy-1,EDITOR_LINE_FLAG_RENDER);
			update_line(e,e->st.cy,EDITOR_LINE_FLAG_RENDER);
		}
	}
	else if (k==EDITOR_KEY_LEFT){
		if (e->st.cx){
			e->st.cx--;
			update_line(e,e->st.cy,0);
		}
		else if (e->st.cy){
			e->st.cy--;
			e->st.cx=(*(e->l+e->st.cy))->l;
			if (e->st.l_off>e->st.cy){
				e->st.l_off--;
			}
			update_line(e,e->st.cy,EDITOR_LINE_FLAG_RENDER);
			update_line(e,e->st.cy+1,EDITOR_LINE_FLAG_RENDER);
		}
	}
	else if (k==EDITOR_KEY_HOME){
		uint32_t ws_c=(*(e->l+e->st.cy))->_ws_c;
		if (!ws_c){
			if (e->st.cx){
				e->st.cx=0;
				update_line(e,e->st.cy,0);
			}
		}
		else{
			e->st.cx=(e->st.cx==ws_c?0:ws_c);
			update_line(e,e->st.cy,0);
		}
	}
	else if (k==EDITOR_KEY_END){
		uint32_t l=(*(e->l+e->st.cy))->l;
		if (e->st.cx<l){
			e->st.cx=l;
			update_line(e,e->st.cy,0);
		}
	}
	else if (k==EDITOR_KEY_CTRL_UP){
		if (e->st.l_off){
			e->st.l_off--;
			if (e->st.l_off<e->st.cy){
				e->st.cy--;
				update_line(e,e->st.cy,EDITOR_LINE_FLAG_RENDER);
				update_line(e,e->st.cy+1,EDITOR_LINE_FLAG_RENDER);
			}
		}
		else if (e->st.cy){
			e->st.cy--;
			update_line(e,e->st.cy,EDITOR_LINE_FLAG_RENDER);
			update_line(e,e->st.cy+1,EDITOR_LINE_FLAG_RENDER);
		}
	}
	else if (k==EDITOR_KEY_CTRL_DOWN){
		if (e->st.l_off+e->st.h-2<e->ll){
			e->st.l_off++;
			if (e->st.l_off+e->st.h-2>e->st.cy){
				e->st.cy++;
				update_line(e,e->st.cy-1,EDITOR_LINE_FLAG_RENDER);
				update_line(e,e->st.cy,EDITOR_LINE_FLAG_RENDER);
			}
		}
		else if (e->st.cy<e->ll-1){
			e->st.cy++;
			update_line(e,e->st.cy-1,EDITOR_LINE_FLAG_RENDER);
			update_line(e,e->st.cy,EDITOR_LINE_FLAG_RENDER);
		}
	}
	else if (k==EDITOR_KEY_CTRL_HOME){
		if (e->st.cy){
			uint32_t l_cy=e->st.cy;
			e->st.cy=0;
			e->st.l_off=0;
			update_line(e,0,EDITOR_LINE_FLAG_RENDER);
			update_line(e,l_cy,EDITOR_LINE_FLAG_RENDER);
		}
	}
	else if (k==EDITOR_KEY_CTRL_END){
		if (e->st.cy<e->ll-1){
			uint32_t l_cy=e->st.cy;
			e->st.cy=e->ll-1;
			e->st.l_off=e->ll-e->st.h+2;
			update_line(e,l_cy,EDITOR_LINE_FLAG_RENDER);
			update_line(e,e->st.cy,EDITOR_LINE_FLAG_RENDER);
		}
	}
}
