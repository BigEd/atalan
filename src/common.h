#define OK 0
#include <stdlib.h>
#include <memory.h>
#include <string.h>

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
char * StrAlloc(char * str);
Bool   StrEqual(char * str1, char * str2);
#define StrLen(str) strlen(str)

// All global variables are marked with this macro, so they may be easily searched

#define FlagOn(set, flag)  (((set) & (flag))!=0)
#define FlagOff(set, flag)  (((set) & (flag))==0)
#define SetFlagOff(set, flag) set &= !(flag)


#define GLOBAL

#ifndef ASSERT
	#define ASSERT(x) if (!(x)) exit(-1)
#endif

#define MADS_COMMAND "mads.exe \"%s.asm\" -o:\"%s.xex\" -x"
#define DIRSEP '\\'

#if SYSTEM_NAME == Darwin || SYSTEM_NAME == Linux || SYSTEM_NAME == FreeBSD || SYSTEM_NAME == SunOS || SYSTEM_NAME == OSF1 || SYSTEM_NAME == IRIX || SYSTEM_NAME == IRIX64 || SYSTEM_NAME == AIX || SYSTEM_NAME == HP-UX


#undef MADS_COMMAND
#define MADS_COMMAND "mads \"%s.asm\" -o:\"%s.xex\" -x"

#undef DIRSEP
#define DIRSEP '/'

#endif

#if SYSTEM_NAME == Darwin 
#define stricmp strcasecmp
#endif
