/*

Common utilities

Memory management, strings etc.

(c) 2010 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/


#include "common.h"
#pragma warning (disable: 4996)

void * MemAllocEmpty(long size)
{
	void * m = malloc(size);
	memset(m, 0, size);
	return m;
}

char * StrAlloc(char * str)
{
	if (str != NULL) str = strdup(str);
	return str;
}

Bool   StrEqual(char * str1, char * str2)
{
	if (str1 == str2) return true;
	if (str1 == NULL || str2 == NULL) return false;
	return stricmp(str1, str2) == 0;
}

