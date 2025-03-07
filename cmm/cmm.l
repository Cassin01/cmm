%{
#undef ECHO
#define ECHO printf("%s", yytext);
%}
%%
"var"     { ECHO;  return VAR; }
"main"    { ECHO;  return MAIN; }
"("       { ECHO;  return LPAR; }
")"       { ECHO;  return RPAR; }
","       { ECHO;  return COMMA; }
"{"       { ECHO;  return LBRA; }
"}"       { ECHO;  return RBRA; }
"write"   { ECHO;  return WRITE; }
"writeln" { ECHO;  return WRITELN; }
";"       { ECHO;  return SEMI; }
"+"       { ECHO;  return PLUS; }
"-"       { ECHO;  return MINUS; }
"*"       { ECHO;  return MULT; }
"/"       { ECHO;  return DIV; }
"%"       { ECHO;  return MOD; }
"^"       { ECHO;  return POW; }
"if"      { ECHO;  return IF; }
"then"    { ECHO;  return THEN; }
"else"    { ECHO;  return ELSE; }
"endif"   { ECHO;  return ENDIF; }
"while"   { ECHO;  return WHILE; }
"for"     { ECHO;  return FOR; }
"do"      { ECHO;  return DO; }
"goto"    { ECHO;  return GOTO; }
"read"    { ECHO;  return READ; }
"return"  { ECHO;  return RETURN; }
"switch"  { ECHO;  return SWITCH; }
"_"       { ECHO;  return DEFAULT; }
":="      { ECHO;  return COLEQ; }
":"       { ECHO;  return COLON; }
">="      { ECHO;  return GE; }
">"       { ECHO;  return GT; }
"<="      { ECHO;  return LE; }
"<"       { ECHO;  return LT; }
"!="      { ECHO;  return NE; }
"=="      { ECHO;  return EQ; }
"&&"      { ECHO;  return AND; }
"||"      { ECHO;  return OR; }
"!"       { ECHO;  return NOT; }
[0-9]+ {
    ECHO;
    yylval.val = atoi(yytext);
    return NUMBER;
}
[a-z][a-z0-9]* {
    ECHO;
    yylval.name = (char*)malloc(strlen(yytext)+1);
    strcpy(yylval.name, yytext);
    return ID;
}
[ \t\n]+ { ECHO;  /* ignored */ }
.        { ECHO;  return yytext[0]; }
