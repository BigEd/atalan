#define OK 0

#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <math.h>

typedef int Bool;
typedef int Int16;
typedef unsigned int UInt16;
typedef unsigned long UInt32;
typedef long Int32;
typedef unsigned char UInt8;

#define true 1
#define false 0

void * MemAllocEmpty(long size);
#define MemAllocStruct(TYPE) ((TYPE *)MemAllocEmpty(sizeof(TYPE)))
#define MemEmptyVar(adr)  memset(&(adr), 0, sizeof(adr))
#define MemFree(adr) free(adr)

#define MemMove(dest, src, size) memmove(dest, src, size)

char * StrAlloc(char * str);
char * StrAllocLen(char * str, UInt16 len);
#define StrCopy(dest, src) strcpy(dest, src);
Bool   StrEqual(char * str1, char * str2);
Bool   StrEqualPrefix(char * str1, char * str2, int num);
#define StrLen(str) strlen(str)

// All global variables are marked with this macro, so they may be easily searched

#define FlagOn(set, flag)  (((set) & (flag))!=0)
#define FlagOff(set, flag)  (((set) & (flag))==0)
#define SetFlagOff(set, flag) set &= ~(flag)
#define SetFlagOn(set, flag) set |= (flag)


//GLOBAL is uset to mark global variables, so it is easy to find all of them
#define GLOBAL

#ifndef ASSERT
	#define ASSERT(x) if (!(x)) exit(-1)
#endif


// Path functions

#define MAX_PATH_LEN 4096

#if defined(__Darwin__) || defined(__Linux__) || defined(__FreeBSD__) || defined(__SunOS__) || defined(__OSF1__) || defined(__IRIX__) || defined(__IRIX64__) || defined(__AIX__)

#define __UNIX__

//#undef MADS_COMMAND
//#define MADS_COMMAND "mads \"%s.asm\" -o:\"%s.xex\" -x -l:\"%s.lst\""

#undef DIRSEP
#define DIRSEP '/'

//WIDOWS
#else

#define __Windows__
//#define MADS_COMMAND "mads \"%s.asm\" -o:\"%s.xex\" -x -l:\"%s.lst\""
#define DIRSEP '\\'

#endif

// Path functions

char * PathFilename(char * path);
void PathSeparate(char * path, char * dirname, char * filename);
void PathParent(char * path);
void PathMerge(char * path, char * dirname, char * filename);
void GetApplicationDir(char * name, char * path);
