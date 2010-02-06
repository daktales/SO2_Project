#ifndef _FUN_H_
#define _FUN_H_

#define MAXL 40 /*	Max lenght of generater word (null-terminated string)	*/

/* Genereate a string with random letters */
char* gen_data();

/* Capital to non-capital */
void to_lower(char* letters);

/* Non-Capital to Capital */
void to_upper(char* letters);
#endif
