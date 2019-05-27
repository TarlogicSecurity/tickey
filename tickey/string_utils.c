#include "string_utils.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

void stripr(char *line){
	size_t length = strlen(line);

	if(length == 0){
		return;
	}

	size_t i = 0;

	for(i = length - 1; i > 0; i--){
		if(isspace(line[i])){
			line[i] = '\0';
		}else{
			break;
		}
	}

	if(i == 0){
		if(isspace(line[i])){
			line[i] = '\0';
		}
	}
}



char * areplace(const char *origin, const char *match, const char *new_substr){

	size_t match_length = strlen(match);
	char *insertion_point = (char*)origin;
	
	char * tmp = NULL;
	size_t matches_count = 0;
	for(matches_count = 0; (tmp = strstr(insertion_point, match)) != NULL ; matches_count++){
		insertion_point = tmp + match_length;
	}

	size_t new_substr_length = strlen(new_substr);
	size_t result_length = strlen(origin) + (new_substr_length - match_length) * matches_count;
	char * result = (char*)malloc(result_length + 1);
	if(result == NULL){
		return NULL;
	}

	char * tmp_start = (char*)origin;
	char * result_off = result;
	while(matches_count--){
		insertion_point = strstr(tmp_start, match);
		size_t len_front = insertion_point - tmp_start;

		strncpy(result_off, tmp_start, len_front);
		result_off += len_front;
		tmp_start += len_front;

		strcpy(result_off, new_substr);
		result_off += new_substr_length;
		tmp_start += match_length;
	}
	strcpy(result_off, tmp_start);
	return result;
}



