/*********************************************************

  Error reporting

**********************************************************/

extern UInt32 ERROR_CNT;
extern UInt32 LOGIC_ERROR_CNT;

void ErrArgClear();
void ErrArg(Var * var);
void SyntaxErrorBmk(char * text, Bookmark bookmark);
void SyntaxError(char * text);
void LogicWarning(char * text, Bookmark bookmark);
void LogicWarningLoc(char * text, Loc * loc);
void LogicError(char * text, Bookmark bookmark);
void LogicErrorLoc(char * text, Loc * loc);
void InternalError(char * text, ...);
void InternalErrorLoc(char * text, Loc * loc);
void Warning(char * text);
void EndErrorReport();

void InitErrors();

void PlatformError(char * text);
#define OK  (TOK != TOKEN_ERROR)
#define ifok if (OK)
#define iferr if (TOK == TOKEN_ERROR)
