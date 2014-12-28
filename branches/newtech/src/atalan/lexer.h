typedef UInt16 LineNo;   
typedef UInt16 LinePos;  // 0 based position of character on line

Bool SrcOpen(char * name, UInt16 blk_type, Bool parse_options);
void SrcClose();
FILE * FindFile(char * name, char * ext, char * path);

Bool LexWord(UInt8 * text);
Bool LexSymbol(UInt8 * text);
Bool LexSymbol2(UInt8 * text, UInt8 * text2);
Bool LexPrefix(UInt8 * text);
Bool LexId(char * p_name);
Bool LexString(UInt8 * p_str);
Bool LexInt(Var ** p_i);
Bool LexNum(Var ** p_i);
Bool LexText(Var ** p_i);
Bool LexComment();
Bool LexEOL();
Bool LexPeekSymbol(UInt8 * text);
Bool LexSpaces();
Bool LexEOF();

UInt16 LexStringPart(UInt8 * text, UInt8 max_len);

Bool LexIndent();
Bool LexOutdent();

typedef enum {
	BLOCK_FILE = 0,
	BLOCK_LINE = 1,
	BLOCK_BRACES = 2,
	BLOCK_INDENT = 3,
	BLOCK_NONE = 4
} BLOCK_TYPE;

UInt8 LexBlock();
Bool LexBlockSeparator(UInt8 blk_type);
Bool LexBlockEnd(UInt8 type);
