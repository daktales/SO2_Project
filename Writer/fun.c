#include <stdlib.h>

#include "fun.h"

#define MAXL 40 /*	Max lenght of generater word (null-terminated string [0..39]+\0)	*/

/* General purpose */

/* Genereate a string with random letters */
char* gen_data(){
	int loop = (rand()%(MAXL-1))+1; /* Min size 1, Max size Maxl	*/
	int i;
	char* data;
	int tmp;
	
	data = (char *) calloc((size_t)(loop+1),sizeof(char));
	for(i=0;i<loop;i++){
		tmp = (rand()%25 + 65); /* CAPITAL letters	*/
		data[i] = (char)tmp;
	}
	data[loop] = '\0';
	return data;
}

/* Capital to non-capital */
void to_lower(char* letters){
	int i =0;
	int tmp =0;
	while(letters[i]!='\0'){
		tmp = (int) letters[i];
		tmp = tmp+32;
		letters[i]=(char)tmp;
		i++;
			}
}


