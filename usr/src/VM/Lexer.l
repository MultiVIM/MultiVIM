%{
#include "VM/NewCompiler.h"
#include "Parser.tab.h"

#define YY_NO_UNISTD_H
#define isatty(X) NULL
%}

%option noyywrap
%option stack
%option yylineno
%option reentrant
%option extra-type = "MVST_Parser *"
%option prefix = "mvst"
%option caseless

%{

#define YY_USER_ACTION \
    yyextra->recOldPos(); \
    for(int i = 0; yytext[i] != '\0'; i++) \
    { \
        if(yytext[i] == '\n') \
        { \
            yyextra->cr(); \
        } \
        else \
            yyextra->incCol(); \
    }

#define p(x) yyextra->parse(TOK_##x)
#define ps(x) yyextra->parse(TOK_##x, yytext)
#define pi(x) yyextra->parse(TOK_##x, atoi(yytext))

%}

DIGIT ([0-9])
LTR ([a-zA-Z_\xA8\xAA\xAD\xAF\xB2\xB5\xB7\xB8\xB9\xBA\xBC\xBD\xBE]|[\xC0-\xFF][\x80-\xBF]*|\\u([0-9a-fA-F]{4}))
IDENT (:?{LTR})(:?{LTR}|{DIGIT}|::)*
NEWLINE ((\n)|(\r)|(\r\n))
COMMENT \"(\\.|[^"])*\"

%%

"_"{NEWLINE}        { /* line continuation */ }
{NEWLINE}           {} //{ yyextra->parse(TOK_EOL); }

":="                                { p(ASSIGN); }
"<-"                                { p(ASSIGN); }

"subclass:"                         { p(SUBCLASSCOLON); }

["][^"]*["]                            { /* ignore comment */ }

$[^ \t\n.()\[]                      { ps(CHAR); }
#[^ \t\n.()\[]+                     { ps(SYMBOL); }
[0-9]+r-?[0-9A-Z]+(\.[0-9A-Z]+)?(e[-+]?[0-9]+)?     { /* pf(FLOAT) */ }
[0-9]+                                              { pi(INTEGER); }
[0-9]+(\.[0-9]+)?(e[-+]?[0-9]+)?                    { /* pf(FLOAT) */ }
:(:?{LTR})(:?{LTR}|{DIGIT}|::)*     { ps(COLONVAR); }
(:?{LTR})(:?{LTR}|{DIGIT}|::)*:     { ps(KEYWORD); }
{IDENT}"."{IDENT}                   { ps(NAMESPACENAME); }
(:?{LTR})(:?{LTR}|{DIGIT}|::)*      { ps(IDENTIFIER); }
['][^']*[']                         { ps(STRING); }

"<"[0-9]+                       { yyextra->parse(TOK_PRIMNUM, atoi(yytext + 1)); }

"^"                             { p(UP); }
"."                             { p(DOT); }
"|"                             { p(BAR); }
";"                             { p(SEMICOLON); }
"("                             { p(LBRACKET); }
")"                             { p(RBRACKET); }
"["                             { p(SQB_OPEN); }
"]"                             { p(SQB_CLOSE); }
"<"                             { p(LCARET); }
">"                             { p(RCARET); }
"#"                             { p(HASH); }

[^ \t{LTR}]                     { printf("GIVE BINARY %s\n", yytext); ps(BINARY); }


 /* ":="                            {//return(ASSIGN);}
"<-"                            {//return(ASSIGN);}
Class                           //{return(lexsave(CLASS));}
self                            {yylval.p = selfvar;  return(PSEUDO);}
selfProcess			{yylval.p = procvar;  return(PSEUDO);}
super                           {yylval.p = supervar; return(PSEUDO);}
nil                             {yylval.p = nilvar;   return(PSEUDO);}
true                            {yylval.p = truevar;  return(PSEUDO);}
false                           {yylval.p = falsevar; return(PSEUDO);}
smalltalk                       {yylval.p = smallvar; return(PSEUDO);}
\$.                             {yylval.i = yytext[1]; return(LITCHAR);}
#                               {return(PS);}
[0-9]+r-?[0-9A-Z]+(\.[0-9A-Z]+)?(e[-+]?[0-9]+)? {return(lexsave(LITFNUM));}
[0-9]+                          {yylval.i = atoi(yytext); return(LITNUM);}
[0-9]+(\.[0-9]+)?(e[-+]?[0-9]+)?   {return(lexsave(LITFNUM));}
'[^']*'                         {char c; lexunput(c = lexinput());
                                 if (c == '\'') yymore();
                                 else return(lexlstr());}
[a-zA-Z0-9]+:?                  {return(varlex());}
:[a-zA-Z0-9]+                   {return(slexsave(COLONVAR));}
#[^ \t\n.()\[]+                 {return(slexsave(LITSYM));}
"-"                             {return(lexsave(MINUS));}
"("                             {return(LP);}
")"                             {return(RP);}
"["                             {return(LB);}
"]"                             {return(RB);}
"."                             {return(PERIOD);}
^"|"				{return(lexsave(MBAR));}
^"!"				{return(lexsave(MBAR));}
"|"                             {return(lexsave(BAR));}
"!"                             {return(lexsave(BAR));}
";"                             {return(SEMI);}
"^"                             {return(lexsave(UPARROW));}
">"				{return(lexsave(PE));}
[^ \t\nA-Za-z0-9]               {return(lexsave(BINARY));}
"<primitive"   			{return(PRIMITIVE);}
"<"[a-zA-Z0-9]+			{yylval.i = prim_number(&yytext[1]); return(NAMEDPRIM);}*/
