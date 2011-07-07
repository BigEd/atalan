/*

Common utilities

Memory management, strings etc.

(c) 2010 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/


#include "common.h"
#pragma warning (disable: 4996)

#if defined(__Windows__)
    #include <windows.h>
#endif

void * MemAllocEmpty(long size)
/*
Purpose:
	Allocate memory and set it to zeroes.
*/
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

#ifdef __Darwin__
	return strcasecmp(str1, str2) == 0;
#else
	return stricmp(str1, str2) == 0;
#endif
}
Bool   StrEqualPrefix(char * str1, char * str2, int num)
{
	if (str1 == str2) return true;
	if (str1 == NULL || str2 == NULL) return false;

#ifdef __Darwin__
	return strncasecmp(str1, str2, num) == 0;
#else
	return strnicmp(str1, str2, num) == 0;
#endif
}

char * PathFilename(char * path)
/*
Purpose:
	Find filename in path.
	Filename starts after last dir separator in the filename.
*/
{
	char * last_slash;
	last_slash = strrchr(path, DIRSEP);
	return (last_slash == NULL)?path:last_slash+1;
}

void PathSeparate(char * path, char * dirname, char * filename)
{
	char * pf;
	UInt16 len;
	pf = PathFilename(path);
	len = pf - path;

	// Copy dir to other place
	if (dirname != NULL && path != dirname) {		
		memcpy(dirname, path, len);
		dirname[len] = 0;
	}

	// Copy file path
	if (filename != NULL) {
		strcpy(filename, pf);
	}

	// If path and dirname are same, just mark the end of dir in the path
	if (path == dirname) {
		path[len] = 0;
	}
}

void PathParent(char * path)
{
	char * s;
	UInt16 len;
	len = StrLen(path);
	if (len < 2) return;
	s = path + len - 1;
	s--;		// skip last dir separator
	while(s > path) {
		if (*s == DIRSEP) { 
			*(s+1) = 0;
			break;
		}
		s--;
	}
}

void PathCutExtension(char * path, char * ext)
{
	UInt16 path_len;
	UInt16 ext_len;

	path_len = StrLen(path);
	ext_len = StrLen(ext);

	if (path_len > ext_len+1) {
		path_len -= ext_len + 1;
		if (path[path_len] == '.' && StrEqual(ext, &path[path_len+1])) {		
			path[path_len] = 0;
		}
	}
}

void PathMerge(char * path, char * dirname, char * filename)
{
	UInt16 len;
	strcpy(path, dirname);
	len = strlen(path);
	strcpy(path+len, filename);
}

void GetApplicationDir(char * name, char * path)
{
#ifdef __Windows__
	GetModuleFileName(NULL, path, MAX_PATH_LEN);
	PathSeparate(path, path, NULL);
#else
	PathSeparate(name, path, NULL);
#endif
}
