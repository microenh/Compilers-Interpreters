/****************************************************************/
/*                                                              */
/*      Pretty-print statements.                                */
/*                                                              */
/*      FILE:       ppstmt.c                                    */
/*                                                              */
/*      Copyright (c) 1991 by Ronald Mak                        */
/*      For instructional purposes only.  No warranties.        */
/*                                                              */
/****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "error.h"
#include "symtab.h"
#include "scanner.h"
#include "pprint.h"

#define MAX_CODE_BUFFER_SIZE 4096

/*--------------------------------------------------------------*/
/*  Externals                                                   */
/*--------------------------------------------------------------*/

extern TOKEN_CODE token;
extern TOKEN_CODE ctoken;

extern int  left_margin;
extern char pprint_buffer[];
extern int  error_count;

extern char *code_buffer;
extern char *code_bufferp;
extern char *code_segmentp;
extern char *code_segment_limit;

/*--------------------------------------------------------------*/
/*  Globals                                                     */
/*--------------------------------------------------------------*/

char *symbol_strings[] = {
  "<no token>", "<IDENTIFIER>", "<NUMBER>", "<STRING>",
  "^", "*", "(", ")", "-", "+", "=", "[", "]", ":", ";",
  "<", ">", ",", ".", "/", ":=", "<=", ">=", "<>", "..",
  "<END OF FILE>", "<ERROR>",
  "AND", "ARRAY", "BEGIN", "CASE", "CONST", "DIV", "DO", "DOWNTO",
  "ELSE", "END", "FILE", "FOR", "FUNCTION", "GOTO", "IF", "IN",
  "LABEL", "MOD", "NIL", "NOT", "OF", "OR", "PACKED", "PROCEDURE",
  "PROGRAM", "RECORD", "REPEAT", "SET", "THEN", "TO", "TYPE",
  "UNTIL", "VAR", "WHILE", "WITH",
};

		/********************************/
		/*                              */
		/*      Code segment routines   */
		/*                              */
		/********************************/

/*--------------------------------------------------------------*/
/*  Forwards                                                    */
/*--------------------------------------------------------------*/ 


char *create_code_segment(void);
SYMTAB_NODE_PTR get_symtab_cptr(void);
void analyze_block(char *code_segment);
void print_statement(void);
void print_assign_or_call_statement(void);
void print_compound_statement(void);
void print_case_statement(void);
void print_for_statement(void);
void print_if_statement(void);
void print_repeat_statement(void);
void print_while_statement(void);
void print_expression(void);
void print_identifier(void);
void print_number(void);
void print_string(void);

/*--------------------------------------------------------------*/
/*  crunch_token        Append the token code to the code       */
/*                      buffer.  Called by the scanner routine  */
/*                      get_token only while parsing a block.   */
/*--------------------------------------------------------------*/

void crunch_token(void)
{
  char token_code = token;    /* byte-sized token code */

  if (code_bufferp >= code_buffer + MAX_CODE_BUFFER_SIZE) {
  	error(CODE_SEGMENT_OVERFLOW);
	  exit(-CODE_SEGMENT_OVERFLOW);
  } else
    *code_bufferp++ = token_code;
}

/*--------------------------------------------------------------*/
/*  crunch_symtab_node_ptr      Append a symbol table node      */
/*                              pointer to the code buffer.     */
/*--------------------------------------------------------------*/

void crunch_symtab_node_ptr(SYMTAB_NODE_PTR np)  /* pointer to append */
{
  SYMTAB_NODE_PTR *npp = (SYMTAB_NODE_PTR *) code_bufferp;

  if (code_bufferp >= code_buffer + MAX_CODE_BUFFER_SIZE  - sizeof(SYMTAB_NODE_PTR)) {
	  error(CODE_SEGMENT_OVERFLOW);
	  exit(-CODE_SEGMENT_OVERFLOW);
  } else {
	  *npp = np;
	  code_bufferp += sizeof(SYMTAB_NODE_PTR);
  }
}

/*--------------------------------------------------------------*/
/*  create_code_segment     Create a code segment and copy in   */
/*                          the contents of the code buffer.    */
/*                          Reset the code buffer pointer.      */
/*                          Return a pointer to the segment.    */
/*--------------------------------------------------------------*/

char *create_code_segment(void)
{
  char *code_segment = alloc_bytes(code_bufferp - code_buffer);

  code_segment_limit = code_segment + (code_bufferp - code_buffer);
  code_bufferp       = code_buffer;
  code_segmentp      = code_segment;

  /*
  --  Copy in the contents of the code buffer.
  */
  while (code_segmentp != code_segment_limit)
  	*code_segmentp++ = *code_bufferp++;

  code_bufferp = code_buffer;         /* reset code buffer ptr */
  return(code_segment);
}

/*--------------------------------------------------------------*/
/*  get_ctoken          Extract the next token code from the    */
/*                      current code segment.                   */
/*--------------------------------------------------------------*/

#define get_ctoken()    ctoken = *code_segmentp++

