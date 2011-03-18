/*

ATALAN - Programming language Compiler for embedded systems


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

extern Var  ROOT_PROC;
extern Var * INSTRSET;		// enumerator with instructions
extern InstrBlock * CODE;
Var * INTERRUPT;
Var * SYSTEM_SCOPE;
Var * MACRO_PRINT;
Var * MACRO_ASSERT;
Var * MACRO_FORMAT;
char PLATFORM[64];			// name of platform
UInt8 OPTIMIZE;
char VERBOSE_PROC[128];		// name of procedure which should generate verbose output

Bool Verbose(Var * proc)
/*
Purpose:
	Return true if we are supposed to be verbose.
	Verbose may be specified for specific procedure.
	NULL is available value.
*/
{
	if (proc != NULL && *VERBOSE_PROC != 0) {
		return StrEqual(proc->name, VERBOSE_PROC);
	}
	return VERBOSE;
}

void ProcessUsedProc(void (*process)(Var * proc))
{
	Var * var;
	Type * type;

	for(var = VarFirst(); var != NULL; var = VarNext(var)) {
		type = var->type;
		if (type != NULL && type->variant == TYPE_PROC && var->read > 0 && var->instr != NULL) {
			process(var);
		}
	}
	process(&ROOT_PROC);
}

void InitPlatform()
/*
Purpose:
	This procedure is called after the platform file has been parsed.
*/
{
	MACRO_PRINT  = VarFindScope(&ROOT_PROC, "std_print", 0);			// TODO: Screen print
	MACRO_FORMAT = VarFindScope(&ROOT_PROC, "std_format", 0);			// TODO: Memory print
	MACRO_ASSERT =  VarFindScope(SYSTEM_SCOPE, "print_assert", 0);

	VarInitRegisters();
}

