/*

Common utilities

Memory management, strings etc.

(c) 2010 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/

#define OK 0

#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <math.h>


// Basic types

typedef int Bool;
typedef signed int Int16;
typedef unsigned int UInt16;
typedef unsigned long UInt32;
typedef signed long Int32;
typedef unsigned char UInt8;

#define true 1
#define false 0

// Memory management

void * MemAllocEmpty(long size);
#define MemAllocStruct(TYPE) ((TYPE *)MemAllocEmpty(sizeof(TYPE)))
#define MemEmptyVar(adr)  memset(&(adr), 0, sizeof(adr))
#define MemFree(adr) free(adr)

#define MemMove(dest, src, size) memmove(dest, src, size)

// String management

char * StrAlloc(char * str);
char * StrAllocLen(char * str, UInt16 len);
#define StrCopy(dest, src) strcpy(dest, src);
Bool   StrEqual(char * str1, char * str2);
Bool   StrEqualPrefix(char * str1, char * str2, int num);
#define StrLen(str) strlen(str)
Int16 StrEditDistance(char * s, char * t);

// Flag (bit) testing and setting

#define FlagOn(set, flag)  (((set) & (flag))!=0)
#define FlagOff(set, flag)  (((set) & (flag))==0)
#define SetFlagOff(set, flag) set &= ~(flag)
#define SetFlagOn(set, flag) set |= (flag)


//GLOBAL is used to mark global variables, so it is easy to find all of them
#define GLOBAL

// Asserts


// Path functions

#define MAX_PATH_LEN 4096

#if defined(__Darwin__) || defined(__Linux__) || defined(__FreeBSD__) || defined(__SunOS__) || defined(__OSF1__) || defined(__IRIX__) || defined(__IRIX64__) || defined(__AIX__)

#define __UNIX__

#undef DIRSEP
#define DIRSEP '/'

#else

#define __Windows__
#define DIRSEP '\\'

#endif

#ifdef __Windows__
#define ASSERT(x) if (!(x)) { printf("Assert: %s:%ld: %s", __FILE__, __LINE__, #x); while(1); }
#endif

#ifndef ASSERT
	#define ASSERT(x) if (!(x)) exit(-1)
#endif

// Path functions

char * PathFilename(char * path);
void PathSeparate(char * path, char * dirname, char * filename);
void PathParent(char * path);
void PathMerge(char * path, char * dirname, char * filename);
void GetApplicationDir(char * name, char * path);
void PathCutExtension(char * path, char * ext);
