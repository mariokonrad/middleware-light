#!/usr/bin/python3

import ply.lex as lex
import ply.yacc as yacc

keywords = {
	'module'  : 'MODULE',
	'message' : 'MESSAGE',
	'int8'    : 'INT8',
	'int16'   : 'INT16',
	'int32'   : 'INT32',
	'int64'   : 'INT64',
	'uint8'   : 'UINT8',
	'uint16'  : 'UINT16',
	'uint32'  : 'UINT32',
	'uint64'  : 'UINT64',
	'float'   : 'FLOAT',
	'double'  : 'DOUBLE',
	'string'  : 'STRING'
	}

tokens = [
	'IDENTIFIER',
	'NUMBER'
	]

tokens.extend(keywords.values())

literals = [ '{', '}', '[', ']', '(', ')', '<', '>' ]

t_ignore = ' \t'

def t_IDENTIFIER(t):
	r'[a-zA-Z][_a-zA-Z0-9]*'
	t.type = keywords.get(t.value, 'IDENTIFIER')
	print('>> identifier', t)
	return t

def t_NUMBER(t):
	r'[1-9][0-9]*|0[xX][0-9a-fA-F]+'
	t.value = int(t.value)
	t.type = 'NUMBER'
	print('>> number', t)
	return t

def t_newline(t):
	r'\n+'
	t.lexer.lineno += len(t.value)

def t_error(t):
	raise TypeError("Unknown text '%s'" % (t.value))

def p_translation_unit(t):
	"""
	translation_unit : modules
	"""
	print(">> translation_unit", t)

def p_empty(t):
	"""
	empty :
	"""
	pass

def p_modules(t):
	"""
	modules : modules module
	        | module
	"""
	print(">> modules", t)

def p_module(t):
	"""
	module : MODULE IDENTIFIER '{' module_body '}'
	       | empty
	"""
	print(">> module", t)

def p_module_body(t):
	"""
	module_body : messages
	            | empty
	"""
	print(">> module_body", t)

def p_messages(t):
	"""
	messages : messages message
	         | message
	"""
	print(">> messages", t)

def p_message(t):
	"""
	message : MESSAGE IDENTIFIER '<' NUMBER '>' '{' attributes '}'
	"""
	print(">> message", t)

def p_attributes(t):
	"""
	attributes : attributes attribute
	           | attribute
	"""
	print(">> attributes", t)

def p_attribute(t):
	"""
	attribute : attribute_primitive
	          | attribute_string
	          | empty
	"""
	print(">> attribute", t)

def p_attribute_primitive(t):
	"""
	attribute_primitive : type IDENTIFIER
	                    | type IDENTIFIER '[' NUMBER ']'
	"""
	print(">> attribute_primitive", t)

def p_attribute_string(t):
	"""
	attribute_string : STRING '<' NUMBER '>' IDENTIFIER
	                 | STRING '<' NUMBER '>' IDENTIFIER '[' NUMBER ']'
	"""
	print(">> attribute_string", t)

def p_type(t):
	"""
	type : INT8
	     | INT16
	     | INT32
	     | INT64
	     | UINT8
	     | UINT16
	     | UINT32
	     | UINT64
	     | FLOAT
	     | DOUBLE
	"""

def p_error(t):
	print("syntax error:", t)


lex.lex(debug=False)
yacc.yacc(debug=False, write_tables=False)
yacc.parse("""
	module test1
	{
		message A<1>
		{
			int8 id1
			int16 id2
			int32 id3
			int64 id4
		}
		message B<2>
		{
			uint8 id5
			uint16 id6
			uint32 id7
			uint64 id8
		}
		message C<3>
		{
			float v1
			float v2[4]
		}
		message D<4>
		{
			double v1
			double v2[3]
		}
		message E<5>
		{
			string<32> s
			string<32> t[2]
		}
	}
	module test2
	{
		message Test<1>
		{
			int8 id1
		}
		message Foobar<2>
		{
			uint8 id5
		}
	}
	""")

