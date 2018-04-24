#include <stdio.h>
#include <string.h>


int fill_unit(char * buf , const char * text){
	int len = strlen(text);
	//printf("in fill_unit\r\n");
	strcpy(buf , "<td>");
	strcpy(buf+4 , text);
	strncpy(buf+4+len , "</td>" , 5);
	return (len+9);
}

int fill_row(char * buf , const char ** text_arr){
	const char ** p_text;
	int offset = 0;

	//printf("in fill_row\r\n");
	strcpy(buf , "<tr>");
	offset += 4;
	for(p_text = text_arr ; *p_text ; ++p_text){
		offset += fill_unit(buf + offset, *p_text);
	}
	strcpy(buf+offset , "</tr>");
	return offset+5;
}

int form_cp_ret(char* buf,const char * str){
	strcpy(buf,str);
	return strlen(str)+1;
}

int form_insert_cfg_btn(char * buf,const char * name){
	// sprintf function return character 
	// number including terminating NULL
#if 0
	return sprintf(buf ,
	"<input type=\"submit\" value=\"&#37197;&#32622;\" name=\"%s\">",
	name)+1;
#else
	return sprintf(buf ,
	"<button type=\"submit\" name=\"%s\" class=\"btn btn-primary btn-mini\">&#37197;&#32622;</button>",
	name)+1;
#endif
}

int mystrncmp(const char * s,const char * fix_str){
	return strncmp(s,fix_str , strlen(fix_str));
}

