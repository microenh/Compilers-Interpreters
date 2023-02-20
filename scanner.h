/****************************************************************/
/*                                                              */
/*      S C A N N E R   (Header)                                */
/*                                                              */
/*      FILE:       scanner.h                                   */
/*                                                              */
/*      MODULE:     scanner                                     */
/*                                                              */
/*      Copyright (c) 1991 by Ronald Mak                        */
/*      For instructional purposes only.  No warranties.        */
/*                                                              */
/****************************************************************/

#ifndef scanner_h
#define scanner_h

#include "common.h"
#include <stdbool.h>

/*--------------------------------------------------------------*/
/*  Token codes							                                    */
/*--------------------------------------------------------------*/

typedef enum {
  NO_TOKEN, IDENTIFIER, NUMBER, STRING,
  UPARROW, STAR, LPAREN, RPAREN, MINUS, PLUS, EQUAL,
  LBRACKET, RBRACKET, COLON, SEMICOLON, LT, GT, COMMA, PERIOD,
  SLASH, COLONEQUAL, LE, GE, NE, DOTDOT, END_OF_FILE, ERROR,
  AND, ARRAY, BEGIN, CASE, CONST, DIV, DO, DOWNTO, ELSE, END,
  FFILE, FOR, FUNCTION, GOTO, IF, IN, LABEL, MOD, NIL, NOT,
  OF, OR, PACKED, PROCEDURE, PROGRAM, RECORD, REPEAT, SET,
  THEN, TO, TYPE, UNTIL, VAR, WHILE, WITH
} TOKEN_CODE;

/*--------------------------------------------------------------*/
/*  Literal structure						                                */
/*--------------------------------------------------------------*/

typedef enum {
  INTEGER_LIT, REAL_LIT, STRING_LIT
} LITERAL_TYPE;

typedef struct {
  LITERAL_TYPE type;
  union {
  	int   integer;
	  float real;
	  char  string[MAX_SOURCE_LINE_LENGTH];
  } value;
} LITERAL;

void init_scanner(char *name);
void print_line(char line[]);
void quit_scanner(void);
void get_token(void);
void close_source_file(void);
void open_source_file(char *name);
bool token_in(TOKEN_CODE token_list[]);
void synchronize(TOKEN_CODE token_list1[],
                 TOKEN_CODE token_list2[],
                 TOKEN_CODE token_list3[]);

#endif
