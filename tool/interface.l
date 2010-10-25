
%{
#include "interface.tab.h"
extern int yyerror(const char *);
%}

%option noyywrap
%option 8bit
%option yylineno

%%

"module"               { return MODULE; }
"message"              { return MESSAGE; }
"int8"                 { yylval.i = INT8;   return INT8;   }
"int16"                { yylval.i = INT16;  return INT16;  }
"int32"                { yylval.i = INT32;  return INT32;  }
"int64"                { yylval.i = INT64;  return INT64;  }
"uint8"                { yylval.i = UINT8;  return UINT8;  }
"uint16"               { yylval.i = UINT16; return UINT16; }
"uint32"               { yylval.i = UINT32; return UINT32; }
"uint64"               { yylval.i = UINT64; return UINT64; }
"string"               { yylval.i = STRING; return STRING; }
"float"                { yylval.i = FLOAT;  return FLOAT;  }
"double"               { yylval.i = DOUBLE; return DOUBLE; }
"{"                    { return '{'; }
"}"                    { return '}'; }
"<"                    { return '<'; }
">"                    { return '>'; }
"["                    { return '['; }
"]"                    { return ']'; }
[1-9][0-9]*            { yylval.i = strtol(yytext, NULL, 0); return NUMBER; }
0[Xx][0-9a-fA-F]*      { yylval.i = strtol(yytext, NULL, 0); return NUMBER; }
[_a-zA-Z][_a-zA-Z0-9]* { strncpy(yylval.identifier, yytext, sizeof(yylval.identifier)-1); return IDENTIFIER; }
" "                    {}
"\t"                   {}
"\n"                   {}
.                      { yyerror(yytext); }

%%

