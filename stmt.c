/****************************************************************/
/*                                                              */
/*      S T A T E M E N T   P A R S E R                         */
/*								                                              */
/*	Parsing routines for statements.			                      */
/*								                                              */
/*      FILE:       stmt.c                                      */
/*								                                              */
/*      MODULE:     parser                                      */
/*                                                              */
/*      Copyright (c) 1991 by Ronald Mak                        */
/*      For instructional purposes only.  No warranties.        */
/*                                                              */
/****************************************************************/

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "common.h"
#include "error.h"
#include "scanner.h"
#include "symtab.h"
#include "parser.h"
#include "expr.h"

/*--------------------------------------------------------------*/
/*  Externals                                                   */
/*--------------------------------------------------------------*/

extern TOKEN_CODE token;
extern LITERAL    literal;
extern TOKEN_CODE statement_start_list[], statement_end_list[];

void assignment_statement(void);
void while_statement(void);
void repeat_statement(void);
void if_statement(void);
void for_statement(void);
void case_statement(void);
void case_branch(void);
void case_label(void);
void compound_statement(void);

/*--------------------------------------------------------------*/
/*  statement		Process a statement by calling the	            */
/*                      appropriate parsing routine based on    */
/*			the statement's first token.		                        */
/*--------------------------------------------------------------*/

void statement(void)
{
  /*
  --	Call the appropriate routine based on the first
  --	token of the statement.
  */
  switch (token) {
    case IDENTIFIER:    assignment_statement(); break;
    case REPEAT:        repeat_statement();     break;
    case WHILE:         while_statement();      break;
    case IF:            if_statement();         break;
    case FOR:           for_statement();        break;
    case CASE:          case_statement();       break;
    case BEGIN:         compound_statement();   break;
    default: break;
  }

  /*
  --  Error synchronization:  Only a semicolon, END, ELSE, or
  --                          UNTIL may follow a statement.
  --                          Check for a missing semicolon.
  */
  synchronize(statement_end_list, statement_start_list, NULL);
  if (token_in(statement_start_list)) error(MISSING_SEMICOLON);
}

/*--------------------------------------------------------------*/
/*  assignment_statement    Process an assignment statement:	  */
/*								                                              */
/*				<id> := <expr>			                                  */
/*--------------------------------------------------------------*/

void assignment_statement(void)
{
  get_token();
  if_token_get_else_error(COLONEQUAL, MISSING_COLONEQUAL);

  expression();
}

/*--------------------------------------------------------------*/
/*  repeat_statement    Process a REPEAT statement:             */
/*                                                              */
/*                          REPEAT <stmt-list> UNTIL <expr>     */
/*--------------------------------------------------------------*/

void repeat_statement(void)
{
  /*
  --  <stmt-list>
  */
  get_token();
  do {
    statement();
    while (token == SEMICOLON) get_token();
  } while (token_in(statement_start_list));

  if_token_get_else_error(UNTIL, MISSING_UNTIL);
  expression();
}

/*--------------------------------------------------------------*/
/*  while_statement     Process a WHILE statement:              */
/*                                                              */
/*                          WHILE <expr> DO <stmt>              */
/*--------------------------------------------------------------*/

void while_statement(void)
{
  get_token();
  expression();

  if_token_get_else_error(DO, MISSING_DO);
  statement();
}

/*--------------------------------------------------------------*/
/*  if_statement        Process an IF statement:                */
/*                                                              */
/*                          IF <expr> THEN <stmt>               */
/*                                                              */
/*                      or:                                     */
/*                                                              */
/*                          IF <expr> THEN <stmt> ELSE <stmt>   */
/*--------------------------------------------------------------*/

void if_statement(void)
{
  get_token();
  expression();

  if_token_get_else_error(THEN, MISSING_THEN);
  statement();

  /*
  --  ELSE branch?
  */
  if (token == ELSE) {
  	get_token();
	  statement();
  }
}

/*--------------------------------------------------------------*/
/*  for_statement       Process a FOR statement:                */
/*                                                              */
/*                          FOR <id> := <expr> TO|DOWNTO <expr> */
/*                              DO <stmt>                       */
/*--------------------------------------------------------------*/