int main(int argc, char *argv[])
{
	Var * var, * data;
	Type * type;
	Int16 i;
	Bool assembler = true;
	Bool header = true;
	int result = 0;
	char filename[MAX_PATH_LEN], command[MAX_PATH_LEN], path[MAX_PATH_LEN];
	UInt16 filename_len;
	char * s;
	Bool header_out;
	Var * system_scope;
	char * platform = NULL;

	*VERBOSE_PROC = 0;

//	StrCopy(VERBOSE_PROC, "root");
#ifdef DEBUG
//	HeapUnitTest();
#endif

	VERBOSE = false;

	PrintColor(RED+GREEN+BLUE);

	*PLATFORM = 0;

	//TODO: Remove default platform
	// Default platform is "atari"

//	strcpy(PLATFORM, "atari");

	// System folder is parent directory of directory where the binary is stored
	//
	//  bin/
	//      atalan.exe
	//      mads.exe
	//  modules/
	//		system.atl
	//      ;platform independent modules
	//  platforms/
	//      atari/
	//         ;platform dependent modules
	//      c64/
	//  processors/
	//      m6502/

	GetApplicationDir(argv[0], SYSTEM_DIR);
	PathParent(SYSTEM_DIR);
	OPTIMIZE = 255;

	InitErrors();

	//
    // Check arguments.
    //

	i = 1;
	while (i < argc) {		
		if (StrEqual(argv[i], "-V0")) {
			header = false;
		} else if (StrEqual(argv[i], "-V")) {
			VERBOSE = true;
		} else if (StrEqual(argv[i], "-A")) {
			assembler = false;
		} else if (StrEqual(argv[i], "-O0")) {
			OPTIMIZE = 0;
		} else if (StrEqual(argv[i], "-O")) {
			i++;
			if (i<argc) {
				OPTIMIZE = argv[i][0] - '0';
			}
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
		} else if (StrEqual(argv[i], "-P")) {
			i++;
			if (i<argc) {
				platform = argv[i];
			}
		} else {
			break;
		}
		i++;
	}

	if (header) {
		printf("Atalan programming language compiler (19-Feb-2011)\nby Rudla Kudla (http:\\atalan.kutululu.org)\n\n");
	}

    if (i == argc) {
        fprintf(STDERR, "Usage:\n"
	"%s [options] file\n"
	"  -v Verbose output\n"
	"  -I <SYSTEM_DIR> define include path (default: current catalog)\n"
	"  -a Only generate assembler source code, but do not call assembler\n"
	"  -p <name>  Platform to use\n"
	"  -o <num>   Optimization level (0..9) 0 = no optimization\n"
	, argv[0]);
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

	INSTRSET = NULL;
	if (!Parse("system", false)) goto failure;
	
	REGSET    = VarFindScope(&ROOT_PROC, "regset", 0);
	INTERRUPT = VarFindScope(&ROOT_PROC, "interrupt", 0);
	system_scope = VarFindScope(&ROOT_PROC, "system", 0);

	// If the platform has been specified as an argument, parse it
	if (platform != NULL) {
		if (!Parse(platform, false)) goto failure;
	}

	//TODO: Read the var heap definition from configuration

	// Parse the file. This also generates main body of the program (_ROOT procedure).
	// TODO: Root procedure may be just special type of procedure.
	//       Prologue and epilogue may be replaced by proc type specific PROC and ANDPROC instructions.

	Gen(INSTR_PROLOGUE, NULL, NULL, NULL);
	SYSTEM_PARSE = false;
	if (!Parse(filename, true)) goto failure;
	SYSTEM_PARSE = true;

	if (*PLATFORM == 0) {
		SyntaxError("No target platform defined");
		goto failure;
	}

	var = VarFindScope(&ROOT_PROC, "varheap", 0);
	if (var != NULL) {
		HeapAddType(&VAR_HEAP, var->type);
	} else {
		InternalError("Platform does not define varheap");
		goto done;
	}

	Gen(INSTR_EPILOGUE, NULL, NULL, NULL);

	// Report warning about logical errors

	if (LOGIC_ERROR_CNT > 0 && ERROR_CNT == 0) {
		Warning("There were logical errors.\nCompilation will proceed, but the resulting program may be errorneous.");
	}

	ROOT_PROC.instr = CODE;

	VarUse();
	ProcUse(&ROOT_PROC, 0);

	// Now do extra checks in all procedures
	// Some of the checks are postponed to have information from all procedures

	header_out = false;

	for(var = VarFirst(); var != NULL; var = VarNext(var)) {
		type = var->type;
		if (type != NULL && type->variant == TYPE_PROC && var->instr != NULL && var->read > 0) {
			if (Verbose(var)) {
				if (!header_out) {
					PrintHeader("Parsed");
					header_out = true;
				}

				printf("---------------------------------------------\n");
				PrintVar(var);
				printf("\n\n");
				PrintProc(var);
			}
		}
	}

	if (Verbose(&ROOT_PROC)) {
		if (!header_out) {
			PrintHeader("Parsed");
			header_out = true;
		}
		PrintProc(&ROOT_PROC);
	}

	if (ERROR_CNT > 0) goto failure;

	VarUse();
	VarGenerateArrays();

	if (OPTIMIZE > 0) {
		// It is important to call the inline optimization before the code is broken to basic blocks.
		// It makes code inserting easier.
		ProcessUsedProc(ProcInline);
	}

	//***** Analysis
	ProcessUsedProc(GenerateBasicBlocks);
	ProcessUsedProc(CheckValues);


	//***** Translation
	ProcessUsedProc(ProcTranslate);

	//***** Optimization

	// We must generate basic blocks again, as translation may have generated labels and jumps
	ProcessUsedProc(GenerateBasicBlocks);		

	if (OPTIMIZE > 0) {
		ProcessUsedProc(OptimizeJumps);
		ProcessUsedProc(ProcOptimize);
		ProcessUsedProc(DeadCodeElimination);

		if (Verbose(NULL)) {
			PrintHeader("Optimized");
			PrintProc(&ROOT_PROC);
		}
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

	ProcessUsedProc(ProcClearProcessed);
	ProcessUsedProc(AllocateVariables);

	if (TOK == TOKEN_ERROR) goto failure;

	EmitOpen(filename);

	if (Verbose(NULL)) {

		PrintHeader("Variables");

		for(var = VarFirst(); var != NULL; var = VarNext(var)) {
			if (var->write >= 1) {
				PrintVar(var);
			}
		}

		PrintHeader("Output");
	} // verbose

	EmitLabels();

	//TODO: ProcessUsedProc
	if (!EmitProc(&ROOT_PROC)) goto failure;
	EmitProcedures();
	EmitAsmIncludes();	
	EmitInstrOp(INSTR_CODE_END, NULL, NULL, NULL);
	VarEmitAlloc();
	EmitClose();

	//==== Call the assembler
	//     The command to call is defined using rule for INSTR_COMPILER.
	//     Argument is filename (without extension) of the compiled file.

	result = 0;
	if (assembler) {	
		PathMerge(path, PROJECT_DIR, filename);
		var = VarFind("BIN_EXTENSION", 0);
		EmitOpenBuffer(command);

// On Windows, system command requires extra set of parentheses around whole command to correctly support
// quoted exe.

#ifdef __Windows__	
		EmitChar('\"');
#endif
		EmitInstrOp(INSTR_COMPILER, VarNewStr(path), VarNewStr(var->str), NULL);
#ifdef __Windows__	
		EmitChar('\"');
#endif
		EmitCloseBuffer();
		result = system(command);
	}

done:	
   	exit(result);

failure:
	result = 2;
  	goto done;
}
