#include <editor.h>
#include <_editor_internal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>



void init_editor(editor_t* e){
	e->fl=0;
	e->w=0;
	e->h=0;
	e->f.fp[0]=0;
	e->f.fpl=0;
	e->f.cx=0;
	e->f.cy=0;
	e->f.l_off=0;
	e->f.ll=0;
	e->f.l=NULL;
	e->f.s=NULL;
	e->f._cx=0;
	e->f._ll_sz=0;
	e->el=0;
	e->e=NULL;
	e->sl=0;
	e->s=NULL;
}



void set_window_size(editor_t* e,uint32_t w,uint32_t h){
	e->w=w;
	e->h=h;
}



editor_error_t* create_error(editor_t* e){
	e->el++;
	e->e=realloc(e->e,e->el*sizeof(editor_error_t*));
	editor_error_t* o=malloc(sizeof(editor_error_t));
	o->t[0]=0;
	o->tl=0;
	for (uint8_t i=0;i<EDITOR_ERROR_HEIGHT;i++){
		o->e[i][0]=0;
	}
	*(e->e+e->el-1)=o;
	return o;
}



void write_error_body(editor_error_t* e,const char* dt){
	uint8_t i=0;
	uint8_t j=0;
	uint8_t k=0;
	while (*dt){
		if (j==EDITOR_ERROR_WIDTH){
			if (k){
				e->e[i][k-1]=0;
				dt-=j-k;
			}
			e->e[i][j+1]=0;
			j=0;
			k=0;
			i++;
			continue;
		}
		if (*dt==' '){
			do{
				dt++;
			} while (*dt==' ');
			e->e[i][j]=' ';
			j++;
			k=j;
			continue;
		}
		if (*dt=='\n'){
			e->e[i][j]=0;
			j=0;
			k=0;
			i++;
			dt++;
			continue;
		}
		e->e[i][j]=*dt;
		j++;
		dt++;
	}
	e->e[i][j]=0;
	i++;
	while (i<EDITOR_ERROR_HEIGHT){
		e->e[i][0]=0;
		i++;
	}
}



