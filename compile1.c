/****************************************************************/
/*                                                              */
/*      Program 13-1:  Pascal Compiler I                        */
/*                                                              */
/*      Compile assignement statements in procedures            */
/*      and functions.                                          */
/*                                                              */
/*      FILE:       compile1.c                                  */
/*                                                              */
/*      REQUIRES:   Modules parser, symbol table, scanner,      */
/*                          code, error                 	      */
/*								                                              */
/*      USAGE:      compile1 sourcefile objectfile              */
/*                                                              */
/*          sourcefile      [input] source file containing the  */
/*                                  the statements to compile   */
/*                                                              */
/*          objectfile      [output] object file to contain the */
/*                                   generated assembly code    */
/*                                                              */
/*      Copyright (c) 1991 by Ronald Mak                        */
/*      For instructional purposes only.  No warranties.        */
/*                                                              */
/****************************************************************/

#include <stdio.h>
#include "scanner.h"
#include "parser.h"

/*--------------------------------------------------------------*/
/*  Globals                                                     */
/*--------------------------------------------------------------*/

FILE *code_file;    /* ASCII file for the emitted assembly code */

/*--------------------------------------------------------------*/
/*  main                Initialize the scanner and call         */
/*                      routine program.                        */
/*--------------------------------------------------------------*/

int main(int argc, char *argv[])
{
  /*
  --  Open the code file.  If no code file name was given,
  --  use the standard output file.
  */
  code_file = (argc == 3) ? fopen(argv[2], "w") : stdout;

  /*
  --  Initialize the scanner.
  */
  extern bool print_flag;
  print_flag = false;
  init_scanner(argv[1]);

  /*
  --  Process a program.
  */
  get_token();
  program();
}

