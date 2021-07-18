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
	e->f.fpl=0;
	while (*fp){
		e->f.fp[e->f.fpl]=*fp;
		e->f.fpl++;
		fp++;
	}
	e->f.fp[e->f.fpl]=0;
	for (uint16_t i=0;i<e->sl;i++){
		editor_syntax_t* s=*(e->s+i);
		if (!s->e){
			e->f.s=s;
		}
		else{
			for (uint8_t j=0;j<s->el;j++){
				editor_syntax_file_extension_t* ex=s->e+j;
				if (ex->l<=e->f.fpl){
					fp=e->f.fp+e->f.fpl-ex->l;
					for (uint8_t k=0;k<ex->l;k++){
						if (*(fp+k)!=*(ex->dt+k)){
							goto _check_next_extension;
						}
					}
					e->f.s=s;
					goto _break;
_check_next_extension:;
				}
			}
		}
	}
_break:;
	if (!e->f.s){
		editor_error_t* err=create_error(e);
		err->tl=_copy_str(err->t,"Syntax Error");
		err->t[err->tl]=0;
		char bf[4096]="Unable to Find a Syntax for File ";
		uint16_t i=0;
		while (bf[i]){
			i++;
		}
		bf[i+_copy_str(bf+i,fp)];
		write_error_body(err,bf);
		fclose(f);
		return;
	}
	e->fl|=EDITOR_FLAG_OPEN;
	fseek(f,0,SEEK_END);
	uint32_t sz=ftell(f);
	fseek(f,0,SEEK_SET);
	e->f.ll=1;
	e->f.l=malloc(sizeof(editor_line_t*));
	editor_line_t* l=malloc(sizeof(editor_line_t));
	l->l=0;
	l->dt=NULL;
	l->_bf=NULL;
	*(e->f.l)=l;
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
			e->f.ll++;
			e->f.l=realloc(e->f.l,e->f.ll*sizeof(editor_line_t*));
			l=malloc(sizeof(editor_line_t));
			l->l=0;
			l->dt=NULL;
			l->_bf=NULL;
			*(e->f.l+e->f.ll-1)=l;
		}
		else{
			l->l++;
			l->dt=realloc(l->dt,l->l*sizeof(char));
			*(l->dt+l->l-1)=c;
		}
	}
	fclose(f);
	e->f._ll_sz=0;
	uint32_t v=e->f.ll;
	while (v){
		e->f._ll_sz++;
		v/=10;
	}
	for (uint32_t i=0;i<e->f.ll;i++){
		update_line(e,i,1);
	}
	printf("\x1b]0;%s - Editor\x07",e->f.fp);
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
		if (e->f.cy){
			e->f.cy--;
			if (e->f.l_off>e->f.cy){
				e->f.l_off--;
			}
			update_line(e,e->f.cy,EDITOR_LINE_FLAG_RENDER|EDITOR_LINE_FLAG_UPDATE_CURSOR_POS);
			update_line(e,e->f.cy+1,EDITOR_LINE_FLAG_RENDER);
		}
	}
	else if (k==EDITOR_KEY_DOWN){
		if (e->f.cy<e->f.ll-1){
			e->f.cy++;
			if (e->f.l_off+e->h-2<=e->f.cy){
				e->f.l_off++;
			}
			update_line(e,e->f.cy-1,EDITOR_LINE_FLAG_RENDER);
			update_line(e,e->f.cy,EDITOR_LINE_FLAG_RENDER|EDITOR_LINE_FLAG_UPDATE_CURSOR_POS);
		}
	}
	else if (k==EDITOR_KEY_RIGHT){
		if (e->f.cx<(*(e->f.l+e->f.cy))->l){
			e->f.cx++;
			update_line(e,e->f.cy,0);
		}
		else if (e->f.cy<e->f.ll-1){
			e->f.cx=0;
			e->f.cy++;
			if (e->f.l_off+e->h-2<=e->f.cy){
				e->f.l_off++;
			}
			update_line(e,e->f.cy-1,EDITOR_LINE_FLAG_RENDER);
			update_line(e,e->f.cy,EDITOR_LINE_FLAG_RENDER);
		}
	}
	else if (k==EDITOR_KEY_LEFT){
		if (e->f.cx){
			e->f.cx--;
			update_line(e,e->f.cy,0);
		}
		else if (e->f.cy){
			e->f.cy--;
			e->f.cx=(*(e->f.l+e->f.cy))->l;
			if (e->f.l_off>e->f.cy){
				e->f.l_off--;
			}
			update_line(e,e->f.cy,EDITOR_LINE_FLAG_RENDER);
			update_line(e,e->f.cy+1,EDITOR_LINE_FLAG_RENDER);
		}
	}
	else if (k==EDITOR_KEY_HOME){
		uint32_t ws_c=(*(e->f.l+e->f.cy))->_ws_c;
		if (!ws_c){
			if (e->f.cx){
				e->f.cx=0;
				update_line(e,e->f.cy,0);
			}
		}
		else{
			e->f.cx=(e->f.cx==ws_c?0:ws_c);
			update_line(e,e->f.cy,0);
		}
	}
	else if (k==EDITOR_KEY_END){
		uint32_t l=(*(e->f.l+e->f.cy))->l;
		if (e->f.cx<l){
			e->f.cx=l;
			update_line(e,e->f.cy,0);
		}
	}
	else if (k==EDITOR_KEY_CTRL_UP){
		if (e->f.l_off){
			e->f.l_off--;
			if (e->f.l_off<e->f.cy){
				e->f.cy--;
				update_line(e,e->f.cy,EDITOR_LINE_FLAG_RENDER);
				update_line(e,e->f.cy+1,EDITOR_LINE_FLAG_RENDER);
			}
		}
		else if (e->f.cy){
			e->f.cy--;
			update_line(e,e->f.cy,EDITOR_LINE_FLAG_RENDER);
			update_line(e,e->f.cy+1,EDITOR_LINE_FLAG_RENDER);
		}
	}
	else if (k==EDITOR_KEY_CTRL_DOWN){
		if (e->f.l_off+e->h-2<e->f.ll){
			e->f.l_off++;
			if (e->f.l_off+e->h-2>e->f.cy){
				e->f.cy++;
				update_line(e,e->f.cy-1,EDITOR_LINE_FLAG_RENDER);
				update_line(e,e->f.cy,EDITOR_LINE_FLAG_RENDER);
			}
		}
		else if (e->f.cy<e->f.ll-1){
			e->f.cy++;
			update_line(e,e->f.cy-1,EDITOR_LINE_FLAG_RENDER);
			update_line(e,e->f.cy,EDITOR_LINE_FLAG_RENDER);
		}
	}
	else if (k==EDITOR_KEY_CTRL_HOME){
		if (e->f.cy){
			uint32_t l_cy=e->f.cy;
			e->f.cy=0;
			e->f.l_off=0;
			update_line(e,0,EDITOR_LINE_FLAG_RENDER);
			update_line(e,l_cy,EDITOR_LINE_FLAG_RENDER);
		}
	}
	else if (k==EDITOR_KEY_CTRL_END){
		if (e->f.cy<e->f.ll-1){
			uint32_t l_cy=e->f.cy;
			e->f.cy=e->f.ll-1;
			e->f.l_off=e->f.ll-e->h+2;
			update_line(e,l_cy,EDITOR_LINE_FLAG_RENDER);
			update_line(e,e->f.cy,EDITOR_LINE_FLAG_RENDER);
		}
	}
}
