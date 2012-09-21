%{
#include "obj.tab.h"
%}

digit       [0-9]
integer     -?{digit}*
real        {integer}?(\.{digit}*)?([eE]{integer})?

%%

^"#".*      ;
[ \t]+      ;
\n          return NEWLINE;  
real        { yylval = strtof(yytext, NULL); return REAL; }
integer     { yylval = strtol(yytext, NULL, 10); return INTEGER; }
"/"         return SLASH;
^f          return FACE;
^v          return VERTEX;
^vt         return TEXCOORD;
^vn         return NORMAL;
^vp         return FREEFORM;