void render_editor(editor_t* e){
	char* bf=malloc((STR_LEN("\x1b[?12l\x1b[H"EDITOR_SETTINGS_BG_COLOR"\x1b[0K")+e->f.fpl)*sizeof(char));
	uint32_t sz=_copy_str(bf,"\x1b[?12l\x1b[H"EDITOR_SETTINGS_BG_COLOR"\x1b[0K");
	sz+=_copy_str(bf+sz,e->f.fp);
	uint32_t i=0;
	for (;i<(e->f.ll<e->h-2?e->f.ll:e->h-2);i++){
		editor_line_t* l=*(e->f.l+e->f.l_off+i);
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
	if (e->el){
		editor_error_t* err=*(e->e+e->el-1);
		i=(e->h-EDITOR_ERROR_HEIGHT-4)>>1;
		uint32_t j=(e->w-EDITOR_ERROR_WIDTH-2)>>1;
		uint32_t w=EDITOR_ERROR_WIDTH+2;
		bf=realloc(bf,(sz+STR_LEN("\x1b[;H"EDITOR_UI_ERROR_BG_COLOR EDITOR_UI_ERROR_BORDER_COLOR"╭╮")+CURSOR_CONTROL_MAX_POSITION_STRING_LENGTH*2+STR_LEN("─")*(EDITOR_ERROR_WIDTH-err->tl)+err->tl+(STR_LEN("\x1b[;H"EDITOR_UI_ERROR_BG_COLOR EDITOR_UI_ERROR_BORDER_COLOR"│"EDITOR_UI_ERROR_FG_COLOR EDITOR_UI_ERROR_BORDER_COLOR"│")+CURSOR_CONTROL_MAX_POSITION_STRING_LENGTH*2+EDITOR_ERROR_WIDTH)*EDITOR_ERROR_HEIGHT+STR_LEN("\x1b[;H"EDITOR_UI_ERROR_BG_COLOR EDITOR_UI_ERROR_BORDER_COLOR"├┤")+CURSOR_CONTROL_MAX_POSITION_STRING_LENGTH*2+STR_LEN("─")*EDITOR_ERROR_WIDTH+STR_LEN("\x1b[;H"EDITOR_UI_ERROR_BG_COLOR EDITOR_UI_ERROR_BORDER_COLOR"│"EDITOR_UI_ERROR_BUTTON_BG_COLOR EDITOR_UI_ERROR_BUTTON_FG_COLOR"OK"EDITOR_UI_ERROR_BG_COLOR EDITOR_UI_ERROR_BORDER_COLOR"│")+CURSOR_CONTROL_MAX_POSITION_STRING_LENGTH*2+STR_LEN(" ")*(EDITOR_ERROR_WIDTH-STR_LEN("OK"))+STR_LEN("\x1b[;H"EDITOR_UI_ERROR_BG_COLOR EDITOR_UI_ERROR_BORDER_COLOR"╰╯")+CURSOR_CONTROL_MAX_POSITION_STRING_LENGTH*2+STR_LEN("─")*EDITOR_ERROR_WIDTH)*sizeof(char));
		*(bf+sz)='\x1b';
		*(bf+sz+1)='[';
		sz+=2;
		WRITE_INT(i,bf,sz);
		*(bf+sz)=';';
		sz++;
		WRITE_INT(j,bf,sz);
		sz+=_copy_str(bf+sz,"H"EDITOR_UI_ERROR_BG_COLOR EDITOR_UI_ERROR_BORDER_COLOR"╭");
		const char* h_str="─";
		for (uint16_t k=0;k<((EDITOR_ERROR_WIDTH-err->tl)>>1);k++){
			const char* tmp=h_str;
			while (*tmp){
				*(bf+sz)=*tmp;
				sz++;
				tmp++;
			}
		}
		sz+=_copy_str(bf+sz,err->t);
		for (uint16_t k=0;k<((EDITOR_ERROR_WIDTH-err->tl+1)>>1);k++){
			const char* tmp=h_str;
			while (*tmp){
				*(bf+sz)=*tmp;
				sz++;
				tmp++;
			}
		}
		sz+=_copy_str(bf+sz,"╮");
		for (uint16_t k=0;k<EDITOR_ERROR_HEIGHT;k++){
			i++;
			*(bf+sz)='\x1b';
			*(bf+sz+1)='[';
			sz+=2;
			WRITE_INT(i,bf,sz);
			*(bf+sz)=';';
			sz++;
			WRITE_INT(j,bf,sz);
			sz+=_copy_str(bf+sz,"H"EDITOR_UI_ERROR_BG_COLOR EDITOR_UI_ERROR_BORDER_COLOR"│"EDITOR_UI_ERROR_FG_COLOR);
			uint8_t l=0;
			while (err->e[k][l]){
				*(bf+sz)=err->e[k][l];
				sz++;
				l++;
			}
			while (l<EDITOR_ERROR_WIDTH){
				*(bf+sz)=' ';
				sz++;
				l++;
			}
			sz+=_copy_str(bf+sz,EDITOR_UI_ERROR_BORDER_COLOR"│");
		}
		i++;
		*(bf+sz)='\x1b';
		*(bf+sz+1)='[';
		sz+=2;
		WRITE_INT(i,bf,sz);
		*(bf+sz)=';';
		sz++;
		WRITE_INT(j,bf,sz);
		sz+=_copy_str(bf+sz,"H"EDITOR_UI_ERROR_BG_COLOR EDITOR_UI_ERROR_BORDER_COLOR"├");
		for (uint16_t k=0;k<EDITOR_ERROR_WIDTH;k++){
			const char* tmp=h_str;
			while (*tmp){
				*(bf+sz)=*tmp;
				sz++;
				tmp++;
			}
		}
		sz+=_copy_str(bf+sz,"┤\x1b[");
		i++;
		WRITE_INT(i,bf,sz);
		*(bf+sz)=';';
		sz++;
		WRITE_INT(j,bf,sz);
		sz+=_copy_str(bf+sz,"H"EDITOR_UI_ERROR_BG_COLOR EDITOR_UI_ERROR_BORDER_COLOR"│");
		for (uint16_t k=0;k<(EDITOR_ERROR_WIDTH-STR_LEN("OK"))>>1;k++){
			*(bf+sz)=' ';
			sz++;
		}
		sz+=_copy_str(bf+sz,EDITOR_UI_ERROR_BUTTON_BG_COLOR EDITOR_UI_ERROR_BUTTON_FG_COLOR"OK"EDITOR_UI_ERROR_BG_COLOR EDITOR_UI_ERROR_BORDER_COLOR);
		for (uint16_t k=0;k<(EDITOR_ERROR_WIDTH-STR_LEN("OK")+1)>>1;k++){
			*(bf+sz)=' ';
			sz++;
		}
		sz+=_copy_str(bf+sz,"│\x1b[");
		i++;
		WRITE_INT(i,bf,sz);
		*(bf+sz)=';';
		sz++;
		WRITE_INT(j,bf,sz);
		sz+=_copy_str(bf+sz,"H"EDITOR_UI_ERROR_BG_COLOR EDITOR_UI_ERROR_BORDER_COLOR"╰");
		for (uint16_t k=0;k<EDITOR_ERROR_WIDTH;k++){
			const char* tmp=h_str;
			while (*tmp){
				*(bf+sz)=*tmp;
				sz++;
				tmp++;
			}
		}
		sz+=_copy_str(bf+sz,"╯");
	}
	bf=realloc(bf,(sz+STR_LEN("\x1b[;1H"EDITOR_SETTINGS_FG_COLOR EDITOR_SETTINGS_BG_COLOR"\x1b[0KLine , Column \x1b[;H\x1b[0m\x1b[;H")+CURSOR_CONTROL_MAX_POSITION_STRING_LENGTH*7+EDITOR_SYNTAX_NAME_LENGTH)*sizeof(char));
	*(bf+sz)='\x1b';
	*(bf+sz+1)='[';
	sz+=2;
	WRITE_INT(e->h,bf,sz);
	sz+=_copy_str(bf+sz,";1H"EDITOR_SETTINGS_FG_COLOR EDITOR_SETTINGS_BG_COLOR"\x1b[0KLine ");
	WRITE_INT(e->f.cy+1,bf,sz);
	sz+=_copy_str(bf+sz,", Column ");
	WRITE_INT(e->f.cx+1,bf,sz);
	*(bf+sz)='\x1b';
	*(bf+sz+1)='[';
	sz+=2;
	WRITE_INT(e->h,bf,sz);
	*(bf+sz)=';';
	sz++;
	WRITE_INT(e->w-e->f.s->nml+1,bf,sz);
	*(bf+sz)='H';
	sz++;
	sz+=_copy_str(bf+sz,e->f.s->nm);
	sz+=_copy_str(bf+sz,"\x1b[0m\x1b[");
	WRITE_INT(e->f.cy-e->f.l_off+2,bf,sz);
	*(bf+sz)=';';
	sz++;
	WRITE_INT(e->f._cx+e->f._ll_sz+3,bf,sz);
	*(bf+sz)='H';
	fwrite(bf,sizeof(char),sz+1,stdout);
	free(bf);
}


void free_editor(editor_t* e){
	for (uint32_t i=0;i<e->f.ll;i++){
		editor_line_t* l=*(e->f.l+i);
		if (l->dt){
			free(l->dt);
		}
		free(l);
	}
	if (e->f.l){
		free(e->f.l);
	}
	for (uint32_t i=0;i<e->el;i++){
		free(*(e->e+i));
	}
	if (e->e){
		free(e->e);
	}
	for (uint32_t i=0;i<e->sl;i++){
		free(*(e->s+i));
	}
	if (e->s){
		free(e->s);
	}
}
