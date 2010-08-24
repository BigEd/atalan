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
GLOBAL Bool VERBOSE;

extern Var   ROOT_PROC;
extern Var * INSTRSET;		// enumerator with instructions
extern InstrBlock * CODE;
Var * INTERRUPT;

int main(int argc, char *argv[])
{
	Var * var;
	Var * data;
	Type * type;
	Int32 adr;
	Int16 i = 1;
	TypeVariant tv;
	Bool assembler = true;
	int result = 0;
	UInt32 size;
	UInt32 n;
	char filename[128], command[128], path[2048];
	UInt16 filename_len;
	char * last_slash, * s;

//	FILE * asmf;

	VERBOSE = false;
	SYSTEM_DIR[0]='\0';

	InitErrors();

	//
    // Check arguments.
    //

	printf("Atalan programming language compiler (23-Aug-2010)\nby Rudla Kudla (http:\\atalan.kutululu.org)\n\n");

	while (i < argc) {
		if (StrEqual(argv[i], "-v")) {
			VERBOSE = true;
		} else if (StrEqual(argv[i], "-a")) {
			assembler = false;
		} else if (StrEqual(argv[i], "-I")) {
			i++;
			if (i<argc)
				if (strlen(argv[i])<1022) {
					strcpy(SYSTEM_DIR,argv[i]);
					s = &SYSTEM_DIR[strlen(SYSTEM_DIR)-1];
					if (*s!=DIRSEP)
					{
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

    if (i == argc) {
        fprintf(STDERR, "Usage:\n"
	"%s [-v][-a] file\n"
	"  -v Verbose output\n"
	"  -I <SYSTEM_DIR> define include path (default: current catalog)\n"
	"  -a Only generate assembler source code, but do not call assembler\n", argv[0]);
        exit(-1);
    }

	// If the filename does not have .atl extension, add it
	// Also get the name of the file, so we can generate output with same name

	strcpy(filename, argv[i]);
	filename_len = StrLen(filename);

	if (filename_len > 4 && StrEqual(".atl", &filename[filename_len-4])) {
//		strcpy(&filename[filename_len], ".atl");
//	} else {
		filename_len -= 4;
		filename[filename_len] = 0;
	}

	//==== Split dir and filename

	strcpy(PROJECT_DIR, filename);
	last_slash = 0;
	for(s = PROJECT_DIR; *s != 0; s++) {
		if (*s == '/' || *s == '\\') last_slash = s+1;
	}
	strcpy(filename, last_slash);
	if (last_slash != NULL) *last_slash = 0;


	printf("Building %s%s.atl\n", PROJECT_DIR, filename);

	//===== Initialize

	VarInit();
	InstrInit();
	ParseInit();
	LexerInit();

	data = VarNewTmpLabel();

	//==== Initialize system dir

	// system.atl is file defining some ATALAN basics.
	// It must always be included.
	// Some of the definitions in system.atl are directly used by compiler.

	if (!Parse("system")) goto failure;

	
	INSTRSET  = VarFindScope(&ROOT_PROC, "instrs", 0);
	REGSET    = VarFindScope(&ROOT_PROC, "regset", 0);
	INTERRUPT = VarFindScope(&ROOT_PROC, "interrupt", 0);

	if (!Parse("p_6502")) goto failure;
	if (!Parse("atari")) goto failure;

	VarInitRegisters();

	// Parse the file. This also generates main body of the program (_ROOT procedure).
	// TODO: Root procedure may be just specifal type of procedure.
	//       Prologue and epilogue may be replaced by proc type specific PROC and ANDPROC instructions.

	Gen(INSTR_PROLOGUE, NULL, NULL, NULL);
	SYSTEM_PARSE = false;
	if (!Parse(filename)) goto failure;
	SYSTEM_PARSE = true;
	Gen(INSTR_EPILOGUE, NULL, NULL, NULL);

	ROOT_PROC.instr = CODE;
	VarGenerateArrays();

	if (VERBOSE) {
		printf("==== Instructions\n");
		PrintProc(&ROOT_PROC);
		for(var = VarFirst(); var != NULL; var = VarNext(var)) {
			type = var->type;
			if (type != NULL && type->variant == TYPE_PROC && var->instr != NULL) {
				printf(";===================================================\n");
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

	if (VERBOSE) {
		printf("==== Translated\n");
		PrintProc(&ROOT_PROC);
	}

	//***** Optimalization

	Optimize(&ROOT_PROC);

	if (VERBOSE) {
		printf("==== Optimized\n");
		PrintProc(&ROOT_PROC);
	}

	VarUse();

	//==== Assign addresses to variables

	adr = 128;
	n = 1;
	for(var = VarFirst(); var != NULL; var = VarNext(var)) {

//		if (var->name != NULL && _stricmp(var->name, "i") == 0) {
//			printf("Ho!\n");
//		}
//		if (var->idx == 100) {
//			n = n;
//		}

//		if ((var->name != NULL) && (strcmp("i", var->name) == 0) && (var->scope->mode == MODE_SCOPE) && (var->scope->idx == 6)) {
//			PrintVar(var);
//		}

		if ((var->write > 0 || var->read > 0) && var->type != NULL && !VarIsLabel(var)) {
			tv = var->type->variant;
			if (var->adr == NULL && (var->mode == MODE_VAR || var->mode == MODE_ARG)) {
				size = 0;
				if (tv == TYPE_INT) {
					size = var->type->range.max - var->type->range.min;
//					if (size == 0) size = 0;
					if (size <= 255) size = 1;
					else if (size <= 65535) size = 2;
					else if (size <= 0xffffff) size = 3;
					else size = 4;		// we currently do not support bigger numbers than 4 byte integers

					if (size == 0) {
						InternalError("Size of used variable is 0");
					}
				} else if (tv == TYPE_ADR) {
					size = AdrSize();
				}

				if (size > 0) {
					var->adr = VarNewInt(adr);
					adr += size;
				}
			}
		}
		n++;
	}

	if (TOK == TOKEN_ERROR) goto failure;

//	strcpy(&filename[filename_len], ".asm");
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
	EmitClose();

	//==== Call the assembler

	strcpy(path, PROJECT_DIR);
	strcat(path, filename);

	if (assembler) {
#if SYSTEM_NAME == Darwin
//		filename[filename_len] = 0;
		sprintf(command, MADS_COMMAND, path, path, path);
		result = system(command);
		if (result != 0) goto asm_failure;
#else
//		asmf = fopen("mads.exe", "rb");
//		if (asmf != NULL) {
//			fclose(asmf);
//			filename[filename_len] = 0;
			sprintf(command, MADS_COMMAND, path, path, path);
			result = system(command);
			if (result != 0) goto asm_failure;
//		}
#endif
	}
	
 	exit(0);

failure:
	result = -2;
asm_failure:
  	exit(result);
}
