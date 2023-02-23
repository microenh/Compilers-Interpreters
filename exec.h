/****************************************************************/
/*                                                              */
/*      E X E C U T O R   (Header)                              */
/*                                                              */
/*      FILE:       exec.h                                      */
/*                                                              */
/*      MODULE:     executor                                    */
/*                                                              */
/*      Copyright (c) 1991 by Ronald Mak                        */
/*      For instructional purposes only.  No warranties.        */
/*                                                              */
/****************************************************************/

#ifndef exec_h
#define exec_h

#include "common.h"
#include "parser.h"

#define STATEMENT_MARKER 0x70
#define ADDRESS_MARKER   0x71

extern int  exec_line_number;
extern long exec_stmt_count;

/*--------------------------------------------------------------*/
/*  Runtime stack                                               */
/*--------------------------------------------------------------*/

typedef union {
  int     integer;
  float   real;
  char    byte;
  ADDRESS address;
} STACK_ITEM, *STACK_ITEM_PTR;

typedef struct {
  STACK_ITEM function_value;
  STACK_ITEM static_link;
  STACK_ITEM dynamic_link;
  STACK_ITEM return_address;
} *STACK_FRAME_HEADER_PTR;

/*--------------------------------------------------------------*/
/*  Functions                                                   */
/*--------------------------------------------------------------*/

SYMTAB_NODE_PTR get_symtab_cptr();
int             get_statement_cmarker();
TYPE_STRUCT_PTR exec_routine_call();
TYPE_STRUCT_PTR exec_expression(), exec_variable();
char            *crunch_address_marker();
char            *fixup_address_marker();
int             get_statement_cmarker();
char            *get_address_cmarker();
int             get_cinteger();
char            *get_caddress();

		/************************/
		/*                      */
		/*      Macros          */
		/*                      */
		/************************/

/*--------------------------------------------------------------*/
/*  get_ctoken          Extract the next token code from the    */
/*                      current code segment.                   */
/*--------------------------------------------------------------*/

#define get_ctoken()    ctoken = *code_segmentp++

/*--------------------------------------------------------------*/
/*  pop                 Pop the runtime stack.                  */
/*--------------------------------------------------------------*/

#define pop()           --tos

/*--------------------------------------------------------------*/
/*  Tracing routine calls       Unless the following statements */
/*                              are preceded by                 */
/*                                                              */
/*                                      #define trace           */
/*                                                              */
/*                              calls to the tracing routines   */
/*                              are not compiled.               */
/*--------------------------------------------------------------*/

#define trace

#ifndef trace
#define trace_routine_entry(idp)
#define trace_routine_exit(idp)
#define trace_statement_execution()
#define trace_data_store(idp, idp_tp, targetp, target_tp)
#define trace_data_fetch(idp, tp, datap)
#else
void trace_routine_entry(SYMTAB_NODE_PTR idp);
void trace_routine_exit(SYMTAB_NODE_PTR idp);
void trace_statement_execution(void);
void trace_data_store(
    SYMTAB_NODE_PTR idp,     
    TYPE_STRUCT_PTR idp_tp,         
    STACK_ITEM_PTR  targetp,   
    TYPE_STRUCT_PTR target_tp
);
void trace_data_fetch(
  SYMTAB_NODE_PTR idp,            
  TYPE_STRUCT_PTR tp,            
  STACK_ITEM_PTR  datap  
);
void print_data_value(
    STACK_ITEM_PTR  datap,  
    TYPE_STRUCT_PTR tp,  
    char            *str  
);
#endif

int get_statement_cmarker(void);
void push_integer(int item_value);
void push_real(float item_value);
void push_byte(char item_value);
void push_address(ADDRESS address);
void execute(SYMTAB_NODE_PTR rtn_idp);
void crunch_statement_marker(void);
void push_stack_frame_header(int old_level, int new_level);
void exec_statement(void);


#endif
