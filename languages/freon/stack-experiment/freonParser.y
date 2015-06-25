// freonParser.y: bison++ grammar for freon parser
// Copyright (C) 2000 Ryan Daum, portions adapted from coldmud grammar.y by Greg Hudson
// see LICENSE (MD5 f5220f8f599e5e926f37cf32efe3ab68) for terms

// $Id: freonParser.y,v 1.1 2000/12/02 16:54:53 opiate Exp $  
// $Log: freonParser.y,v $
// Revision 1.1  2000/12/02 16:54:53  opiate
// Moved old stack-based system to "stack-experiment".
//
// Revision 1.22  2000/11/15 08:31:16  coldstore
// Replaced local version of openc++ with openc++ 2.5.10,
// debian for the updated version is here:
//  ftp://coldstore.sourceforge.net/pub/coldstore/debs/openc++_2.5.10-1_i386.deb
//
// Revision 1.21  2000/11/13 23:45:27  coldstore
// Stupidly forgot to add freonReduce.cc ... hopefully it's semi-stable atm.
//
// Revision 1.20  2000/11/12 04:01:52  coldstore
// Node tree Reduction - works for simple while-test
//
// Revision 1.19  2000/11/10 05:43:04  coldstore
// A whole slew of mods related to:
// 1) Frame interface cleanup - some more abstract interfaces (getIdent, setIdent)
// 2) new vp element (probably temporary) reduce() for tree-transforming evaluation
//
// Revision 1.18  2000/11/09 05:56:36  opiate
// String::add copied from List::add
// Frame::prepareStack and addStack (nonreversing)
// broken freonNodes::whileNode
//
// Revision 1.17  2000/11/08 22:17:38  opiate
// Oops
//
// Revision 1.16  2000/11/08 22:07:11  opiate
// Added CVS Id and log entries.
// 


%name freonParser

%define STYPE Slot
%define ENUM_TOKEN Token
%define ERROR_BODY { cout << cur_line + 1 << ":" << msg << "\n"; }
%define LEX_BODY { }
%define MEMBERS Slot prog; int cur_line; virtual ~freonParser() { };
%{
/* grammar.y: C-- grammar. */
    
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <Data.hh>
#include <Slot.hh>
    
#include "freonNodes.hh"

%}


/* The following tokens are terminals for the parser. */

%token	INTEGER DBREF
%token	COMMENT STRING SYMBOL NAME IDENT ERROR
%token	DISALLOW_OVERRIDES ARG VAR
%token	IF FOR IN UPTO WHILE SWITCH CASE DEFAULT LOCK
%token	BREAK CONTINUE RETURN_
%token		CATCH ANY HANDLER
%token		FORK
%token		PASS CRITLEFT CRITRIGHT PROPLEFT PROPRIGHT

%left	TO
%right	'?' '|'
%right	OR
%right	AND
%left	IN
%left	EQ NE '>' GE '<' LE
%left	'+' '-'
%left	'*' '/' '%'
%left	'!'
%left	'[' ']' START_DICT START_BUFFER
%left	'.'

/* Declarations to shut up shift/reduce conflicts. */
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE
%nonassoc LOWER_THAN_WITH
%nonassoc WITH

/* The parser does not use the following tokens.  I define them here so
 * that I can use them, along with the above tokens, as statement and
 * expression types for the code generator, and as opcodes for the
 * interpreter. */

%token NOOP EXPR COMPOUND ASSIGN IF_ELSE FOR_RANGE FOR_LIST END RETURN_EXPR
%token CASE_VALUE CASE_RANGE LAST_CASE_VALUE LAST_CASE_RANGE END_CASE RANGE
%token FUNCTION_CALL MESSAGE EXPR_MESSAGE LIST DICT BUFFER FROB INDEX UNARY
%token BINARY CONDITIONAL SPLICE NEG SPLICE_ADD POP START_ARGS ZERO ONE
%token SET_LOCAL SET_OBJ_VAR GET_LOCAL GET_OBJ_VAR CATCH_END HANDLER_END
%token CRITICAL CRITICAL_END PROPAGATE PROPAGATE_END JUMP

