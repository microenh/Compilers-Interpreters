/****************************************************************/
/*                                                              */
/*      E X E C U T O R   U T I L I T I E S                     */
/*                                                              */
/*      Utility routines for the executor module.               */
/*                                                              */
/*      FILE:       executil.c                                  */
/*                                                              */
/*      MODULE:     executor                                    */
/*                                                              */
/*      Copyright (c) 1991 by Ronald Mak                        */
/*      For instructional purposes only.  No warranties.        */
/*                                                              */
/****************************************************************/

#include <stdio.h>
#include "common.h"
#include "error.h"
#include "symtab.h"
#include "scanner.h"
#include "exec.h"

/*--------------------------------------------------------------*/
/*  Externals                                                   */
/*--------------------------------------------------------------*/

extern TOKEN_CODE token;
extern int        line_number;
extern int        level;

extern TYPE_STRUCT_PTR integer_typep, real_typep,
		       boolean_typep, char_typep;

/*--------------------------------------------------------------*/
/*  Globals                                                     */
/*--------------------------------------------------------------*/

char *code_buffer;                      /* code buffer */
char *code_bufferp;                     /* code buffer ptr */
char *code_segmentp;                    /* code segment ptr */
char *code_segment_limit;               /* end of code segment */
char *statement_startp;                 /* ptr to start of stmt */

TOKEN_CODE     ctoken;                  /* token from code segment */
int            exec_line_number;        /* no. of line executed */
long           exec_stmt_count = 0;     /* count of stmts executed */

STACK_ITEM     *stack;                  /* runtime stack */
STACK_ITEM_PTR tos;                     /* ptr to runtime stack top */
STACK_ITEM_PTR stack_frame_basep;       /* ptr to stack frame base */

/*--------------------------------------------------------------*/
/*  Forwards                                                    */
/*--------------------------------------------------------------*/

void crunch_token(void);
void crunch_symtab_node_ptr(SYMTAB_NODE_PTR np);
void crunch_statement_marker(void);
char *crunch_address_marker(ADDRESS address);
char *fixup_address_marker(ADDRESS address);
void crunch_offset(ADDRESS address);

char *create_code_segment(void);
SYMTAB_NODE_PTR get_symtab_cptr(void);

char *get_address_cmarker(void);
int get_cinteger(void);
char *get_caddress(void);

void routine_entry(SYMTAB_NODE_PTR rtn_idp);
void routine_exit(SYMTAB_NODE_PTR rtn_idp);

void alloc_local(TYPE_STRUCT_PTR tp);
void free_data(SYMTAB_NODE_PTR idp);

		/********************************/
		/*                              */
		/*      Code segment routines   */
		/*                              */
		/********************************/

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

void crunch_symtab_node_ptr(SYMTAB_NODE_PTR np) /* pointer to append */
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
/*  crunch_statement_marker     Append a statement marker to    */
/*                              the code buffer.                */
/*--------------------------------------------------------------*/

void crunch_statement_marker(void)
{
  if (code_bufferp >= code_buffer + MAX_CODE_BUFFER_SIZE - sizeof(int)) {
	  error(CODE_SEGMENT_OVERFLOW);
	  exit(-CODE_SEGMENT_OVERFLOW);
  } else {
	  char save_code = *(--code_bufferp);

	  *code_bufferp++ = STATEMENT_MARKER;
	  *((int *) code_bufferp) = line_number;
	  code_bufferp += sizeof(int);
	  *code_bufferp++ = save_code;
  }
}

/*--------------------------------------------------------------*/
/*  crunch_address_marker       Append a code address to the    */
/*                              code buffer.  Return the        */
/*                              addesss of the address.         */
/*--------------------------------------------------------------*/

char *crunch_address_marker(ADDRESS address) /* address value to append */
{
  char *save_code_bufferp;

  if (code_bufferp >= code_buffer + MAX_CODE_BUFFER_SIZE - sizeof(ADDRESS)) {
	  error(CODE_SEGMENT_OVERFLOW);
	  exit(-CODE_SEGMENT_OVERFLOW);
  } else {
	  char save_code = *(--code_bufferp);

	  *code_bufferp++ = ADDRESS_MARKER;
	  save_code_bufferp = code_bufferp;
	  *((ADDRESS *) code_bufferp) = address;
	  code_bufferp += sizeof(ADDRESS);
	  *code_bufferp++ = save_code;

	  return(save_code_bufferp);
  }
}

/*--------------------------------------------------------------*/
/*  fixup_address_marker        Fix up an address marker with   */
/*                              the offset from the address     */
/*                              marker to the current code      */
/*                              buffer address.  Return the old */
/*                              value of the address marker.    */
/*--------------------------------------------------------------*/

char *fixup_address_marker(ADDRESS address) /* address of address marker to be fixed up */
{
  char *old_address = *((ADDRESS *) address);

  *((int *) address) = code_bufferp - (char *) address;
  return(old_address);
}

/*--------------------------------------------------------------*/
/*  crunch_integer      Append an integer value to the code     */
/*                      buffer.                                 */
/*--------------------------------------------------------------*/

