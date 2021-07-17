#include <editor.h>
#include <_editor_internal.h>
#include <stdint.h>
#include <stdlib.h>



void update_line(editor_t* e,uint32_t i,uint8_t fl){
	editor_line_t* l=*(e->l+i);
	if (fl&EDITOR_LINE_FLAG_RENDER){
		if (l->_bf){
			free(l->_bf);
		}
		l->_bfl=e->_ll_sz+l->l+(i==e->st.cy?STR_LEN(EDITOR_UI_START_STR_HIGHLIGHT):STR_LEN(EDITOR_UI_START_STR))+STR_LEN(" "EDITOR_UI_DEFAULT_FG_COLOR);
		l->_ws_c=0;
	}
	uint32_t j=0;
	uint32_t n=0;
	fl|=(e->st.cy==i&&!(fl&_EDITOR_LINE_FLAG_UPDATE_CURSOR_POS)?_EDITOR_LINE_FLAG_HAS_CURSOR:0)|(fl&EDITOR_LINE_FLAG_RENDER?_EDITOR_LINE_FLAG_IN_WHITESPACE:0);
	for (;j<l->l;j++){
		uint8_t c=*(l->dt+j);
		if ((fl&_EDITOR_LINE_FLAG_UPDATE_CURSOR_POS)&&n>=e->st._cx){
			fl&=~_EDITOR_LINE_FLAG_UPDATE_CURSOR_POS;
			e->st.cx=j;
			e->st._cx=n;
		}
		else if ((fl&_EDITOR_LINE_FLAG_HAS_CURSOR)&&e->st.cx==j){
			fl&=~_EDITOR_LINE_FLAG_HAS_CURSOR;
			e->st._cx=n;
		}
		if ((fl&_EDITOR_LINE_FLAG_IN_WHITESPACE)&&(c==' '||c=='\t'||c=='\f'||c=='\v')){
			l->_ws_c++;
		}
		else{
			fl&=~_EDITOR_LINE_FLAG_IN_WHITESPACE;
		}
		if (c=='\t'){
			if (fl&EDITOR_LINE_FLAG_RENDER){
				l->_bfl+=EDITOR_TAB_SIZE-(n%EDITOR_TAB_SIZE)-1;
			}
			n+=EDITOR_TAB_SIZE-(n%EDITOR_TAB_SIZE);
		}
		else if (c<31||c>126){
			if (fl&EDITOR_LINE_FLAG_RENDER){
				l->_bfl+=5;
			}
			n++;
		}
		else{
			n++;
		}
	}
	if (fl&_EDITOR_LINE_FLAG_UPDATE_CURSOR_POS){
		e->st.cx=l->l;
		e->st._cx=n;
	}
	if (fl&_EDITOR_LINE_FLAG_HAS_CURSOR){
		e->st._cx=n;
	}
	if (!(fl&EDITOR_LINE_FLAG_RENDER)){
		return;
	}
	l->_bf=malloc(l->_bfl*sizeof(char));
	uint32_t k=_copy_str(l->_bf,(i==e->st.cy?EDITOR_UI_START_STR_HIGHLIGHT:EDITOR_UI_START_STR));
	char bf[10];
	j=0;
	uint32_t v=i+1;
	while (v){
		bf[j]=(v%10)+48;
		j++;
		v/=10;
	}
	while (j<e->_ll_sz){
		bf[j]=' ';
		j++;
	}
	while (j){
		j--;
		*(l->_bf+k)=bf[j];
		k++;
	}
	k+=_copy_str(l->_bf+k," "EDITOR_UI_DEFAULT_FG_COLOR);
	n=0;
	for (j=0;j<l->l;j++){
		uint8_t c=*(l->dt+j);
		if (c>31&&c<127){
			*(l->_bf+k)=c;
			k++;
			n++;
		}
		else if (c=='\t'){
			for (uint8_t m=0;m<EDITOR_TAB_SIZE-(n%EDITOR_TAB_SIZE);m++){
				*(l->_bf+k)=' ';
				k++;
			}
			n+=EDITOR_TAB_SIZE-(n%EDITOR_TAB_SIZE);
		}
		else{
			*(l->_bf+k)='<';
			*(l->_bf+k+1)='0';
			*(l->_bf+k+2)='x';
			*(l->_bf+k+3)=(c>>4)+((c>>4)>9?87:48);
			*(l->_bf+k+4)=(c&15)+((c&15)>9?87:48);
			*(l->_bf+k+5)='>';
			k+=6;
			n++;
		}
	}
}
