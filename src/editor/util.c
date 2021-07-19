#include <editor.h>
#include <_editor_internal.h>
#include <stdint.h>



uint32_t _copy_str(char* d,const char* s){
	uint32_t o=0;
	while (*(s+o)){
		*(d+o)=*(s+o);
		o++;
	}
	return o;
}



uint8_t _cmp_str_len(const char* a,const char* b,uint32_t l){
	for (uint32_t i=0;i<l;i++){
		if (*(a+i)!=*(b+i)){
			return 0;
		}
	}
	return 1;
}