void crunch_integer(int value)  /* value to append */
{
  if (code_bufferp >= code_buffer + MAX_CODE_BUFFER_SIZE - sizeof(int)) {
	  error(CODE_SEGMENT_OVERFLOW);
	  exit(-CODE_SEGMENT_OVERFLOW);
  } else {
	  *((int *) code_bufferp) = value;
	  code_bufferp += sizeof(int);
  }
}

/*--------------------------------------------------------------*/
/*  crunch_offset       Append an integer value to the code     */
/*                      that represents the offset from the     */
/*                      given address to the current code       */
/*                      buffer address.                         */
/*--------------------------------------------------------------*/

void crunch_offset(ADDRESS address) /* address from which to offset */
{
  if (code_bufferp >= code_buffer + MAX_CODE_BUFFER_SIZE  - sizeof(int)) {
    error(CODE_SEGMENT_OVERFLOW);
    exit(-CODE_SEGMENT_OVERFLOW);
  } else {
	  *((int *) code_bufferp) = (char *) address - code_bufferp;
	  code_bufferp += sizeof(int);
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

/*--------------------------------------------------------------*/
/*  get_statement_cmarker   Extract a statement marker from the */
/*                          current code segment and return its */
/*                          statement line number.              */
/*--------------------------------------------------------------*/

int get_statement_cmarker(void)
{
  int line_num;

  if (ctoken == STATEMENT_MARKER) {
  	line_num = *((int *) code_segmentp);
	  code_segmentp += sizeof(int);
  }

  return(line_num);
}

/*--------------------------------------------------------------*/
/*  get_address_cmarker     Extract an address marker from the  */
/*                          current code segment.  Add its      */
/*                          offset value to the code segment    */
/*                          address and return the new address. */
/*--------------------------------------------------------------*/

char *get_address_cmarker(void)
{
  ADDRESS address;    /* address to return */

  if (ctoken == ADDRESS_MARKER) {
  	address = *((int *) code_segmentp) + code_segmentp - 1;
	  code_segmentp += sizeof(ADDRESS);
  }

  return(address);
}

/*--------------------------------------------------------------*/
/*  get_cinteger        Extract an integer value from the       */
/*                      current code segment and return the     */
/*                      value.                                  */
/*--------------------------------------------------------------*/

int get_cinteger(void)
{
  int value;          /* value to extract and return */

  value = *((int *) code_segmentp);
  code_segmentp += sizeof(int);

  return(value);
}

/*--------------------------------------------------------------*/
/*  get_caddress        Extract an offset from the current code */
/*                      segment and add it to the code segment  */
/*                      address.  Return the new address.       */
/*--------------------------------------------------------------*/

char *get_caddress(void)
{
  ADDRESS address;    /* address to return */

  address = *((int *) code_segmentp) + code_segmentp - 1;
  code_segmentp += sizeof(int);

  return(address);
}


		/********************************/
		/*                              */
		/*      Executor utilities      */
		/*                              */
		/********************************/

/*--------------------------------------------------------------*/
/*  push_integer        Push an integer onto the runtime stack. */
/*--------------------------------------------------------------*/

void push_integer(int item_value)
{
  STACK_ITEM_PTR itemp = ++tos;

  if (itemp >= &stack[MAX_STACK_SIZE])
  	runtime_error(RUNTIME_STACK_OVERFLOW);

  itemp->integer = item_value;
}

/*--------------------------------------------------------------*/
/*  push_real           Push a real onto the runtime stack.     */
/*--------------------------------------------------------------*/

void push_real(float item_value)
{
  STACK_ITEM_PTR itemp = ++tos;

  if (itemp >= &stack[MAX_STACK_SIZE])
  	runtime_error(RUNTIME_STACK_OVERFLOW);

  itemp->real = item_value;
}

/*--------------------------------------------------------------*/
/*  push_byte           Push a byte onto the runtime stack.     */
/*--------------------------------------------------------------*/

void push_byte(char item_value)
{
  STACK_ITEM_PTR itemp = ++tos;

  if (itemp >= &stack[MAX_STACK_SIZE])
  	runtime_error(RUNTIME_STACK_OVERFLOW);

  itemp->byte = item_value;
}

/*--------------------------------------------------------------*/
/*  push_address        Push an address onto the runtime stack. */
/*--------------------------------------------------------------*/

void push_address(ADDRESS address)
{
  STACK_ITEM_PTR itemp = ++tos;

  if (itemp >= &stack[MAX_STACK_SIZE])
  	runtime_error(RUNTIME_STACK_OVERFLOW);

  itemp->address = address;
}

/*--------------------------------------------------------------*/
/*  execute             Execute a routine's code segment.       */
/*--------------------------------------------------------------*/

void execute(SYMTAB_NODE_PTR rtn_idp)
{
  routine_entry(rtn_idp);

  get_ctoken();
  exec_statement();

  routine_exit(rtn_idp);
}

/*--------------------------------------------------------------*/
/*  routine_entry       Point to the new routine's code         */
/*                      segment, and allocate its locals.       */
/*--------------------------------------------------------------*/

void routine_entry(SYMTAB_NODE_PTR rtn_idp)  /* new routine's id */
{
  SYMTAB_NODE_PTR var_idp;    /* local variable id */

  trace_routine_entry(rtn_idp);

  /*
  --  Switch to the new code segment.
  */
  code_segmentp = rtn_idp->defn.info.routine.code_segment;

  /*
  --  Allocate local variables.
  */
  for (var_idp = rtn_idp->defn.info.routine.locals; var_idp != NULL; var_idp = var_idp->next)
    alloc_local(var_idp->typep);
}

/*--------------------------------------------------------------*/
/*  routine_exit        Deallocate the routine's parameters and */
/*                      locals.  Cut off its stack frame, and   */
/*                      return to the caller's code segment.    */
/*--------------------------------------------------------------*/

void routine_exit(SYMTAB_NODE_PTR rtn_idp) /* exiting routine's id */
{
  SYMTAB_NODE_PTR        idp;     /* variable or parm id */
  STACK_FRAME_HEADER_PTR hp;      /* ptr to stack frame header */

  trace_routine_exit(rtn_idp);

  /*
  --  Deallocate parameters and local variables.
  */
  for (idp = rtn_idp->defn.info.routine.parms; idp != NULL; idp = idp->next)
    free_data(idp);
  for (idp = rtn_idp->defn.info.routine.locals; idp != NULL; idp = idp->next)
    free_data(idp);

  /*
  --  Pop off the stack frame and return to the
  --  caller's code segment.
  */
  hp = (STACK_FRAME_HEADER_PTR) stack_frame_basep;
  code_segmentp = hp->return_address.address;
  tos = (rtn_idp->defn.key == PROC_DEFN)
    ? stack_frame_basep - 1
    : stack_frame_basep;
  stack_frame_basep = (STACK_ITEM_PTR) hp->dynamic_link.address;
}

/*--------------------------------------------------------------*/
/*  push_stack_frame_header     Allocate the callee routine's   */
/*                              stack frame.                    */
/*--------------------------------------------------------------*/

void push_stack_frame_header(int old_level, int new_level) /* levels of caller and callee */
{
  STACK_FRAME_HEADER_PTR hp;

  push_integer(0);                            /* return value */
  hp = (STACK_FRAME_HEADER_PTR) stack_frame_basep;

  /*
  --  Static link.
  */
  if (new_level == old_level + 1) {
    /*
    --  Calling a routine nested within the caller:
    --  Push pointer to caller's stack frame.
    */
    push_address(hp);
  } else if (new_level == old_level) {
    /*
    --  Calling another routine at the same level:
    --  Push pointer to stack frame of common parent.
    */
    push_address(hp->static_link.address);
  }  else  /* new_level < old_level */  {
    /*
    --  Calling a routine at a lesser level (nested less deeply):
    --  Push pointer to stack frame of nearest common ancestor.
    */
    int delta = old_level - new_level;

    while (delta-- >= 0)
      hp = (STACK_FRAME_HEADER_PTR) hp->static_link.address;
    push_address(hp);
  }

  push_address(stack_frame_basep);            /* dynamic link */
  push_address(0);    /* return address to be filled in later */
}

/*--------------------------------------------------------------*/
/*  alloc_local         Allocate a local variable on the stack. */
/*--------------------------------------------------------------*/

void alloc_local(TYPE_STRUCT_PTR tp) /* ptr to type of variable */
{
  if      (tp == integer_typep) push_integer(0);
  else if (tp == real_typep)    push_real(0.0);
  else if (tp == boolean_typep) push_byte(0);
  else if (tp == char_typep)    push_byte(0);

  else switch (tp->form) {
  	case ENUM_FORM:
	    push_integer(0);
	    break;

	  case SUBRANGE_FORM:
	    alloc_local(tp->info.subrange.range_typep);
	    break;

  	case ARRAY_FORM: {
	    char *ptr = alloc_bytes(tp->size);

	    push_address((ADDRESS) ptr);
	    break;
	  }

	  case RECORD_FORM: {
	    char *ptr = alloc_bytes(tp->size);

	    push_address((ADDRESS) ptr);
	    break;
	  }
    default:
      break;
  }
}

/*--------------------------------------------------------------*/
/*  free_data           Deallocate the data area of an array    */
/*                      or record local variable or value       */
/*                      parameter.                              */
/*--------------------------------------------------------------*/

void free_data(SYMTAB_NODE_PTR idp) /* parm or variable id */
{
  STACK_ITEM_PTR  itemp;              /* ptr to stack item */
  TYPE_STRUCT_PTR tp = idp->typep;    /* ptr to id's type */

  if (((tp->form == ARRAY_FORM) || (tp->form == RECORD_FORM)) && (idp->defn.key != VARPARM_DEFN)) {
	  itemp = stack_frame_basep + idp->defn.info.data.offset;
	  free(itemp->address);
  }
}