void for_statement(void)
{
  get_token();
  if_token_get_else_error(IDENTIFIER, MISSING_IDENTIFIER);

  if_token_get_else_error(COLONEQUAL, MISSING_COLONEQUAL);
  expression();

  if ((token == TO) || (token == DOWNTO)) get_token();
  else error(MISSING_TO_OR_DOWNTO);

  expression();
  if_token_get_else_error(DO, MISSING_DO);

  statement();
}

/*--------------------------------------------------------------*/
/*  case_statement      Process a CASE statement:               */
/*                                                              */
/*                          CASE <expr> OF                      */
/*                              <case-branch> ;                 */
/*                              ...                             */
/*                          END                                 */
/*--------------------------------------------------------------*/

TOKEN_CODE follow_expr_list[]      = {OF, SEMICOLON, 0};

TOKEN_CODE case_label_start_list[] = {IDENTIFIER, NUMBER, PLUS,
				      MINUS, STRING, 0};

void case_statement(void)
{
  bool another_branch;

  get_token();
  expression();

  /*
  --  Error synchronization:  Should be OF
  */
  synchronize(follow_expr_list, case_label_start_list, NULL);
  if_token_get_else_error(OF, MISSING_OF);

  /*
  --  Loop to process CASE branches.
  */
  another_branch = token_in(case_label_start_list);
  while (another_branch) {
	  if (token_in(case_label_start_list)) case_branch();

    if (token == SEMICOLON) {
      get_token();
      another_branch = true;
    }
    else if (token_in(case_label_start_list)) {
      error(MISSING_SEMICOLON);
      another_branch = true;
    }
    else another_branch = false;
  }

  if_token_get_else_error(END, MISSING_END);
}

/*--------------------------------------------------------------*/
/*  case_branch             Process a CASE branch:              */
/*                                                              */
/*                              <case-label-list> : <stmt>      */
/*--------------------------------------------------------------*/

TOKEN_CODE follow_case_label_list[] = {COLON, SEMICOLON, 0};

void case_branch(void)
{
  bool another_label;

  /*
  --  <case-label-list>
  */
  do {
  	case_label();

	  get_token();
    if (token == COMMA) {
      get_token();
      if (token_in(case_label_start_list)) another_label = true;
      else {
  		  error(MISSING_CONSTANT);
	  	  another_label = false;
      }
	  } else another_label = false;
  } while (another_label);

  /*
  --  Error synchronization:  Should be :
  */
  synchronize(follow_case_label_list, statement_start_list, NULL);
  if_token_get_else_error(COLON, MISSING_COLON);

  statement();
}

/*--------------------------------------------------------------*/
/*  case_label              Process a CASE label and return a   */
/*                          pointer to its type structure.      */
/*--------------------------------------------------------------*/

void case_label(void)
{
  TOKEN_CODE sign     = PLUS;         /* unary + or - sign */
  bool       saw_sign = false;        /* TRUE iff unary sign */

  /*
  --  Unary + or - sign.
  */
  if ((token == PLUS) || (token == MINUS)) {
  	sign     = token;
	  saw_sign = true;
	  get_token();
  }

  /*
  --  Number or identifier.
  */
  if ((token == NUMBER) || (token == IDENTIFIER))
    return;

    /*
    --  String constant:  Character type only.
    */
  else if (token == STRING) {
  	if (saw_sign) error(INVALID_CONSTANT);

  	if (strlen(literal.value.string) != 1)
	    error(INVALID_CONSTANT);
  }
}

/*--------------------------------------------------------------*/
/*  compound_statement	    Process a compound statement:	      */
/*								                                              */
/*				BEGIN <stmt-list> END	                              	*/
/*--------------------------------------------------------------*/

void compound_statement(void)
{
  /*
  --  <stmt-list>
  */
  get_token();
  do {
  	statement();
    while (token == SEMICOLON) get_token();
	  if (token == END) break;

    /*
    --  Error synchronization:  Should be at the start of the
    --                          next statement.
    */
  	synchronize(statement_start_list, NULL, NULL);
  } while (token_in(statement_start_list));

  if_token_get_else_error(END, MISSING_END);
}
