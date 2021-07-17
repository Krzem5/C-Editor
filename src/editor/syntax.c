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
	o->nm[0]=0;
	o->nml=0;
	*(e->s+e->sl-1)=o;
	return o;
}



void load_syntax(editor_t* e,editor_syntax_t* o,const char* dt,uint32_t dtl){
	if (dtl>=4&&*dt==0&&*(dt+1)=='S'&&*(dt+2)=='T'&&*(dt+3)=='X'){
		printf("Compiled: %s\n",dt);
	}
	else{
		json_object_t json;
		json_parser_state_t p=(json_parser_state_t)dt;
		if (_parse_json(&p,&json)){
			editor_error_t* err=create_error(e);
			err->tl=_copy_str(err->t,"JSON Error");
			err->t[err->tl]=0;
			printf("JSON Error: Offset %u\n",p-dt-1);
			return;
		}
		char dt_p[4096]="<root>";
		uint16_t dt_pi=0;
		while (dt_p[dt_pi]){
			dt_pi++;
		}
		if (json.t!=JSON_OBJECT_TYPE_MAP){
			editor_error_t* err=create_error(e);
			err->tl=_copy_str(err->t,"JSON Schema Error");
			err->t[err->tl]=0;
			char bf[4096]="Expected a Map:\n";
			uint32_t i=0;
			while (bf[i]){
				i++;
			}
			bf[i+_copy_str(bf+i,dt_p)]=0;
			write_error_body(err,bf);
			return;
		}
		json_object_t* nm=_get_by_key(&json,"name");
		if (!nm){
			editor_error_t* err=create_error(e);
			err->tl=_copy_str(err->t,"JSON Schema Error");
			err->t[err->tl]=0;
			char bf[4096]="Expected a \"name\" Property:\n";
			uint32_t i=0;
			while (bf[i]){
				i++;
			}
			bf[i+_copy_str(bf+i,dt_p)]=0;
			write_error_body(err,bf);
			return;
		}
		dt_p[dt_pi+_copy_str(dt_p+dt_pi,".name")]=0;
		if (nm->t!=JSON_OBJECT_TYPE_STRING){
			editor_error_t* err=create_error(e);
			err->tl=_copy_str(err->t,"JSON Schema Error");
			err->t[err->tl]=0;
			char bf[4096]="The \"name\" Property Must be a String:\n";
			uint32_t i=0;
			while (bf[i]){
				i++;
			}
			bf[i+_copy_str(bf+i,dt_p)]=0;
			write_error_body(err,bf);
			return;
		}
		if (nm->dt.s.l>UINT8_MAX){
			editor_error_t* err=create_error(e);
			err->tl=_copy_str(err->t,"JSON Schema Error");
			err->t[err->tl]=0;
			char bf[4096]="The \"name\" Property Must be an Less Than ";
			uint32_t i=0;
			while (bf[i]){
				i++;
			}
			WRITE_INT(UINT8_MAX,bf,i);
			i+=_copy_str(bf+i," Characters:\n");
			bf[i+_copy_str(bf+i,dt_p)]=0;
			write_error_body(err,bf);
			return;
		}
		o->nml=nm->dt.s.l;
		for (uint8_t i=0;i<o->nml;i++){
			o->nm[i]=*(nm->dt.s.v+i);
		}
		o->nm[o->nml]=0;
		json_object_t* el=_get_by_key(&json,"extensions");
		dt_p[dt_pi]=0;
		if (!el){
			editor_error_t* err=create_error(e);
			err->tl=_copy_str(err->t,"JSON Schema Error");
			err->t[err->tl]=0;
			char bf[4096]="Expected an \"extensions\" Property:\n";
			uint32_t i=0;
			while (bf[i]){
				i++;
			}
			bf[i+_copy_str(bf+i,dt_p)]=0;
			write_error_body(err,bf);
			return;
		}
		dt_p[dt_pi+_copy_str(dt_p+dt_pi,".extensions")]=0;
		if (el->t!=JSON_OBJECT_TYPE_ARRAY){
			editor_error_t* err=create_error(e);
			err->tl=_copy_str(err->t,"JSON Schema Error");
			err->t[err->tl]=0;
			char bf[4096]="The \"extensions\" Property Must be an Array:\n";
			uint32_t i=0;
			while (bf[i]){
				i++;
			}
			bf[i+_copy_str(bf+i,dt_p)]=0;
			write_error_body(err,bf);
			return;
		}
		_free_json(&json);
	}
}
