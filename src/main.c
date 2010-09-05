/*

ATALAN - Programming language Compiler for embeded systems


(c) 2010 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/

#include "language.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> 

#define STDERR stderr

#ifdef __UNIX__
#include <unistd.h>
#endif

GLOBAL Bool VERBOSE;

extern Var   ROOT_PROC;
extern Var * INSTRSET;		// enumerator with instructions
extern InstrBlock * CODE;
Var * INTERRUPT;
char PLATFORM[64];			// name of platform

int main(int argc, char *argv[])
{
	Var * var, * data;
	Type * type;
	Int16 i;
	TypeVariant tv;
	Bool assembler = true;
	int result = 0;
	UInt32 size, n, adr;
	char filename[MAX_PATH_LEN], command[MAX_PATH_LEN], path[MAX_PATH_LEN];
	UInt16 filename_len;
	char * s;
	UInt16 len;

	VERBOSE = false;

	// Default platform is "atari"

	strcpy(PLATFORM, "atari");

	// System folder is parent directory of directory where the binary is stored
	//
	//  bin/
	//      atalan.exe
	//      mads.exe
	//  modules/
	//		system.atl
	//      ;platform independent modules
	//  plaforms/
	//      atari/
	//         ;platform dependent modules
	//      c64/
	//  processors/
	//      m6502/

	getcwd(ABSOLUTE_PATH,MAX_PATH_LEN);

	printf("argv[0]: %s\n", argv[0]);
	printf("cwd: %s\n", ABSOLUTE_PATH);

	GetApplicationDir(argv[0], SYSTEM_DIR);
	PathParent(SYSTEM_DIR);

	InitErrors();

	//
    // Check arguments.
    //

	printf("Atalan programming language compiler (23-Aug-2010)\nby Rudla Kudla (http:\\atalan.kutululu.org)\n\n");

	i = 1;
	while (i < argc) {		
		if (StrEqual(argv[i], "-V")) {
			VERBOSE = true;
		} else if (StrEqual(argv[i], "-A")) {
			assembler = false;
		} else if (StrEqual(argv[i], "-O")) {

		} else if (StrEqual(argv[i], "-I")) {
			i++;
			if (i<argc)
				if (strlen(argv[i]) < MAX_PATH_LEN) {
					strcpy(SYSTEM_DIR, argv[i]);
					s = &SYSTEM_DIR[strlen(SYSTEM_DIR)-1];
					if (*s!=DIRSEP)	{
						s++;
						*s++=DIRSEP;
						*s='\0';
					}

				}
		} else {
			break;
		}
		i++;
	}

	printf("System dir: %s\n", SYSTEM_DIR);

    if (i == argc) {
        fprintf(STDERR, "Usage:\n"
	"%s [-v][-a] file\n"
	"  -v Verbose output\n"
	"  -I <SYSTEM_DIR> define include path (default: current catalog)\n"
	"  -a Only generate assembler source code, but do not call assembler\n", argv[0]);
        exit(-1);
    }

	// If the filename has .atl extension, cut it

	strcpy(filename, argv[i]);
	filename_len = StrLen(filename);

	if (filename_len > 4 && StrEqual(".atl", &filename[filename_len-4])) {
		filename_len -= 4;
		filename[filename_len] = 0;
	}

	//==== Split dir and filename

	PathSeparate(filename, PROJECT_DIR, filename);

	printf("Project dir: %s\n", PROJECT_DIR);

	printf("Building %s%s.atl\n", PROJECT_DIR, filename);

	//===== Initialize

	TypeInit();
	VarInit();
	InstrInit();
	ParseInit();
	LexerInit();

	data = VarNewTmpLabel();

	//==== Initialize system dir

	// system.atl is file defining some ATALAN basics.
	// It must always be included.
	// Some of the definitions in system.atl are directly used by compiler.

	if (!Parse("system", false)) goto failure;
	
	INSTRSET  = VarFindScope(&ROOT_PROC, "instrs", 0);
	REGSET    = VarFindScope(&ROOT_PROC, "regset", 0);
	INTERRUPT = VarFindScope(&ROOT_PROC, "interrupt", 0);

	if (!Parse("m6502", false)) goto failure;
	if (!Parse("atari", false)) goto failure;

	VarInitRegisters();

	// Parse the file. This also generates main body of the program (_ROOT procedure).
	// TODO: Root procedure may be just specifal type of procedure.
	//       Prologue and epilogue may be replaced by proc type specific PROC and ANDPROC instructions.

	Gen(INSTR_PROLOGUE, NULL, NULL, NULL);
	SYSTEM_PARSE = false;
	if (!Parse(filename, true)) goto failure;
	SYSTEM_PARSE = true;
	Gen(INSTR_EPILOGUE, NULL, NULL, NULL);

	// Report warning about logical errors

	if (LOGIC_ERROR_CNT > 0 && ERROR_CNT == 0) {
		Warning("There were logical errors.\nCompilation will proceed, but the resulting program may be errorneous.");
	}


	ROOT_PROC.instr = CODE;
	VarGenerateArrays();

	if (VERBOSE) {
		printf("================= Parsed =================\n");
		PrintProc(&ROOT_PROC);
		for(var = VarFirst(); var != NULL; var = VarNext(var)) {
			type = var->type;
			if (type != NULL && type->variant == TYPE_PROC && var->instr != NULL) {
				printf("---------------------------------------------");
				PrintVar(var);
				printf("\n\n");
				PrintProc(var);
			}
		}
	}

	//***** Translation

	for(var = VarFirst(); var != NULL; var = VarNext(var)) {
		type = var->type;
		if (type != NULL && type->variant == TYPE_PROC && var->instr != NULL) {
//			PrintProc(var);
			ProcTranslate(var);
		}
	}

	ProcTranslate(&ROOT_PROC);

	//***** Optimalization

	Optimize(&ROOT_PROC);

	if (VERBOSE) {
		printf("============== Optimized ==============\n");
		PrintProc(&ROOT_PROC);
	}

	VarUse();

	//==== Assign offsets to structure items
	for(var = VarFirst(); var != NULL; var = VarNext(var)) {
		// Compute offsets for structure members
		if (var->type != NULL && var->type->variant == TYPE_STRUCT && var->type->owner == var) {
			TypeStructAssignOffsets(var->type);
		}
	}

	//==== Assign addresses to variables

	adr = 128;
	n = 1;
	for(var = VarFirst(); var != NULL; var = VarNext(var)) {

		if ((var->write > 0 || var->read > 0) && var->type != NULL && !VarIsLabel(var)) {
			tv = var->type->variant;
			if (var->adr == NULL && (var->mode == MODE_VAR || var->mode == MODE_ARG)) {
				size = TypeSize(var->type);		

				if (size > 0) {
					var->adr = VarNewInt(adr);
					adr += size;
				}
			}
		}
		n++;
	}

	if (TOK == TOKEN_ERROR) goto failure;

	EmitOpen(filename);

	if (VERBOSE) {
/*
		printf("==== Types\n");
		for(var = VarFirst(); var != NULL; var = VarNext(var)) {
			if (var->adr == ADR_VOID && !var->value_nonempty && var->scope == &ROOT_PROC) {
				PrintVar(var);
			}
		}

		printf("==== Constants\n");
		for(var = VarFirst(); var != NULL; var = VarNext(var)) {
			if (VarIsConst(var)) {
				if (var->name != NULL) {
					PrintVar(var);
				}
			}
		}
*/
		printf("==== Variables\n");

		for(var = VarFirst(); var != NULL; var = VarNext(var)) {
			if (var->write >= 1) {
				PrintVar(var);
			}
		}

		printf("==== Output\n");
	} // verbose

	EmitLabels();
	if (!EmitProc(&ROOT_PROC)) goto failure;
	EmitProcedures();
	EmitAsmIncludes();
	EmitClose();

	//==== Call the assembler

	strcpy(path, PROJECT_DIR);
	strcat(path, filename);

	PlatformPath(command);
	len = StrLen(command);

	result = 0;
	if (assembler) {
#if defined(__UNIX__)
		sprintf(command+len, MADS_COMMAND, path, path, path);
		result = system(command);
#else
		sprintf(command+len, MADS_COMMAND, path, path, path);
		result = system(command);
#endif
	}

done:	
 	exit(result);

failure:
	result = -2;
  	goto done;
}
