#include "../language.h"

void SrcFilePrint(Cell * cell)
{
	SrcLineBlock * blk;
	SrcLine * line;
	UInt32 line_no;
	UInt16 line_idx;

	for(blk = cell->src_file.lines; blk != NULL; blk = blk->next) {
		line_no = blk->line_no;
		for(line_idx = 0, line = blk->lines; line_idx < LINES_PER_BLOCK; line_idx++, line++) {
			if (line->txt == NULL) break;
			PrintFmt("%4d: ", line_no);
			Print(line->txt);
			PrintEOL();

			line_no++;
		}
	}
}

LineNo LineNumber(SrcLine * line)
{
	UInt16 offset = line - line->block->lines;
	return line->block->line_no + offset;
}

char * LineFileName(SrcLine * line)
{
	return VarName(line->block->file->src_file.file_var);
}
