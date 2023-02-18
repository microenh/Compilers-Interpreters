/****************************************************************/
/*                                                              */
/*      Program 2-3:  Pascal Source Compactor                   */
/*                                                              */
/*      Compact a Pascal source file by removing                */
/*      all comments and unnecessary blanks.                    */
/*                                                              */
/*      FILE:       compact.c                                   */
/*                                                              */
/*      REQUIRES:   Modules error, scanner                      */
/*                                                              */
/*      USAGE:      compact sourcefile                          */
/*                                                              */
/*          sourcefile      name of source file to compact      */
/*                                                              */
/*      Copyright (c) 1991 by Ronald Mak                        */
/*      For instructional purposes only.  No warranties.        */
/*                                                              */
/****************************************************************/

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "common.h"
#include "scanner.h"

#define MAX_OUTPUT_RECORD_LENGTH	80

/*--------------------------------------------------------------*/
/*  Token classes						*/
/*--------------------------------------------------------------*/

typedef enum {
    DELIMITER, NONDELIMITER
} TOKEN_CLASS;

/*--------------------------------------------------------------*/
/*  Externals                                                   */
/*--------------------------------------------------------------*/

extern TOKEN_CODE token;
extern char token_string[];
extern bool print_flag;

/*--------------------------------------------------------------*/
/*  Globals                                                     */
/*--------------------------------------------------------------*/

int  record_length;             /* length of output record */
char *recp;                     /* pointer into output record */

char output_record[MAX_OUTPUT_RECORD_LENGTH];

TOKEN_CLASS token_class(void);
void append_blank(void);
void append_token(void);
void flush_output_record(void);

/*--------------------------------------------------------------*/
/*  main                Loop to process tokens.                 */
/*--------------------------------------------------------------*/

int main(int argc, char *argv[])
{
  TOKEN_CLASS cur_class;		/* current token class */
  TOKEN_CLASS prev_class;	/* previous token class */
  
  /*
  --  Initialize the scanner.
  */
  print_flag = false;
  init_scanner(argv[1]);

  /*
  --  Initialize the compactor.
  */
  prev_class = DELIMITER;
  recp  = output_record;
  *recp = '\0';
  record_length = 0;

  /*
  --  Repeatedly process tokens until a period
  --  or the end of file.
  */
  do {
	  get_token();
	  if (token == END_OF_FILE) break;
	  cur_class = token_class();

    /*
    --  Append a blank only if two adjacent nondelimiters.
    --  Then append the token string.
    */
  	if ((prev_class == NONDELIMITER) && (cur_class == NONDELIMITER))
	    append_blank();
	  append_token();

	  prev_class = cur_class;
  } while (token != PERIOD);

  /*
  --  Flush the last output record if it is partially filled.
  */
  if (record_length > 0) flush_output_record();

  quit_scanner();
}

/*--------------------------------------------------------------*/
/*  token_class		Return the class of the current token.	*/
/*--------------------------------------------------------------*/

TOKEN_CLASS token_class(void)
{
    /*
    --  Nondelimiters:	identifiers, numbers, and reserved words
    --  Delimiters:	strings and special symbols
    */
    switch (token) {

	case IDENTIFIER:
	case NUMBER:
	    return(NONDELIMITER);

	default:
	    return(token < AND ? DELIMITER : NONDELIMITER);
    }
}

/*--------------------------------------------------------------*/
/*  append_blank	Append a blank to the output record,	*/
/*			or flush the record if it is full.	*/
/*--------------------------------------------------------------*/

void append_blank(void)
{
    if (++record_length == MAX_OUTPUT_RECORD_LENGTH - 1)
	flush_output_record();
    else strcat(output_record, " ");
}

/*--------------------------------------------------------------*/
/*  append_token	Append the token string to the output	*/
/*			record if it fits.  If not, flush the	*/
/*			current record and append the string	*/
/*			to append to the new record.		*/
/*--------------------------------------------------------------*/

void append_token(void)
{
    int	token_length;		/* length of token string */
    
    token_length = strlen(token_string);
    if (record_length + token_length
				>= MAX_OUTPUT_RECORD_LENGTH - 1)
	flush_output_record();

    strcat(output_record, token_string);
    record_length += token_length;
}

/*--------------------------------------------------------------*/
/*  flush_output_record		Flush the current output	*/
/*				record.				*/
/*--------------------------------------------------------------*/

void flush_output_record(void)
{
     printf("%s\n", output_record);
     recp  = output_record;
     *recp = '\0';
     record_length = 0;
}
