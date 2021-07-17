#include <editor.h>
#include <_editor_internal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>



void init_editor(editor_t* e){
	e->fp[0]=0;
	e->fpl=0;
	e->fl=0;
	e->st.cx=0;
	e->st.cy=0;
	e->st.l_off=0;
	e->w=0;
	e->h=0;
	e->sl=0;
	e->s=NULL;
	e->ll=0;
	e->l=NULL;
}



void set_window_size(editor_t* e,uint32_t w,uint32_t h){
	e->w=w;
	e->h=h;
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
			update_line(e,e->st.cy,EDITOR_LINE_FLAG_RENDER|_EDITOR_LINE_FLAG_UPDATE_CURSOR_POS);
			update_line(e,e->st.cy+1,EDITOR_LINE_FLAG_RENDER);
		}
	}
	else if (k==EDITOR_KEY_DOWN){
		if (e->st.cy<e->ll-1){
			e->st.cy++;
			if (e->st.l_off+e->h-2<=e->st.cy){
				e->st.l_off++;
			}
			update_line(e,e->st.cy-1,EDITOR_LINE_FLAG_RENDER);
			update_line(e,e->st.cy,EDITOR_LINE_FLAG_RENDER|_EDITOR_LINE_FLAG_UPDATE_CURSOR_POS);
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
			if (e->st.l_off+e->h-2<=e->st.cy){
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
		if (e->st.l_off+e->h-2<e->ll){
			e->st.l_off++;
			if (e->st.l_off+e->h-2>e->st.cy){
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
			e->st.l_off=e->ll-e->h+2;
			update_line(e,l_cy,EDITOR_LINE_FLAG_RENDER);
			update_line(e,e->st.cy,EDITOR_LINE_FLAG_RENDER);
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
	for (uint32_t i=0;i<e->sl;i++){
		editor_syntax_t* s=*(e->s+i);
		free(s);
	}
	if (e->s){
		free(e->s);
	}
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
