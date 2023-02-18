/****************************************************************/
/*                                                              */
/*      Program 3-2:  Pascal Source Cruncher			              */
/*                                                              */
/*      Crunch a Pascal source file.  It can be restored later  */
/*      with the uncruncher utility.                            */
/*                                                              */
/*      FILE:       crunch.c                                    */
/*                                                              */
/*      REQUIRES:   Modules symbol table, scanner, error        */
/*                                                              */
/*      USAGE:      crunch sourcefile crunchfile                */
/*                                                              */
/*          sourcefile      [input] source file to crunch       */
/*                                                              */
/*          crunchfile      [output] crunch file                */
/*                                                              */
/*      Copyright (c) 1991 by Ronald Mak                        */
/*      For instructional purposes only.  No warranties.        */
/*                                                              */
/****************************************************************/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "scanner.h"
#include "symtab.h"

/*--------------------------------------------------------------*/
/*  Externals                                                   */
/*--------------------------------------------------------------*/

extern TOKEN_CODE  token;
extern char        token_string[];
extern char        word_string[];
extern bool        print_flag;

extern SYMTAB_NODE_PTR symtab_root;

/*--------------------------------------------------------------*/
/*  Globals                                                     */
/*--------------------------------------------------------------*/

short st_index = 0;        /* symtab entry index */
FILE  *crunch_file;

void do_pass_1(void);
void do_pass_2(void);
void output_crunched_symtab(SYMTAB_NODE_PTR np);
void output_crunched_token(void);

/*--------------------------------------------------------------*/
/*  Main program	Crunch a source file in two passes	*/
/*			over the file.				*/
/*--------------------------------------------------------------*/

int main(int argc, char *argv[])
{
  /*
  --  Initialize the scanner.
  */
  print_flag = false;
  init_scanner(argv[1]);

  /*
  --  Pass 1.
  */
  do_pass_1();
  close_source_file();

  /*
  --  Open the crunch file and output the crunched
  --  symbol table.
  */
  crunch_file = fopen(argv[2], "wb");
  if (crunch_file == NULL) {
	  fprintf(stderr, "*** ERROR: Failed to open crunch file.\n");
	  exit(-2);
  }
  fwrite(&st_index, sizeof(short), 1, crunch_file);
  output_crunched_symtab(symtab_root);

  /*
  --  Pass 2.
  */
  open_source_file(argv[1]);
  do_pass_2();

  fclose(crunch_file);
  quit_scanner();
}

/*--------------------------------------------------------------*/
/*  do_pass_1           Pass 1: Read the source file to build   */
/*                              the symbol table.               */
/*--------------------------------------------------------------*/

void do_pass_1(void)
{
  SYMTAB_NODE_PTR np;         /* ptr to symtab node */

  /*
  --  Repeatedly process tokens until a period
  --  or the end of file.
  */
  do {
    get_token();
    if (token == END_OF_FILE) break;

    /*
    --  Enter each identifier, number, or string into
    --  the symbol table if it isn't already in there.
    */
    switch (token) {
      case IDENTIFIER:
        if ((np = search_symtab(word_string, symtab_root)) == NULL) {
          np = enter_symtab(word_string, &symtab_root);
          np->info.s_data = st_index++;
        }
        break;

      case NUMBER:
      case STRING:
        if ((np = search_symtab(token_string, symtab_root)) == NULL) {
          np = enter_symtab(token_string, &symtab_root);
          np->info.s_data = st_index++;
        }
        break;

      default:
        break;
    }
  } while (token != PERIOD);
}

/*--------------------------------------------------------------*/
/*  do_pass_2           Pass 2: Reread the source file to       */
/*                              output the crunched program.    */
/*--------------------------------------------------------------*/

void do_pass_2(void)
{
  SYMTAB_NODE_PTR np;         /* ptr to symtab node */

  /*
  --  Repeatedly process tokens until a period
  --  or the end of file.
  */
  do {
  	get_token();
	  if (token == END_OF_FILE) break;

	  output_crunched_token();
  } while (token != PERIOD);
}

/*--------------------------------------------------------------*/
/*  output_crunched_symtab      Output a crunched symbol table  */
/*                              in alphabetical order.          */
/*--------------------------------------------------------------*/

void output_crunched_symtab(
  SYMTAB_NODE_PTR np)         /* ptr to symtab subtree */
{
  char length;                /* byte-sized string length */

  if (np == NULL) return;

  /*
  --  First, crunch the left subtree.
  */
  output_crunched_symtab(np->left);

  /*
  --  Then, crunch the root of the subtree.
  */
  length = strlen(np->name) + 1;
  st_index  = np->info.s_data;
  fwrite(&st_index,  sizeof(short), 1, crunch_file);
  fwrite(&length, 1,             1, crunch_file);
  fwrite(np->name,length,        1, crunch_file);

  /*
  --  Finally, crunch the right subtree.
  */
  output_crunched_symtab(np->right);
}

/*--------------------------------------------------------------*/
/*  output_crunched_token	Output a token record.            		*/
/*--------------------------------------------------------------*/
void output_crunched_token(void)
{
  SYMTAB_NODE_PTR np;      /* ptr to symtab node */
  char token_code = token; /* byte-sized token code */

  /*
  --  Write the token code.
  */
  fwrite(&token_code, 1, 1, crunch_file);
  
  /*
  --  If it's an identifier, number, or string,
  --  look up the symbol table entry and write
  --  the entry st_index.
  */
  switch (token) {
  	case IDENTIFIER:
	    np = search_symtab(word_string, symtab_root);
	    st_index = np->info.s_data;
	    fwrite(&st_index, sizeof(short), 1, crunch_file);
	    break;

	  case NUMBER:
	  case STRING:
	    np = search_symtab(token_string, symtab_root);
	    st_index = np->info.s_data;
	    fwrite(&st_index, sizeof(short), 1, crunch_file);
	    break;

	  default:
	    break;
  }
}
