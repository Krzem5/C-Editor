#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS 1
#endif
#include <editor.h>
#include <_editor_internal.h>
#include <stdarg.h>
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



void _schema_error(editor_t* e,const char* str,...){
	editor_error_t* err=create_error(e);
	va_list va;
	va_start(va,str);
	char bf[4096];
	uint16_t i=0;
	while (*str){
		if (*str=='%'){
			str++;
			if (*str=='s'){
				i+=_copy_str(bf+i,va_arg(va,const char*));
			}
			else if (*str=='i'){
				WRITE_INT(va_arg(va,uint32_t),bf,i);
			}
			else{
				bf[i]='%';
				bf[i+1]=*str;
				i++;
			}
			str++;
			continue;
		}
		bf[i]=*str;
		i++;
		str++;
	}
	va_end(va);
	bf[i]=0;
	err->tl=_copy_str(err->t,"JSON Schema Error");
	err->t[err->tl]=0;
	write_error_body(err,bf);
}



uint8_t _find_scope(const char* nm){
	for (uint8_t i=1;i<sizeof(SYNTAX_SCOPE_NAME_LIST)/sizeof(SYNTAX_SCOPE_NAME_LIST[0]);i++){
		uint8_t j=-1;
		do{
			j++;
			if (*(SYNTAX_SCOPE_NAME_LIST[i]+j)!=*(nm+j)){
				goto _check_next_scope;
			}
		} while (*(SYNTAX_SCOPE_NAME_LIST[i]+j));
		return i;
_check_next_scope:;
	}
	return EDITOR_SYNTAX_UNKNOWN_SCOPE_INDEX;
}



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
			_schema_error(e,"Expected a Map:\n%s:<root>",f_nm);
			_free_json(&json);
			return;
		}
		json_object_t* nm=_get_by_key(&json,"name");
		if (!nm){
			_schema_error(e,"Expected a \"name\" Property:\n%s:<root>",f_nm);
			_free_json(&json);
			return;
		}
		if (nm->t!=JSON_OBJECT_TYPE_STRING){
			_schema_error(e,"The \"name\" Property Must be a String:\n%s:<root>.name",f_nm);
			_free_json(&json);
			return;
		}
		if (nm->dt.s.l>EDITOR_SYNTAX_NAME_LENGTH){
			_schema_error(e,"The \"name\" Property Must be No More Than %i Character:\n%s:<root>.name",EDITOR_SYNTAX_NAME_LENGTH,f_nm);
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
			_schema_error(e,"Expected an \"extensions\" Property:\n%s:<root>",f_nm);
			_free_json(&json);
			return;
		}
		if (el->t==JSON_OBJECT_TYPE_NULL){
			o->e=NULL;
			o->el=0;
		}
		else{
			if (el->t!=JSON_OBJECT_TYPE_ARRAY){
				_schema_error(e,"The \"extensions\" Property Must be an Array:\n%s:<root>.extensions",f_nm);
				_free_json(&json);
				return;
			}
			o->e=malloc(el->dt.a.l*sizeof(editor_syntax_file_extension_t));
			o->el=el->dt.a.l;
			for (uint32_t i=0;i<el->dt.a.l;i++){
				json_object_t* ex=el->dt.a.dt+i;
				if (ex->t!=JSON_OBJECT_TYPE_STRING){
					_schema_error(e,"Extension Must be a String:\n%s:<root>.extensions[%i]",f_nm,i);
					_free_json(&json);
					return;
				}
				if (ex->dt.s.l>UINT8_MAX){
					_schema_error(e,"An Extension Must be No More Than %i Characters:\n%s:<root>.extensions[%i]",UINT8_MAX,f_nm,i);
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
			_schema_error(e,"Expected a \"base_scope\" Property:\n%s:<root>",f_nm);
			_free_json(&json);
			return;
		}
		if (b_sc->t!=JSON_OBJECT_TYPE_ARRAY){
			_schema_error(e,"The \"base_scope\" Property Must be an Array:\n%s:<root>.base_scope",f_nm);
			_free_json(&json);
			return;
		}
		if (b_sc->dt.a.l>EDITOR_SYNTAX_MAX_SCOPE_COUNT){
			_schema_error(e,"The \"base_scope\" Property Must Have no More Than %i Items:\n%s:<root>.base_scope",EDITOR_SYNTAX_MAX_SCOPE_COUNT,f_nm);
			_free_json(&json);
			return;
		}
		for (uint8_t i=0;i<b_sc->dt.a.l;i++){
			json_object_t* sc=b_sc->dt.a.dt+i;
			if (sc->t!=JSON_OBJECT_TYPE_STRING){
				_schema_error(e,"A Scope Must be a String:\n%s:<root>.base_scope[%i]",f_nm,i);
				_free_json(&json);
				return;
			}
			o->b_sc.dt[i]=_find_scope(sc->dt.s.v);
			if (o->b_sc.dt[i]==EDITOR_SYNTAX_UNKNOWN_SCOPE_INDEX){
				_schema_error(e,"Unknown Scope Name '%s':\n%s:<root>.base_scope[%i]",sc->dt.s.v,f_nm,i);
				_free_json(&json);
				return;
			}
		}
		if (b_sc->dt.a.l<EDITOR_SYNTAX_MAX_SCOPE_COUNT){
			o->b_sc.dt[b_sc->dt.a.l]=EDITOR_SYNTAX_UNKNOWN_SCOPE_INDEX;
		}
		json_object_t* dt=_get_by_key(&json,"data");
		if (dt){
			if (dt->t!=JSON_OBJECT_TYPE_ARRAY){
				_schema_error(e,"The \"data\" Property Must be an Array:\n%s:<root>.data",f_nm);
				_free_json(&json);
				return;
			}
			if (dt->dt.a.l>UINT16_MAX){
				_schema_error(e,"The \"data\" Property Must Have no More Than %i Items:\n%s:<root>.data",UINT16_MAX,f_nm);
				_free_json(&json);
				return;
			}
			o->cl=dt->dt.a.l;
			o->c=malloc(o->cl*sizeof(editor_syntax_context_t));
			for (uint16_t i=0;i<o->cl;i++){
				json_object_t* ctx=dt->dt.a.dt+i;
				editor_syntax_context_t* c=o->c+i;
				if (ctx->t!=JSON_OBJECT_TYPE_ARRAY){
					_schema_error(e,"A Context Must be an Array:\n%s:<root>.data[%i]",f_nm,i);
					_free_json(&json);
					return;
				}
				if (ctx->dt.a.l>UINT16_MAX){
					_schema_error(e,"A Context Must Have no More Than %i Items:\n%s:<root>.data[%i]",UINT16_MAX,f_nm,i);
					_free_json(&json);
					return;
				}
				c->l=ctx->dt.a.l;
				c->rw=0;
				c->sc.dt[0]=EDITOR_SYNTAX_UNKNOWN_SCOPE_INDEX;
				c->e=malloc(c->l*sizeof(editor_syntax_context_element_t));
				for (uint16_t j=0;j<c->l;j++){
					json_object_t* ctx_e=ctx->dt.a.dt+j;
					if (ctx_e->t!=JSON_OBJECT_TYPE_MAP){
						_schema_error(e,"A Context Must be a Map:\n%s:<root>.data[%i][%i]",f_nm,i,j);
						_free_json(&json);
						return;
					}
					json_object_t* ctx_e_t=_get_by_key(ctx_e,"type");
					if (!ctx_e_t){
						_schema_error(e,"Expected a \"type\" Property:\n%s:<root>.data[%i][%i]",f_nm,i,j);
						_free_json(&json);
						return;
					}
					if (ctx_e_t->t!=JSON_OBJECT_TYPE_STRING){
						_schema_error(e,"The \"type\" Property Must be a String:\n%s:<root>.data[%i][%i]",f_nm,i,j);
						_free_json(&json);
						return;
					}
					if (ctx_e_t->dt.s.l==4&&_cmp_str_len(ctx_e_t->dt.s.v,"link",4)){
						json_object_t* ctx_e_id=_get_by_key(ctx_e,"index");
						if (!ctx_e_id){
							_schema_error(e,"Expected a \"index\" Property:\n%s:<root>.data[%i][%i]",f_nm,i,j);
							_free_json(&json);
							return;
						}
						if (ctx_e_id->t!=JSON_OBJECT_TYPE_INTEGER||ctx_e_id->dt.i<0){
							_schema_error(e,"The \"index\" Property Must be a Positive Integer:\n%s:<root>.data[%i][%i].index",f_nm,i,j);
							_free_json(&json);
							return;
						}
						if (ctx_e_id->dt.i>=o->cl){
							_schema_error(e,"The \"index\" Property Must be Lower Than %i:\n%s:<root>.data[%i][%i].index",o->cl,f_nm,i,j);
							_free_json(&json);
							return;
						}
						(c->e+j)->fl=EDITOR_SYNTAX_CONTEXT_TYPE_LINK;
						(c->e+j)->dt.l=(uint16_t)(ctx_e_id->dt.i);
					}
					else if (ctx_e_t->dt.s.l==5&&_cmp_str_len(ctx_e_t->dt.s.v,"regex",5)){
						json_object_t* ctx_e_rx=_get_by_key(ctx_e,"regex");
						if (!ctx_e_rx){
							_schema_error(e,"Expected a \"regex\" Property:\n%s:<root>.data[%i][%i]",f_nm,i,j);
							_free_json(&json);
							return;
						}
						if (ctx_e_rx->t!=JSON_OBJECT_TYPE_STRING){
							_schema_error(e,"The \"regex\" Property Must be a String:\n%s:<root>.data[%i][%i].regex",f_nm,i,j);
							_free_json(&json);
							return;
						}
						(c->e+j)->fl=EDITOR_SYNTAX_CONTEXT_TYPE_REGEX;
						editor_syntax_context_element_data_regex_t* r_dt=malloc(sizeof(editor_syntax_context_element_data_regex_t));
						(c->e+j)->dt.m=r_dt;
						json_object_t* ctx_e_sc=_get_by_key(ctx_e,"scope");
						if (!ctx_e_sc){
							_schema_error(e,"Expected a \"scope\" Property:\n%s:<root>.data[%i][%i]",f_nm,i,j);
							_free_json(&json);
							return;
						}
						if (ctx_e_sc->t!=JSON_OBJECT_TYPE_ARRAY){
							_schema_error(e,"A \"scope\" Property Must be an Array:\n%s:<root>.data[%i][%i].scope",f_nm,i,j);
							_free_json(&json);
							return;
						}
						if (ctx_e_sc->dt.a.l>EDITOR_SYNTAX_MAX_SCOPE_COUNT){
							_schema_error(e,"A \"scope\" Property Must Have no More Than %i Items:\n%s:<root>.data[%i][%i].scope",EDITOR_SYNTAX_MAX_SCOPE_COUNT,f_nm,i,j);
							_free_json(&json);
							return;
						}
						for (uint8_t k=0;k<ctx_e_sc->dt.a.l;k++){
							json_object_t* sc=ctx_e_sc->dt.a.dt+k;
							if (sc->t!=JSON_OBJECT_TYPE_STRING){
								_schema_error(e,"A Scope Must be a String:\n%s:<root>.data[%i][%i].scope[%i]",f_nm,i,j,k);
								_free_json(&json);
								return;
							}
							r_dt->sc.dt[k]=_find_scope(sc->dt.s.v);
							if (r_dt->sc.dt[k]==EDITOR_SYNTAX_UNKNOWN_SCOPE_INDEX){
								_schema_error(e,"Unknown Scope Name '%s':\n%s:<root>.data[%i][%i].scope[%i]",sc->dt.s.v,f_nm,i,j,k);
								_free_json(&json);
								return;
							}
						}
						if (ctx_e_sc->dt.a.l<EDITOR_SYNTAX_MAX_SCOPE_COUNT){
							r_dt->sc.dt[ctx_e_sc->dt.a.l]=EDITOR_SYNTAX_UNKNOWN_SCOPE_INDEX;
						}
						json_object_t* ctx_e_id=_get_by_key(ctx_e,"push");
						if (ctx_e_id){
							if (ctx_e_id->t!=JSON_OBJECT_TYPE_INTEGER||ctx_e_id->dt.i<0){
								_schema_error(e,"The \"push\" Property Must be a Positive Integer:\n%s:<root>.data[%i][%i].push",f_nm,i,j);
								_free_json(&json);
								return;
							}
							if (ctx_e_id->dt.i>=o->cl){
								_schema_error(e,"The \"push\" Property Must be Lower Than %i:\n%s:<root>.data[%i][%i].push",o->cl,f_nm,i,j);
								_free_json(&json);
								return;
							}
							r_dt->p=(uint16_t)(ctx_e_id->dt.i);
						}
						json_object_t* ctx_e_p=_get_by_key(ctx_e,"pop");
						if (ctx_e_p){
							if (ctx_e_p->t==JSON_OBJECT_TYPE_TRUE){
								(c->e+j)->fl|=EDITOR_SYNTAX_CONTEXT_FLAG_POP;
							}
							else if(ctx_e_p->t!=JSON_OBJECT_TYPE_FALSE){
								_schema_error(e,"The \"pop\" Property Must be a Boolean:\n%s:<root>.data[%i][%i].pop",f_nm,i,j);
								_free_json(&json);
								return;
							}
						}
					}
					else if (ctx_e_t->dt.s.l==5&&_cmp_str_len(ctx_e_t->dt.s.v,"scope",5)){
						c->l--;
						if (c->sc.dt[0]!=EDITOR_SYNTAX_UNKNOWN_SCOPE_INDEX){
							_schema_error(e,"Multiple \"scope\" Sections not Allowed:\n%s:<root>.data[%i][%i]",f_nm,i,j);
							_free_json(&json);
							return;
						}
						json_object_t* ctx_e_sc=_get_by_key(ctx_e,"scope");
						if (!ctx_e_sc){
							_schema_error(e,"Expected a \"scope\" Property:\n%s:<root>.data[%i][%i]",f_nm,i,j);
							_free_json(&json);
							return;
						}
						if (ctx_e_sc->dt.a.l>EDITOR_SYNTAX_MAX_SCOPE_COUNT){
							_schema_error(e,"The \"scope\" Property Must Have no More Than %i Items:\n%s:<root>.data[%i][%i].scope",EDITOR_SYNTAX_MAX_SCOPE_COUNT,f_nm,i,j);
							_free_json(&json);
							return;
						}
						for (uint8_t k=0;k<ctx_e_sc->dt.a.l;k++){
							json_object_t* sc=ctx_e_sc->dt.a.dt+k;
							if (sc->t!=JSON_OBJECT_TYPE_STRING){
								_schema_error(e,"A Scope Must be a String:\n%s:<root>.data[%i][%i].scope[%i]",f_nm,i,j,k);
								_free_json(&json);
								return;
							}
							c->sc.dt[k]=_find_scope(sc->dt.s.v);
							if (c->sc.dt[k]==EDITOR_SYNTAX_UNKNOWN_SCOPE_INDEX){
								_schema_error(e,"Unknown Scope Name '%s':\n%s:<root>.data[%i][%i].scope[%i]",sc->dt.s.v,f_nm,i,j,k);
								_free_json(&json);
								return;
							}
						}
						if (ctx_e_sc->dt.a.l<EDITOR_SYNTAX_MAX_SCOPE_COUNT){
							c->sc.dt[ctx_e_sc->dt.a.l]=EDITOR_SYNTAX_UNKNOWN_SCOPE_INDEX;
						}
					}
					else if (ctx_e_t->dt.s.l==6&&_cmp_str_len(ctx_e_t->dt.s.v,"rewind",6)){
						if (c->rw){
							_schema_error(e,"Multiple \"rewind\" Sections not Allowed:\n%s:<root>.data[%i][%i]",f_nm,i,j);
							_free_json(&json);
							return;
						}
						json_object_t* ctx_e_cnt=_get_by_key(ctx_e,"count");
						if (!ctx_e_cnt){
							_schema_error(e,"Expected a \"count\" Property:\n%s:<root>.data[%i][%i]",f_nm,i,j);
							_free_json(&json);
							return;
						}
						if (ctx_e_cnt->t!=JSON_OBJECT_TYPE_INTEGER||ctx_e_cnt->dt.i<0){
							_schema_error(e,"The \"count\" Property Must be a Positive Integer:\n%s:<root>.data[%i][%i].count",f_nm,i,j);
							_free_json(&json);
							return;
						}
						c->rw=(ctx_e_cnt->dt.i?(uint16_t)(ctx_e_cnt->dt.i):EDITOR_SYNTAX_REWIND_ALL);
					}
					else{
						_schema_error(e,"Unknown Type '%s':\n%s:<root>.data[%i][%i].type",ctx_e_t->dt.s.v,f_nm,i,j);
						_free_json(&json);
						return;
					}
				}
				if (ctx->dt.a.l!=c->l){
					c->e=realloc(c->e,c->l*sizeof(editor_syntax_context_element_t));
				}
			}
		}
		_free_json(&json);
	}
}
