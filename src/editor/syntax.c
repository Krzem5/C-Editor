#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS 1
#endif
#include <editor.h>
#include <_editor_internal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>



editor_syntax_t* create_syntax(editor_t* e){
	e->sl++;
	e->s=realloc(e->s,e->sl*sizeof(editor_syntax_t*));
	editor_syntax_t* o=malloc(sizeof(editor_syntax_t));
	*(e->s+e->sl-1)=o;
	return o;
}



void load_syntax(editor_syntax_t* s,const char* dt,uint32_t dtl){
	if (dtl>=4&&*dt==0&&*(dt+1)=='S'&&*(dt+2)=='T'&&*(dt+3)=='X'){
		printf("Compiled: %s\n",dt);
	}
	else{
		printf("JSON: %s\n",dt);
	}
	getchar();
}
