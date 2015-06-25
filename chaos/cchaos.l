/* forth.cc - simple forth interpreter as a level 1 test
Copyright (C) 1998,1999 Colin McCormack, Jordan B. Baker
see LICENSE (MD5 d7a11a4148f3858e544bf89cdde91727) for terms
*/
%{
#include "cchaos.hh"
%}
%option yyclass="CChaos"
/*%option always-interactive*/

%x str
DIGIT	[0-9]
HEX	0[xX][0-9a-fA-F]+
VAR	[$][a-zA-Z][a-zA-Z0-9_]*
STR	([^ \t\n{$'\0][^ \t\n\0]*)
QSTR	["]([^"]|(\\["]))*["]
NQSTR	([^"]|(\\["]))+
ID	[\.a-zA-Z0-9_=<>+*/!@%&|^#()-[\]]+
OBRACE	[{]
CBRACE	[}]
%%

{DIGIT}+[.]{DIGIT}[Ee]?{DIGIT}* {
    Real(YYText());
}

[-]{DIGIT}+[.]{DIGIT}[Ee]?{DIGIT}* {
    Real(YYText());
}

{DIGIT}+ {
    Digit(YYText());
}

[-]{DIGIT}+ {
    Digit(YYText());
}

{HEX} {
    Digit(YYText());
}

{OBRACE} {
    if (!string) {
       string = (char*)malloc(1024);
       strl = 1024;
    }
    sp = string;
    paren = 1;
    BEGIN(str);
}

<str>{OBRACE} {
    paren++;
    if ((sp - string) >= strl) {
       sp -= (int)string;
       string = (char*)realloc(string, strl += 1024);
       sp += (int)string;
    }
    *sp++ = '{';
}

<str>{CBRACE} {
    if (!--paren) {
       *sp = '\0';
       BString(string);
       BEGIN(INITIAL);
    } else {
       if ((sp - string) >= strl) {
          sp -= (int)string;
          string = (char*)realloc(string, strl += 1024);
          sp += (int)string;
       }
       *sp++ = '}';
    }
}

<str>\n {
    // ignore
}

<str>. {
    if ((sp - string) >= strl) {
       sp -= (int)string;
       string = (char*)realloc(string, strl += 1024);
       sp += (int)string;
    }
    if (YYText()[0] == '"') {
	*sp++ = '\\';
    }
      *sp++ = YYText()[0];
}

"'"{STR}             { BString(1+YYText()); }
"$"{STR}             { Var(1+YYText());   }
(@)|(get)            { VarPop();      }
"fn"                 { FnPop();       }

{QSTR}               { String(YYText());  }

{ID}                 { Fn(YYText());      }

[ \t\n]+             /* eat up whitespace */  
<<EOF>>              { yyterminate();       } 
"\0"                 { /* I have no idea why this keeps getting NULs */ } 
.                    { cerr << "Unrecognized character: " << YYText() << '\n'; }

%% 
/* 
{NQSTR}["] {
    if ((sp - string) >= strl) {
       sp -= (int)string;
       string = (char*)realloc(string, strl += 1024);
       sp += (int)string;
    }
    *sp++ = '\\';
    *sp++ = '\"';
}
*/