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

/****************************************/
/*Implementation of Levenshtein distance*/
/****************************************/

int minimum(int a,int b,int c)
/*Gets the minimum of three values*/
{
  int min=a;
  if(b<min)
    min=b;
  if(c<min)
    min=c;
  return min;
}

Int16 StrEditDistance(char * s, char * t)
/*Compute levenshtein distance between s and t*/
{
  //Step 1
  int k,i,j,n,m,cost,*d,distance;
  n = StrLen(s); 
  m = StrLen(t);

  if (n!=0&&m!=0) {
    d = malloc((sizeof(int))*(m+1)*(n+1));
    m++;
    n++;
    //Step 2	
    for(k=0;k<n;k++) d[k]=k;
    for(k=0;k<m;k++) d[k*n]=k;
    //Step 3 and 4	
    for (i=1;i<n;i++) {
      for (j=1;j<m;j++) {
        //Step 5
        if (s[i-1]==t[j-1])
          cost=0;
        else
          cost=1;
        //Step 6			 
        d[j*n+i]=minimum(d[(j-1)*n+i]+1,d[j*n+i-1]+1,d[(j-1)*n+i-1]+cost);
      }
	}
    distance=d[n*m-1];
    free(d);
    return distance;
  }
  else 
    return -1; //a negative return value means that one or both strings are empty.
}


#if defined(__Windows__)
    #include <windows.h>
#endif

UInt16 G_OLD_CP;
UInt8 G_COLOR;
FILE * G_PRINT_OUTPUT;		// either STDOUT or STDERR

void PrintInit()
{
#ifdef __Windows__
	G_OLD_CP = GetConsoleOutputCP();
	SetConsoleOutputCP(CP_UTF8);
#endif
	PrintDestination(stdout);
	PrintColor(RED+GREEN+BLUE);
}

void PrintCleanup()
{
#ifdef __Windows__
	SetConsoleOutputCP(G_OLD_CP);
#endif
}

FILE * PrintDestination(FILE * file)
{
	FILE * f = G_PRINT_OUTPUT;
	G_PRINT_OUTPUT = file;
	return f;
}

UInt8 PrintColor(UInt8 color)
/*
Purpose:
	Change the color of printed text.
*/
{
	UInt8 old_color = G_COLOR;
#ifdef __Windows__
	HANDLE hStdout; 
	hStdout = GetStdHandle(STD_OUTPUT_HANDLE); 
	SetConsoleTextAttribute(hStdout, color);
#endif
	G_COLOR = color;
	return old_color;
}

void Print(char * text)
{
	if (text != NULL) {
		fprintf(G_PRINT_OUTPUT, "%s", text);
	}
}

void PrintChar(char c)
{
	fputc(c, G_PRINT_OUTPUT);
}

void PrintEOL()
{
	Print("\n");
}

void PrintInt(Int32 n)
{
	fprintf(G_PRINT_OUTPUT, "%d", n);
}

void PrintRepeat(char * text, UInt16 cnt)
{
	while(cnt-- > 0) {
		Print(text);
	}
}
