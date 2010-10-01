
%{
#include <model.hpp>
#include <iostream>

extern int yylex();
int yyerror(const char *);

extern Model model;

%}

%locations
%defines
%error-verbose

%union {
	int i;
	char * identifier;
}

%token MODULE MESSAGE
%token <i> INT8 INT16 INT32 INT64
%token <i> UINT8 UINT16 UINT32 UINT64
%token <i> STRING
%token <i> FLOAT DOUBLE
%token <i> NUMBER
%token <identifier> IDENTIFIER

%type <i> type

%start translation_unit

%%

translation_unit
	: module_list
	;

module_list
	: module_list module
	| module
	;

module
	: MODULE IDENTIFIER '{' module_body '}'
		{
			model.add_module($2);
		}
	| /* epsilon */
	;

module_body
	: message_list
	;

message_list
	: message_list message
	| message
	;

message
	: MESSAGE IDENTIFIER '{' attribute_list '}'
		{
			model.add_message($2);
		}
	| /* epsilon */
	;

attribute_list
	: attribute_list attribute
	| attribute
	;

attribute
	: type IDENTIFIER
		{
			model.add_attribute($1, $2);
		}
	| type IDENTIFIER '[' NUMBER ']'
		{
			model.add_attribute($1, $2, 0, $4);
		}
	| STRING '<' NUMBER '>' IDENTIFIER
		{
			model.add_attribute($1, $5, $3);
		}
	| STRING '<' NUMBER '>' IDENTIFIER '[' NUMBER ']'
		{
			model.add_attribute($1, $5, $3, $7);
		}
	| /* epsilon */
	;

type
	: INT8
	| INT16
	| INT32
	| INT64
	| UINT8
	| UINT16
	| UINT32
	| UINT64
	| FLOAT
	| DOUBLE
	;

%%

extern int yylineno;

int yyerror(const char * s)
{
	std::cerr << std::endl << "ERROR:" << yylineno << ": " << s << std::endl;
	exit(-1);
}

