/*
 * File: ast.h
 * Author: Saumya Debray
 * Purpose: typedefs and prototypes for syntax trees
 */

#ifndef __AST_H__
#define __AST_H__

/*******************************************************************************
 *                                                                             *
 *                                AST NODE TYPES                               *
 *                                                                             *
 *******************************************************************************/

typedef enum {
  DUMMY,            /* a placeholder */
  FUNC_DEF,         /* function definition */
  FUNC_CALL,        /* function call */
  IF,               /* if statement */
  WHILE,            /* while statement */
  ASSG,             /* assignment statement */
  RETURN,           /* return statement */
  STMT_LIST,        /* list of statements */
  EXPR_LIST,        /* list of expressions */
  IDENTIFIER,       /* identifier */
  INTCONST,         /* integer constant */
  EQ,               /* == */
  NE,               /* != */
  LE,               /* <= */
  LT,               /* < */
  GE,               /* >= */
  GT,               /* > */
  ADD,              /* + */
  SUB,              /* - (binary) */
  MUL,              /* * */
  DIV,              /* / */
  UMINUS,           /* - (unary) */
  AND,              /* && */
  OR                /* || */
} NodeType;

/*******************************************************************************
 *                                                                             *
 *                           GETTERS FOR SYNTAX TREES                          *
 *                                                                             *
 *******************************************************************************/

/* 
 * ptr: an arbitrary non-NULL AST pointer; ast_node_type() returns the node type 
 * for the AST node ptr points to.
 */
NodeType ast_node_type(void *ptr);

/* 
 * ptr: pointer to an AST for a function definition; func_def_name() returns 
 * a pointer to the function name (a string) of the function definition AST that
 * ptr points to.
 */
char * func_def_name(void *ptr);

/*
 * ptr: pointer to an AST for a function definition; func_def_nargs() returns 
 * the number of formal parameters for that function.
 */
int func_def_nargs(void *ptr);

/*
 * ptr: pointer to an AST for a function definition, n is an integer. If n > 0
 * and n <= no. of arguments for the function, then func_def_argname() returns 
 * a pointer to the name (a string) of the nth formal parameter for that function;
 * the first formal parameter corresponds to n == 1.  If the value of n is outside
 * these parameters, the behavior of this function is undefined.
 */
char *func_def_argname(void *ptr, int n);

/* 
 * ptr: pointer to an AST for a function definition; func_def_body() returns 
 * a pointer to the AST that is the function body of the function that ptr
 * points to.
 */
void * func_def_body(void *ptr);

/*
 * ptr: pointer to an AST node for a function call; func_call_callee() returns 
 * a pointer to a string that is the name of the function being called.
 */
char * func_call_callee(void *ptr);

/*
 * ptr: pointer to an AST node for a function call; func_call_args() returns 
 * a pointer to the AST that is the list of arguments to the call.
 */
void * func_call_args(void *ptr);

/*
 * ptr: pointer to an AST node for a statement list; stmt_list_head() returns 
 * a pointer to the AST of the statement at the beginning of this list.
 */
void * stmt_list_head(void *ptr);

/*
 * ptr: pointer to an AST node for a statement list; stmt_list_rest() returns 
 * a pointer to the AST of the rest of this list (i.e., the pointer to the
 * next node in the list).
 */
void * stmt_list_rest(void *ptr);

/*
 * ptr: pointer to an AST node for an expression list; expr_list_head() returns 
 * a pointer to the AST of the expression at the beginning of this list.
 */
void * expr_list_head(void *ptr);

/*
 * ptr: pointer to an AST node for an expression list; expr_list_rest() returns 
 * a pointer to the AST of the rest of this list (i.e., the pointer to the
 * next node in the list).
 */
void * expr_list_rest(void *ptr);

/*
 * ptr: pointer to an AST node for an IDENTIFIER; expr_id_name() returns a 
 * pointer to the name of the identifier (a string).
 */
char *expr_id_name(void *ptr);

/*
 * ptr: pointer to an AST node for an INTCONST; expr_intconst_val() returns the
 * integer value of the constant.
 */
int expr_intconst_val(void *ptr);

/*
 * ptr: pointer to an AST node for an arithmetic or boolean expression.
 * expr_operand_1() returns a pointer to the AST of the first operand.
 */
void * expr_operand_1(void *ptr);

/*
 * ptr: pointer to an AST node for an arithmetic or boolean expression.
 * expr_operand_2() returns a pointer to the AST of the second operand.
 */
void * expr_operand_2(void *ptr);

/*
 * ptr: pointer to an AST node for an IF statement.  stmt_if_expr() returns
 * a pointer to the AST for the expression tested by the if statement.
 */
void * stmt_if_expr(void *ptr);

/*
 * ptr: pointer to an AST node for an IF statement.  stmt_if_then() returns
 * a pointer to the AST for the then-part of the if statement, i.e., the
 * statement to be executed if the condition is true.
 */
void * stmt_if_then(void *ptr);

/*
 * ptr: pointer to an AST node for an IF statement.  stmt_if_else() returns
 * a pointer to the AST for the else-part of the if statement, i.e., the
 * statement to be executed if the condition is false.
 */
void * stmt_if_else(void *ptr);

/*
 * ptr: pointer to an AST node for an assignment statement.  stmt_assg_lhs()
 * returns a pointer to the name of the identifier on the LHS of the
 * assignment.
 */
char *stmt_assg_lhs(void *ptr);

/*
 * ptr: pointer to an AST node for an assignment statement.  stmt_assg_rhs()
 * returns a pointer to the AST of the expression on the RHS of the assignment.
 */
void *stmt_assg_rhs(void *ptr);

/*
 * ptr: pointer to an AST node for a while statement.  stmt_while_expr()
 * returns a pointer to the AST of the expression tested by the while statement.
 */
void *stmt_while_expr(void *ptr);

/*
 * ptr: pointer to an AST node for a while statement.  stmt_while_body()
 * returns a pointer to the AST of the body of the while statement.
 */
void *stmt_while_body(void *ptr);

/*
 * ptr: pointer to an AST node for a return statement.  stmt_return_expr()
 * returns a pointer to the AST of the expression whose value is returned.
 */
void *stmt_return_expr(void *ptr);

/*******************************************************************************
 *                                                                             *
 *                         TOP-LEVEL AST PRINT ROUTINE                         *
 *                                                                             *
 *******************************************************************************/

/*
 * print_ast(tree) takes a pointer to an AST node and uses the getter
 * functions supplied by the user to traverse and print the tree below
 * that node.
 */
void print_ast(void *tree);

#endif  /* __AST_H__ */
