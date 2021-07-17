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