/*--------------------------------------------------------------*/
/*  get_symtab_cptr     Extract a symbol table node pointer     */
/*                      from the current code segment and       */
/*                      return it.                              */
/*--------------------------------------------------------------*/

SYMTAB_NODE_PTR get_symtab_cptr(void)
{
  SYMTAB_NODE_PTR np;
  SYMTAB_NODE_PTR *npp = (SYMTAB_NODE_PTR *) code_segmentp;

  np = *npp;
  code_segmentp += sizeof(SYMTAB_NODE_PTR);
  return(np);
}

		/****************************************/
		/*                                      */
		/*      Pretty-printing routines        */
		/*                                      */
		/****************************************/

/*--------------------------------------------------------------*/
/*  analyze_block       Pretty-print the code segment of        */
/*                      a block.                                */
/*--------------------------------------------------------------*/

void analyze_block(char *code_segment)
{
  if (error_count > 0)
    return;

  code_segmentp = code_segment;
  emit(" ");
  flush();

  get_ctoken();
  print_statement();      /* should be a compound statement */
  retreat_left_margin();

  /*
  --  Output any trailing semicolons or period.
  */
  while (code_segmentp <= code_segment_limit) {
	  indent();
	  emit(symbol_strings[token]);
	  flush();
	  get_ctoken();
  }
}

/*--------------------------------------------------------------*/
/*  print_statement     Call the appropriate statement printing */
/*                      routine.                                */
/*--------------------------------------------------------------*/

void print_statement(void)
{
  indent();

  switch (ctoken) {
    case IDENTIFIER:  print_assign_or_call_statement();  break;
    case BEGIN:       print_compound_statement();        break;
    case CASE:        print_case_statement();            break;
    case FOR:         print_for_statement();             break;
    case IF:          print_if_statement();              break;
    case REPEAT:      print_repeat_statement();          break;
    case WHILE:       print_while_statement();           break;
  }

  while (ctoken == SEMICOLON) {
  	emit(";");
	  get_ctoken();
  }

  flush();
}

/*--------------------------------------------------------------*/
/*  print_assign_or_call_statement      Pretty-print an assign- */
/*                                      ment or procedure call  */
/*                                      statement:              */
/*                                                              */
/*                                      <variable> := <expr>    */
/*                                      <id>(<parm-list)        */
/*--------------------------------------------------------------*/

void print_assign_or_call_statement(void)
{
  print_identifier();

  if (ctoken == COLONEQUAL) {
  	emit(" := ");
	  get_ctoken();
	  print_expression();
  }
}

/*--------------------------------------------------------------*/
/*  print_compound_statement    Pretty-print a compound         */
/*                              statement:                      */
/*                                                              */
/*                                  BEGIN                       */
/*                                      <stmt-list>             */
/*                                  END                         */
/*--------------------------------------------------------------*/

void print_compound_statement(void)
{
  emit("BEGIN");
  flush();
  advance_left_margin();

  get_ctoken();
  while (ctoken != END)
    print_statement();

  retreat_left_margin();
  indent();
  emit("END");
  get_ctoken();
}

/*--------------------------------------------------------------*/
/*  print_case_statement        Pretty-print a CASE statement:  */
/*                                                              */
/*                                  CASE <expr> OF              */
/*                                      <const-list> : <stmt>   */
/*                                      ...                     */
/*                                  END                         */
/*--------------------------------------------------------------*/

void print_case_statement(void)
{
  emit("CASE ");

  get_ctoken();
  print_expression();
  emit(" OF ");
  flush();
  advance_left_margin();

  get_ctoken();

  /*
  --  Loop to print CASE branches.
  */
  do {
  	indent();

    /*
    --  Loop to print each constant
    --  in the constant list.
    */
    do {
	    print_expression();
	    if (ctoken == COMMA) {
    		emit(", ");
		    get_ctoken();
	    }
	  } while (ctoken != COLON);

  	emit(":");
	  flush();
	  advance_left_margin();

  	get_ctoken();
	  print_statement();
  	retreat_left_margin();
  } while (ctoken != END);

  retreat_left_margin();
  indent();
  emit("END");
  get_ctoken();
}

/*--------------------------------------------------------------*/
/*  print_for_statement     Pretty print a FOR statement:       */
/*                                                              */
/*                              FOR <id> := <expr> TO|DOWNTO    */
/*                                  <expr> DO <stmt>            */
/*--------------------------------------------------------------*/

void print_for_statement(void)
{
  emit("FOR ");

  get_ctoken();
  print_identifier();
  emit(" := ");

  get_ctoken();
  print_expression();
  emit(ctoken == TO ? " TO " : " DOWNTO ");

  get_ctoken();
  print_expression();
  emit(" DO ");
  flush();

  advance_left_margin();
  get_ctoken();
  print_statement();
  retreat_left_margin();
}

/*--------------------------------------------------------------*/
/*  print_if_statement          Pretty-print an IF statement:   */
/*                                                              */
/*                                  IF <expr> THEN              */
/*                                      <stmt>                  */
/*                                                              */
/*                                  IF <expr> THEN              */
/*                                      <stmt>                  */
/*                                  ELSE                        */
/*                                      <stmt>                  */
/*--------------------------------------------------------------*/

