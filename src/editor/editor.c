#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS 1
#endif
#include <editor.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>



void init_editor(editor_t* e){
	e->fp[0]=0;
	e->fpl=0;
	e->fl=0;
	e->cx=0;
	e->cy=0;
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
	*(e->l)=l;
	for (uint32_t i=0;i<sz;i++){
		uint8_t c=fgetc(f);
		if (c!='\r'||(c=fgetc(f))!='\n'){
			l->l++;
			l->dt=realloc(l->dt,l->l*sizeof(char));
			*(l->dt+l->l-1)=(c=='\n'?'\r':c);
		}
		else{
			e->ll++;
			e->l=realloc(e->l,e->ll*sizeof(editor_line_t*));
			l=malloc(sizeof(editor_line_t));
			l->l=0;
			l->dt=NULL;
			*(e->l+e->ll-1)=l;
		}
	}
	fclose(f);
}



void type_key(editor_t* e,uint16_t k){
	if (k==EDITOR_KEY_CTRL_C){
		e->fl&=~EDITOR_FLAG_OPEN;
	}
}



void render_editor(editor_t* e){
	printf("\x1b[H%s\n",e->fp);
	for (uint32_t i=0;i<e->ll;i++){
		editor_line_t* l=*(e->l+i);
		printf("%u: %s\n",i,l->dt);
	}
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

