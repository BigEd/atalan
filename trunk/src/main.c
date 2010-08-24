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


void PrintVar(Var * var)
{
	Type * type;
	Var * arg;

	if (FlagOn(var->submode, SUBMODE_REF)) {
		printf("@");
	}

	if (var->mode == MODE_ELEMENT) {
		PrintVarName(var->adr);
		printf("(");
		PrintVar(var->var);
		printf(")");
	} else if (var->mode == MODE_CONST) {
		printf("%ld", var->n);
		return;
	} else {

		PrintVarName(var);

		if (var->adr != NULL) {
			printf("@");
			PrintVarVal(var->adr);
		}

		type = var->type;
		if (type != NULL) {
			if (type->variant == TYPE_PROC) {
				printf(":proc");
			} else if (type->variant == TYPE_MACRO) {
				printf(":macro");
			}
			printf("(");
			for(arg = var->next; arg != NULL; arg = arg->next) {
				if (arg->mode == MODE_ARG && arg->scope == var) {
					printf(" %s", arg->name);
				}
			}
			printf(")");
		}

		if (VarIsConst(var)) {
			printf(" = %ld", var->n);
		} else {
			type = var->type;
			if (type != NULL) {
				if (type->variant == TYPE_INT) {
					if (type->range.min == 0 && type->range.min == 255) {
					} else {
						printf(":%ld..%ld", type->range.min, type->range.max);
					}
				}
			}
		}
	}
	printf("  R%ld W%ld\n", var->read, var->write);
}

extern InstrBlock * CODE;

int main(int argc, char *argv[])
{
	Var * var;
	Var * data;
	Type * type;
	Int32 adr;
	Int16 i = 1;
	TypeVariant tv;
	FILE * asmf;
	Bool assembler = true;
	int result = 0;
	UInt32 size;
	UInt32 n;
	char filename[128], command[128], include_path[1024], path[2048];
	UInt16 filename_len;

	VERBOSE = false;
	include_path[0]='\0';

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
					strcpy(include_path,argv[i]);
					char * t = & include_path[strlen(include_path)-1];
					if (*t!=DIRSEP)
					{
						t++;
						*t++=DIRSEP;
						*t='\0';
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
	"  -I <include_path> define include path (default: current catalog)\n"
	"  -a Only generate assembler source code, but do not call assembleri\n", argv[0]);
        exit(-1);
    }

	// If the filename does not have .atl extension, add it
	// Also get the name of the file, so we can generate output with same name

	strcpy(filename, argv[i]);
	filename_len = StrLen(filename);

	if (filename_len < 4 || !StrEqual(".atl", &filename[filename_len-4])) {
		strcpy(&filename[filename_len], ".atl");
	} else {
		filename_len -= 4;
	}

	printf("Building %s\n", filename);

	VarInit();
	InstrInit();
	ParseInit();
	LexerInit();

	data = VarNewTmpLabel();

	// system.atl is file defining some ATALAN basics.
	// It must always be included.
	// Some of the definitions in system.atl are directly used by compiler.

	strcpy(path,include_path);
	strcat(path,"system.atl");
	if (!Parse(path)) goto failure;
	
	INSTRSET = VarFindScope(&ROOT_PROC, "instrs", 0);
	REGSET = VarFindScope(&ROOT_PROC, "regset", 0);

	strcpy(path,include_path);
	strcat(path,"p_6502.atl");
	if (!Parse(path)) goto failure;

	strcpy(path,include_path);
	strcat(path,"atari.atl");
	if (!Parse(path)) goto failure;

	VarInitRegisters();

	// Parse the file. This also generates main body of the program (_ROOT procedure).
	// TODO: Root procedure may be just specifal type of procedure.
	//       Prologue and epilogue may be replaced by proc type specific PROC and ANDPROC instructions.

	Gen(INSTR_PROLOGUE, NULL, NULL, NULL);
	SYSTEM_PARSE = false;
	if (!Parse(filename)) goto failure;
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

	strcpy(&filename[filename_len], ".asm");
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

	if (assembler) {
#if SYSTEM_NAME == Darwin
		filename[filename_len] = 0;
		sprintf(command, MADS_COMMAND, filename, filename);
		result = system(command);
		if (result != 0) goto asm_failure;
#else
		asmf = fopen("mads.exe", "rb");
		if (asmf != NULL) {
			fclose(asmf);
			filename[filename_len] = 0;
			sprintf(command, MADS_COMMAND, filename, filename);
			result = system(command);
			if (result != 0) goto asm_failure;
		}
#endif
	}
	
 	exit(0);

failure:
	result = -2;
asm_failure:
  	exit(result);
}
