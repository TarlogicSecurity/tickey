#include "krb_conf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "string_utils.h"

#define CCACHE_NAME_FIELD "default_ccache_name"



ccache_info_t * new_ccache_info(const char * type, const char * subtype, const char * name){
	size_t type_size = 1, subtype_size = 1, name_size = 1;

	type_size = strlen(type) + 1;
	if(name){
		name_size = strlen(name) + 1;
	}
	if(subtype){
		subtype_size = strlen(subtype) + 1;
	}

	ccache_info_t * ccache_info = (ccache_info_t*)calloc(sizeof(ccache_info_t) + type_size + subtype_size + name_size, 1);
	if(ccache_info == NULL){
		return NULL;
	}

	char * str_offset = ((char*)ccache_info) + sizeof(ccache_info_t);
	
	strcpy(str_offset, type);
	ccache_info->type = str_offset;
	str_offset += type_size;

	if(subtype_size > 1){
		strcpy(str_offset, subtype);
	}
	ccache_info->subtype = str_offset;
	str_offset += subtype_size;

	if(name_size > 1){
		strcpy(str_offset, name);
	}
	ccache_info->name = str_offset;

	return ccache_info;
}


ccache_info_t * default_ccache_info(){
	return new_ccache_info("FILE", NULL, "/tmp/krb5cc_%{uid}");
}

int is_comment(char * line, size_t line_length){
	
	size_t i = 0;

	for (i = 0; i < line_length; i++){
		if(isspace(line[i])){
			continue;
		}
		if(line[i] == ';'){
			return 1;
		}
		return 0;
	}

	return 0;
}


ccache_info_t * get_ccache_name_from_option_line(char * line){
	char * index = strstr(line, CCACHE_NAME_FIELD);	
    ccache_info_t * ccache_info = NULL;
    
	index += strlen(CCACHE_NAME_FIELD);

	while(isspace(*index) || *index == '='){
		index++;
	}

	char * ccache_type = NULL, *ccache_subtype = NULL, *ccache_name = NULL;
	size_t strings_size = 0;
	char * tok = strtok(index, ":");
	if(tok == NULL){
		goto close;
	}
	ccache_type = strdup(tok);
	strings_size = strlen(ccache_type) + 1;

	tok = strtok(NULL, ":");
	if(tok != NULL){
		ccache_name = strdup(tok);
		strings_size += strlen(ccache_name) + 1;
	
		tok = strtok(NULL, ":");
		if(tok != NULL){
			ccache_subtype = ccache_name;
			ccache_name = strdup(tok); 
			strings_size += strlen(ccache_name) + 1;
		}
	}

	ccache_info = new_ccache_info(ccache_type, ccache_subtype, ccache_name);

close:
	if(ccache_type){
		free(ccache_type); ccache_type = NULL;
	}

	if(ccache_name){
		free(ccache_name); ccache_name = NULL;
	}

	if(ccache_subtype){
		free(ccache_subtype); ccache_subtype = NULL;
	}

	return ccache_info;
}

ccache_info_t * ccache_info_from_krb_conf(){

	FILE *fp = fopen(KRB5_CONF_FILE, "r");
	if(fp == NULL){
		return NULL;
	}
	
	ccache_info_t *ccache_info = NULL;
	char * line = NULL;
	size_t line_length = 0;
	while(getline(&line, &line_length, fp) != -1){
		if(strstr(line, CCACHE_NAME_FIELD) == NULL){
			continue;
		}
		if(is_comment(line, line_length)){
			continue;
		}
		stripr(line);
		ccache_info = get_ccache_name_from_option_line(line);
		break;
	}

	if(line) {
		free(line); line = NULL;
	}
	fclose(fp); fp = NULL;

	return ccache_info;
}


ccache_info_t * krb5_ccache_info(){
	ccache_info_t * ccache_info = ccache_info_from_krb_conf(); 
	if(ccache_info == NULL){
		ccache_info = default_ccache_info();
	}
	return ccache_info;
}
