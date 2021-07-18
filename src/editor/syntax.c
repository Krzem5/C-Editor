#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS 1
#endif
#include <editor.h>
#include <_editor_internal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>



const char* SYNTAX_SCOPE_NAME_LIST[]={
	NULL,
	"accessor",
	"annotation",
	"builtin_type",
	"builtin_variable",
	"comment",
	"constant",
	"definition_keyword",
	"escape",
	"function",
	"function_keyword",
	"invalid",
	"keyword",
	"label",
	"modifier",
	"number",
	"operator",
	"parameter",
	"placeholder",
	"regex",
	"separator",
	"string",
	"tag",
	"text",
	"title",
	"unquoted_string",
	"variable"
};



editor_syntax_t* create_syntax(editor_t* e){
	e->sl++;
	e->s=realloc(e->s,e->sl*sizeof(editor_syntax_t*));
	editor_syntax_t* o=malloc(sizeof(editor_syntax_t));
	o->nm[0]=0;
	o->nml=0;
	o->e=NULL;
	o->el=0;
	o->b_sc.dt[EDITOR_SYNTAX_UNKNOWN_SCOPE_INDEX]=0;
	o->cl=0;
	o->c=NULL;
	*(e->s+e->sl-1)=o;
	return o;
}



void load_syntax(editor_t* e,editor_syntax_t* o,const char* dt,uint32_t dtl,const char* fp){
	const char* f_nm=fp;
	while (*fp){
		if (*fp=='/'||*fp=='\\'){
			f_nm=fp+1;
		}
		fp++;
	}
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
			char bf[4096]="Invalid Character:\n";
			uint16_t i=0;
			while (bf[i]){
				i++;
			}
			i+=_copy_str(bf+i,f_nm);
			bf[i]=':';
			i++;
			WRITE_INT(p-dt-1,bf,i);
			bf[i]=0;
			write_error_body(err,bf);
			return;
		}
		if (json.t!=JSON_OBJECT_TYPE_MAP){
			editor_error_t* err=create_error(e);
			err->tl=_copy_str(err->t,"JSON Schema Error");
			err->t[err->tl]=0;
			char bf[4096]="Expected a Map:\n";
			uint16_t i=0;
			while (bf[i]){
				i++;
			}
			i+=_copy_str(bf+i,f_nm);
			bf[i+_copy_str(bf+i,":<root>")]=0;
			write_error_body(err,bf);
			_free_json(&json);
			return;
		}
		json_object_t* nm=_get_by_key(&json,"name");
		if (!nm){
			editor_error_t* err=create_error(e);
			err->tl=_copy_str(err->t,"JSON Schema Error");
			err->t[err->tl]=0;
			char bf[4096]="Expected a \"name\" Property:\n";
			uint16_t i=0;
			while (bf[i]){
				i++;
			}
			i+=_copy_str(bf+i,f_nm);
			bf[i+_copy_str(bf+i,":<root>")]=0;
			write_error_body(err,bf);
			_free_json(&json);
			return;
		}
		if (nm->t!=JSON_OBJECT_TYPE_STRING){
			editor_error_t* err=create_error(e);
			err->tl=_copy_str(err->t,"JSON Schema Error");
			err->t[err->tl]=0;
			char bf[4096]="The \"name\" Property Must be a String:\n";
			uint16_t i=0;
			while (bf[i]){
				i++;
			}
			i+=_copy_str(bf+i,f_nm);
			bf[i+_copy_str(bf+i,":<root>.name")]=0;
			write_error_body(err,bf);
			_free_json(&json);
			return;
		}
		if (nm->dt.s.l>EDITOR_SYNTAX_NAME_LENGTH){
			editor_error_t* err=create_error(e);
			err->tl=_copy_str(err->t,"JSON Schema Error");
			err->t[err->tl]=0;
			char bf[4096]="The \"name\" Property Must be No More Than ";
			uint32_t i=0;
			while (bf[i]){
				i++;
			}
			WRITE_INT(EDITOR_SYNTAX_NAME_LENGTH,bf,i);
			i+=_copy_str(bf+i," Characters:\n");
			i+=_copy_str(bf+i,f_nm);
			bf[i+_copy_str(bf+i,":<root>.name")]=0;
			write_error_body(err,bf);
			_free_json(&json);
			return;
		}
		o->nml=nm->dt.s.l;
		for (uint8_t i=0;i<o->nml;i++){
			o->nm[i]=*(nm->dt.s.v+i);
		}
		o->nm[o->nml]=0;
		json_object_t* el=_get_by_key(&json,"extensions");
		if (!el){
			editor_error_t* err=create_error(e);
			err->tl=_copy_str(err->t,"JSON Schema Error");
			err->t[err->tl]=0;
			char bf[4096]="Expected an \"extensions\" Property:\n";
			uint16_t i=0;
			while (bf[i]){
				i++;
			}
			i+=_copy_str(bf+i,f_nm);
			bf[i+_copy_str(bf+i,":<root>")]=0;
			write_error_body(err,bf);
			_free_json(&json);
			return;
		}
		if (el->t==JSON_OBJECT_TYPE_NULL){
			o->e=NULL;
			o->el=0;
		}
		else{
			if (el->t!=JSON_OBJECT_TYPE_ARRAY){
				editor_error_t* err=create_error(e);
				err->tl=_copy_str(err->t,"JSON Schema Error");
				err->t[err->tl]=0;
				char bf[4096]="The \"extensions\" Property Must be an Array:\n";
				uint16_t i=0;
				while (bf[i]){
					i++;
				}
				i+=_copy_str(bf+i,f_nm);
				bf[i+_copy_str(bf+i,":<root>.extensions")]=0;
				write_error_body(err,bf);
				_free_json(&json);
				return;
			}
			o->e=malloc(el->dt.a.l*sizeof(editor_syntax_file_extension_t));
			o->el=el->dt.a.l;
			for (uint32_t i=0;i<el->dt.a.l;i++){
				json_object_t* ex=el->dt.a.dt+i;
				if (ex->t!=JSON_OBJECT_TYPE_STRING){
					editor_error_t* err=create_error(e);
					err->tl=_copy_str(err->t,"JSON Schema Error");
					err->t[err->tl]=0;
					char bf[4096]="An Extension Must be a String:\n";
					uint32_t j=0;
					while (bf[j]){
						j++;
					}
					j+=_copy_str(bf+j,f_nm);
					j+=_copy_str(bf+j,":<root>.extensions[");
					WRITE_INT(i,bf,j);
					bf[j]=']';
					bf[j+1]=0;
					write_error_body(err,bf);
					_free_json(&json);
					return;
				}
				if (ex->dt.s.l>UINT8_MAX){
					editor_error_t* err=create_error(e);
					err->tl=_copy_str(err->t,"JSON Schema Error");
					err->t[err->tl]=0;
					char bf[4096]="An Extension Must be No More Than ";
					uint32_t j=0;
					while (bf[j]){
						j++;
					}
					WRITE_INT(UINT8_MAX,bf,j);
					j+=_copy_str(bf+j," Characters:\n");
					j+=_copy_str(bf+j,f_nm);
					j+=_copy_str(bf+j,":<root>.extensions[");
					WRITE_INT(i,bf,j);
					bf[j]=']';
					bf[j+1]=0;
					write_error_body(err,bf);
					_free_json(&json);
					return;
				}
				editor_syntax_file_extension_t* fe=o->e+i;
				fe->l=_copy_str(fe->dt,ex->dt.s.v);
				fe->dt[fe->l]=0;
			}
		}
		json_object_t* b_sc=_get_by_key(&json,"base_scope");
		if (!b_sc){
			editor_error_t* err=create_error(e);
			err->tl=_copy_str(err->t,"JSON Schema Error");
			err->t[err->tl]=0;
			char bf[4096]="Expected The \"base_scope\" Property:\n";
			uint32_t i=0;
			while (bf[i]){
				i++;
			}
			i+=_copy_str(bf+i,f_nm);
			bf[i+_copy_str(bf+i,":<root>.name")]=0;
			write_error_body(err,bf);
			_free_json(&json);
			return;
		}
		if (b_sc->t!=JSON_OBJECT_TYPE_ARRAY){
			editor_error_t* err=create_error(e);
			err->tl=_copy_str(err->t,"JSON Schema Error");
			err->t[err->tl]=0;
			char bf[4096]="The \"base_scope\" Property Must be an Array:\n";
			uint32_t i=0;
			while (bf[i]){
				i++;
			}
			i+=_copy_str(bf+i,f_nm);
			bf[i+_copy_str(bf+i,":<root>.base_scope")]=0;
			write_error_body(err,bf);
			_free_json(&json);
			return;
		}
		if (b_sc->dt.a.l>EDITOR_SYNTAX_MAX_SCOPE_COUNT){
			editor_error_t* err=create_error(e);
			err->tl=_copy_str(err->t,"JSON Schema Error");
			err->t[err->tl]=0;
			char bf[4096]="The \"base_scope\" Property Must Have no More Than ";
			uint32_t i=0;
			while (bf[i]){
				i++;
			}
			WRITE_INT(EDITOR_SYNTAX_MAX_SCOPE_COUNT,bf,i);
			i+=_copy_str(bf+i," Items:\n");
			i+=_copy_str(bf+i,f_nm);
			bf[i+_copy_str(bf+i,":<root>.base_scope")]=0;
			write_error_body(err,bf);
			_free_json(&json);
			return;
		}
		for (uint8_t i=0;i<b_sc->dt.a.l;i++){
			json_object_t* sc=b_sc->dt.a.dt+i;
			if (sc->t!=JSON_OBJECT_TYPE_STRING){
				editor_error_t* err=create_error(e);
				err->tl=_copy_str(err->t,"JSON Schema Error");
				err->t[err->tl]=0;
				char bf[4096]="A Scope Must be a String:\n";
				uint32_t j=0;
				while (bf[j]){
					j++;
				}
				j+=_copy_str(bf+j,f_nm);
				j+=_copy_str(bf+j,":<root>.base_scope[");
				WRITE_INT(i,bf,j);
				bf[j]=']';
				bf[j+1]=0;
				write_error_body(err,bf);
				_free_json(&json);
				return;
			}
			for (uint8_t j=1;j<sizeof(SYNTAX_SCOPE_NAME_LIST)/sizeof(SYNTAX_SCOPE_NAME_LIST[0]);j++){
				uint8_t k=-1;
				do{
					k++;
					if (*(SYNTAX_SCOPE_NAME_LIST[j]+k)!=*(sc->dt.s.v+k)){
						goto _check_next_name;
					}
				} while (*(SYNTAX_SCOPE_NAME_LIST[j]+k));
				o->b_sc.dt[i]=j;
				goto _check_next_scope;
_check_next_name:;
			}
			editor_error_t* err=create_error(e);
			err->tl=_copy_str(err->t,"JSON Schema Error");
			err->t[err->tl]=0;
			char bf[4096]="Unknown Scope Name '";
			uint32_t j=0;
			while (bf[j]){
				j++;
			}
			j+=_copy_str(bf+j,sc->dt.s.v);
			j+=_copy_str(bf+j,"':\n");
			j+=_copy_str(bf+j,f_nm);
			j+=_copy_str(bf+j,":<root>.base_scope[");
			WRITE_INT(i,bf,j);
			bf[j]=']';
			bf[j+1]=0;
			write_error_body(err,bf);
			_free_json(&json);
			return;
_check_next_scope:;
		}
		if (b_sc->dt.a.l<EDITOR_SYNTAX_MAX_SCOPE_COUNT){
			o->b_sc.dt[b_sc->dt.a.l]=EDITOR_SYNTAX_UNKNOWN_SCOPE_INDEX;
		}
		_free_json(&json);
	}
}