void print_if_statement(void)
{
  emit("IF ");

  get_ctoken();
  print_expression();
  emit(" THEN");
  flush();

  advance_left_margin();
  get_ctoken();
  print_statement();
  retreat_left_margin();

  if (ctoken == ELSE) {
  	indent();
	  emit("ELSE");
	  flush();

	  advance_left_margin();
	  get_ctoken();
	  print_statement();
	  retreat_left_margin();
  }
}

/*--------------------------------------------------------------*/
/*  print_repeat_statement      Pretty-print a REPEAT           */
/*                              statement:                      */
/*                                                              */
/*                                  REPEAT                      */
/*                                      <stmt-list>             */
/*                                  UNTIL <expr>                */
/*--------------------------------------------------------------*/

void print_repeat_statement(void)
{
  emit("REPEAT");
  flush();
  advance_left_margin();

  get_ctoken();
  while (ctoken != UNTIL)
    print_statement();

  retreat_left_margin();
  indent();
  emit("UNTIL ");

  get_ctoken();
  print_expression();
}

/*--------------------------------------------------------------*/
/*  print_while_statement       Pretty-print a WHILE statement: */
/*                                                              */
/*                                  WHILE <expr> DO             */
/*                                      <stmt>                  */
/*--------------------------------------------------------------*/

void print_while_statement(void)
{
  emit("WHILE ");

  get_ctoken();
  print_expression();

  emit(" DO");
  flush();
  advance_left_margin();

  get_ctoken();
  print_statement();
  retreat_left_margin();
}

/*--------------------------------------------------------------*/
/*  print_expression            Pretty-print an expression.     */
/*--------------------------------------------------------------*/

void print_expression(void)
{
  bool done = false;       /* TRUE at end of expression */

  do {
  	switch (ctoken) {
	    case IDENTIFIER:    print_identifier();             break;
	    case NUMBER:        print_number();                 break;
	    case STRING:        print_string();                 break;

	    case PLUS:          emit("+");      get_ctoken();   break;
	    case MINUS:         emit("-");      get_ctoken();   break;
	    case STAR:          emit("*");      get_ctoken();   break;
	    case SLASH:         emit("/");      get_ctoken();   break;
	    case DIV:           emit(" DIV ");  get_ctoken();   break;
	    case MOD:           emit(" MOD ");  get_ctoken();   break;
	    case AND:           emit(" AND ");  get_ctoken();   break;
	    case OR:            emit(" OR ");   get_ctoken();   break;
	    case EQUAL:         emit(" = ");    get_ctoken();   break;
	    case NE:            emit(" <> ");   get_ctoken();   break;
	    case LT:            emit(" < ");    get_ctoken();   break;
	    case LE:            emit(" <= ");   get_ctoken();   break;
	    case GT:            emit(" > ");    get_ctoken();   break;
	    case GE:            emit(" >= ");   get_ctoken();   break;
	    case NOT:           emit("NOT ");   get_ctoken();   break;

	    case LPAREN:
	    	emit("(");
		    get_ctoken();
		    print_expression();
		    emit(")");
		    get_ctoken();
		    break;

	    default:
		    done = true;
		    break;
	  }
  } while (!done);
}

/*--------------------------------------------------------------*/
/*  print_identifier        Pretty-print an identifier, which   */
/*                          can be a variable or a procedure    */
/*                          or function call.                   */
/*--------------------------------------------------------------*/

void print_identifier(void)
{
  SYMTAB_NODE_PTR idp = get_symtab_cptr();

  emit(idp->name);
  get_ctoken();

  /*
  --  Loop to print any following modifiers.
  */
  while ((ctoken == LBRACKET) || (ctoken == LPAREN) || (ctoken == PERIOD)) {
    /*
    --  Subscripts or actual parameters.
    */
    if ((ctoken == LBRACKET) || (ctoken == LPAREN)) {
      emit(ctoken == LBRACKET ? "[" : "(");
      get_ctoken();
      while ((ctoken != RBRACKET) && (ctoken != RPAREN)) {
        print_expression();
        while (ctoken == COLON) {
          emit(":");
          get_ctoken();
          print_expression();
        }
        if (ctoken == COMMA) {
          emit(", ");
          get_ctoken();
        }
	    }
	    emit(ctoken == RBRACKET ? "]" : ")");
	    get_ctoken();
	  }

    /*
    --  Record fields.
    */
    else /* ctoken == DOT */ {
      emit(".");
      get_ctoken();
      print_identifier();
    }
  }
}

/*--------------------------------------------------------------*/
/*  print_number                Pretty-print a number.          */
/*--------------------------------------------------------------*/

void print_number(void)
{
  SYMTAB_NODE_PTR idp = get_symtab_cptr();

  emit(idp->name);
  get_ctoken();
}

/*--------------------------------------------------------------*/
/*  print_string                Pretty-print a string.          */
/*--------------------------------------------------------------*/

void print_string(void)
{
  SYMTAB_NODE_PTR idp = get_symtab_cptr();

  emit(idp->name);
  get_ctoken();
}


