/****************************************************************/
/*                                                              */
/*      P A R S I N G   R O U T I N E S   (Header)              */
/*                                                              */
/*      FILE:       parser.h                                    */
/*                                                              */
/*      MODULE:     parser                                      */
/*                                                              */
/*      Copyright (c) 1991 by Ronald Mak                        */
/*      For instructional purposes only.  No warranties.        */
/*                                                              */
/****************************************************************/

#ifndef parser_h
#define parser_h

#include <stdlib.h>
#include <stdbool.h>
#include "common.h"
#include "symtab.h"

// #define analyze

/*--------------------------------------------------------------*/
/*  Uses of a variable                                          */
/*--------------------------------------------------------------*/

typedef enum {
    EXPR_USE, TARGET_USE, VARPARM_USE,
} USE;

/*--------------------------------------------------------------*/
/*  Functions                                                   */
/*--------------------------------------------------------------*/

TYPE_STRUCT_PTR expression();
TYPE_STRUCT_PTR variable(SYMTAB_NODE_PTR var_idp,  USE use); 
TYPE_STRUCT_PTR routine_call(SYMTAB_NODE_PTR rtn_idp, bool parm_check_flag);
TYPE_STRUCT_PTR base_type(TYPE_STRUCT_PTR tp);
TYPE_STRUCT_PTR standard_routine_call(SYMTAB_NODE_PTR rtn_idp); 
bool is_assign_type_compatible();
void statement(void);
void declarations(SYMTAB_NODE_PTR rtn_idp);
void compound_statement(void);
void routine(void);
void actual_parm_list(SYMTAB_NODE_PTR rtn_idp, bool parm_check_flag);
void program(void); 
void crunch_symtab_node_ptr(SYMTAB_NODE_PTR np);
void crunch_token(void);

		/********************************/
		/*                              */
		/*      Macros for parsing      */
		/*                              */
		/********************************/

/*--------------------------------------------------------------*/
/*  if_token_get                If token equals token_code, get */
/*                              the next token.                 */
/*--------------------------------------------------------------*/

#define if_token_get(token_code)		\
    if (token == token_code) get_token()

/*--------------------------------------------------------------*/
/*  if_token_get_else_error     If token equals token_code, get */
/*                              the next token, else error.     */
/*--------------------------------------------------------------*/

#define if_token_get_else_error(token_code, error_code)	\
    if (token == token_code) get_token(); 		          \
    else error(error_code)

/*--------------------------------------------------------------*/
/*  Analysis routine calls      Unless the following statements */
/*                              are preceded by                 */
/*                                                              */
/*                                      #define analyze         */
/*                                                              */
/*                              calls to the analysis routines  */
/*                              are not compiled.               */
/*--------------------------------------------------------------*/

#ifdef analyze
void analyze_const_defn(SYMTAB_NODE_PTR idp);
void analyze_type_defn(SYMTAB_NODE_PTR idp);
void analyze_var_decl(SYMTAB_NODE_PTR idp);
void analyze_routine_header(SYMTAB_NODE_PTR rtn_idp);
void analyze_block(char *code_segment);
#else
#define analyze_const_defn(idp)
#define analyze_var_decl(idp)
#define analyze_type_defn(idp)
#define analyze_routine_header(idp)
#define analyze_block(idp)
#endif

#endif