%token BITAND, BITOR, BITSHIFT
%token TYPE CLASS TOINT TOSTR TOLITERAL TODBREF TOSYM TOERR VALID
%token STRLEN SUBSTR EXPLODE STRSUB PAD MATCH_BEGIN MATCH_TEMPLATE
%token MATCH_PATTERN MATCH_REGEXP CRYPT UPPERCASE LOWERCASE STRCMP
%token LISTLEN SUBLIST INSERT REPLACE DELETE SETADD SETREMOVE TOSET UNION FACTOR QSORT
%token DICT_KEYS DICT_ADD DICT_DEL DICT_ADD_ELEM DICT_DEL_ELEM DICT_CONTAINS
%token BUFFER_LEN BUFFER_RETRIEVE BUFFER_APPEND BUFFER_REPLACE BUFFER_ADD
%token BUFFER_TRUNCATE BUFFER_TO_STRINGS BUFFER_FROM_STRINGS
%token VERSION RANDOM TIME CTIME MIN MAX ABS GET_NAME
%token THIS DEFINER SENDER CALLER TASK_ID REP TICKS_LEFT
%token ERROR_FUNC TRACEBACK ERROR_STR ERROR_ARG THROW RETHROW
%token ECHO_FUNC ECHO_FILE DISCONNECT FILESTAT READ WRITE LS
%token ADD_PARAMETER PARAMETERS DEL_PARAMETER SET_VAR GET_VAR RMVAR COMPILE
%token METHODS FIND_METHOD FIND_NEXT_METHOD LIST_METHOD DEL_METHOD PARENTS
%token CHILDREN ANCESTORS HAS_ANCESTOR SIZE IDENTS
%token CREATE CHPARENTS DESTROY LOG CONN_ASSIGN BINARY_DUMP BINARY_BACKUP TEXT_DUMP
%token RUN_SCRIPT SHUTDOWN BIND UNBIND CONNECT SET_HEARTBEAT_FREQ DATA SET_NAME
%token DEL_NAME DB_TOP TICK HOSTNAME IP
%token RESUME SUSPEND TASKS CANCEL PAUSE CALLERS DISASSEMBLE DEBUG

%token METHOD ARGDECL ID_LIST STMT_LIST


/* Reserved for future use. */
%token FORK ATOMIC NON_ATOMIC

/* LAST_TOKEN tells opcodes.c how much space to allocate for the opcodes
 * table. */
%token LAST_TOKEN

%%

method	: ovdecl argdecl vars stmts	{ prog = new methodNode( cur_line, $1, $2, $3, $4); }
	;

ovdecl	: /* nothing */			{ $$ = new overrideableNode(cur_line, 1); }
	| DISALLOW_OVERRIDES ';'	{ $$ = new overrideableNode(cur_line, 0); }
	;

argdecl	: /* nothing */			{ $$ = new argDeclNode(cur_line); }
	| ARG '[' IDENT ']' ';'		{ $$ = new argDeclNode(cur_line, $3); }
	| ARG idlist ';'		{ $$ = new argDeclNode( cur_line, $2); }
	| ARG idlist ',' '[' IDENT ']' ';'
					{ $$ = new argDeclNode( cur_line, $2, $5); }
	;

vars	: /* nothing */			{ $$ = new varNode( cur_line ); }
	| VAR idlist ';'		{ $$ = new varNode( cur_line, $2); }
	;

idlist	: IDENT				{ $$ = new idListNode( cur_line, $1); }
	| idlist ',' IDENT		{ $$ = new idListNode( cur_line, $3, $1); }
	;

errors	: ANY				{ $$ = noopNode( cur_line ); }
	| errlist			{ $$ = $1; }
	;

errlist	: ERROR				{ $$ = new idListNode( cur_line, $1); }
	| errlist ',' ERROR		{ $$ = new idListNode( cur_line, $3, $1); }
	;

compound: '{' stmts '}'		{ $$ = $2; }
	;

stmts	: stmt				{ $$ = new stmtNode( cur_line, $1 ); }
	| stmts stmt			{ $$ = new stmtNode( cur_line, $1, $2 ); }
	; 


stmt	: COMMENT			{ $$ = new commentNode(cur_line, $1); }
	| ';'				{ $$ = new noopNode(cur_line); }
        | expr ';'			{ $$ = new exprNode(cur_line, $1) };
        | compound			{ $$ = new compoundNode(cur_line, $1 ) };
	| if %prec LOWER_THAN_ELSE	{ $$ = $1; }
	| if ELSE stmt			{ $$ = new ifElseNode(cur_line, $1, $3); }
	| for '[' expr UPTO expr ']' stmt
					{ $$ = new forRangeNode(cur_line, $1, $3, $5, $7); }
	| for '(' expr ')' stmt		{ $$ = new forListNode(cur_line, $1, $3, $5); }
	| WHILE '(' expr ')' stmt	{ $$ = new whileNode(cur_line, new frameNode(cur_line, $5), $3) ; }
	| SWITCH '(' expr ')' caselist	{ $$ = new switchNode(cur_line, $3, $5); }
	| BREAK ';'			{ $$ = new breakNode(cur_line); }
	| CONTINUE ';'			{ $$ = new continueNode(cur_line); }
	| RETURN_ ';'			{ $$ = new returnNode( cur_line ); }
	| RETURN_ expr ';'		{ $$ = new returnNode( cur_line, $2 ); }
	| CATCH errors stmt %prec LOWER_THAN_WITH
					{ $$ = new catchNode(cur_line, $2, $3); }
	| CATCH errors stmt WITH HANDLER stmt
					{ $$ = new catchNode(cur_line, $2, $3, $6); }
	| error ';'			{ yyerrok; $$ = new noopNode(cur_line); }
	;

if	: IF '(' expr ')' stmt		{ $$ = new ifNode(cur_line, new frameNode(cur_line, $5), $3); }
	;

for	: FOR IDENT IN			{ $$ = $2; }
	;

caselist: '{' '}'			{ $$ = new noopNode(cur_line); }
	| '{' cases '}'			{ $$ = $2; }
	;

