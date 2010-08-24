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

char * StrAllocLen(char * str, UInt16 len)
{
	char * s = malloc(len + 1);
	memcpy(s, str, len);
	s[len] = 0;
	return s;
}

Bool   StrEqual(char * str1, char * str2)
{
	if (str1 == str2) return true;
	if (str1 == NULL || str2 == NULL) return false;

#if SYSTEM_NAME == Darwin
	return stricmp(str1, str2) == 0;
#else
	return stricmp(str1, str2) == 0;
#endif
}

