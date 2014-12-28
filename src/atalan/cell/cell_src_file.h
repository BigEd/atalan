typedef struct SrcLineTag SrcLine;
typedef struct SrcLineBlockTag SrcLineBlock;

#define LINES_PER_BLOCK 64

struct SrcLineTag {
	SrcLineBlock * block;
	char * txt;					// UTF8 encoded, zero ended text
};

struct SrcLineBlockTag {
	SrcLineBlock * next;
	UInt32  line_no;		//  number of the first line
	Cell * file;
	SrcLine lines[LINES_PER_BLOCK];
};

typedef struct {
	SrcLineBlock  * lines;
	UInt32 line_count;
	Cell * file_var;
} SrcFileCell;

void SrcFilePrint(Cell * cell);

LineNo LineNumber(SrcLine * line);
char * LineFileName(SrcLine * line);