cases	: case_ent			{ $$ = new caseNode(cur_line, $1); }
	| cases case_ent		{ $$ = new caseNode(cur_line, $2, $1); }
	;

case_ent: CASE cvals ':' stmts	{ $$ = new caseNode(cur_line, $2, $4); }
	| DEFAULT ':' stmts		{ $$ = new caseNode(cur_line, (Data*)NULL, $3); }
	;

expr	: INTEGER			{ $$ = $1; }
	| STRING			{ $$ = $1; }
	| DBREF				{ $$ = new dbrefNode(cur_line, $1); }
	| SYMBOL			{ $$ = new symbolNode(cur_line, $1); }
	| ERROR				{ $$ = new errorNode(cur_line, $1); }
	| NAME				{ $$ = new nameNode(cur_line, $1); }
	| IDENT				{ $$ = new identNode(cur_line, $1); }
	| IDENT '(' args ')'		{ $$ = new functionCallNode( cur_line, $1, $3); }
	| IDENT '=' expr 		{ $$ = new assignNode(cur_line, $1, $3); }
	| PASS '(' args ')'		{ $$ = new passNode(cur_line, $3); }
	| expr '.' IDENT '(' args ')'	{ $$ = new messageNode(cur_line, $1, $3, $5); }
	| '.' IDENT '(' args ')'	{ $$ = new messageNode(cur_line, (Data*)NULL, $2, $4); }
	| expr '.' '(' expr ')' '(' args ')'
					{ $$ = new exprMessageNode(cur_line, $1, $4, $7); }
	| '.' '(' expr ')' '(' args ')'
				{ $$ = new exprMessageNode( cur_line, $3, $6); }
	| '[' args ']'			{ $$ = new listNode(cur_line, $2); }
	| START_DICT dictargs ']'		{ $$ = new dictNode(cur_line, $2); }
	| START_BUFFER args ']'		{ $$ = new bufferNode(cur_line, $2); }
	| '<' expr ',' expr '>'		{ $$ = new frobNode(cur_line, $2, $4); }
	| expr '[' expr ']'		{ $$ = new indexNode(cur_line, $1, $3); }
	| '!' expr			{ $$ = new unaryNode(cur_line, '!', $2); }
	| '-' expr %prec '!'		{ $$ = new unaryNode(cur_line, NEG, $2); }
	| '+' expr %prec '!'		{ $$ = $2; }
	| expr '*' expr			{ $$ = new mulNode(cur_line, $1, $3); }
	| expr '/' expr			{ $$ = new divNode(cur_line, $1, $3); }
	| expr '%' expr			{ $$ = new modNode(cur_line, $1, $3); }
	| expr '+' expr			{ $$ = new addNode(cur_line, $1, $3); }
	| expr '-' expr			{ $$ = new subNode(cur_line, $1, $3); }
	| expr EQ expr			{ $$ = new eqNode(cur_line, $1, $3); }
	| expr NE expr			{ $$ = new neNode(cur_line, $1, $3); }
	| expr '>' expr			{ $$ = new gtNode(cur_line, $1, $3); }
	| expr GE expr			{ $$ = new geNode(cur_line, $1, $3); }
	| expr '<' expr			{ $$ = new ltNode(cur_line, $1, $3); }
	| expr LE expr			{ $$ = new leNode(cur_line, $1, $3); }
	| expr IN expr			{ $$ = new inNode(cur_line, $1, $3); }
	| expr AND expr			{ $$ = new andNode(cur_line, $1, $3); }
	| expr OR expr			{ $$ = new orNode(cur_line, $1, $3); }
	| expr '?' expr '|' expr	{ $$ = new conditionalNode(cur_line, $1, $3, $5); }
	| '(' expr ')'			{ $$ = $2; }
        | CRITLEFT expr CRITRIGHT       { $$ = new criticalNode(cur_line, $2); }
	| PROPLEFT expr PROPRIGHT	{ $$ = new propagateNode(cur_line, $2); }
	;

sexpr	: expr				{ $$ = $1; }
	| '@' expr			{ $$ = new spliceNode(cur_line, $2); }
	;

dictargs:          			{ $$ = new noopNode( cur_line ); }
        | dictlist                      { $$ = $1; }

dictelem: sexpr ':' rexpr               { $$ = new dictElementNode(cur_line, $1, $3); }

dictlist: dictelem                      { $$ = $1; }
        | dictelem ',' dictlist         { $$ = new dictElementNode(cur_line, $1, $3); }

args	: /* nothing */			{ $$ = new noopNode( cur_line ); }
	| arglist			{ $$ = $1; }
	;

arglist	: sexpr				{ $$ = new listElementNode(cur_line, $1); }
	| arglist ',' sexpr		{ $$ = new listElementNode(cur_line, $3, $1); }
	;

rexpr	: expr				{ $$ = $1; }
	| expr UPTO expr		{ $$ = new rangeNode(cur_line, $1, $3); }
	;

cvals	: rexpr				{ $$ = new listElementNode(cur_line, $1); }
	| cvals ',' rexpr		{ $$ = new listElementNode(cur_line, $1, $3); }
	;

%%
