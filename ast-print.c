/*
 * File: ast-print.c
 * Author: Saumya Debray
 * Purpose: Code to print syntax trees
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

char *opname(NodeType ntype);
static void print_ast_formatted(void *tree, int n, int nl);

/*
 * print_ast(tree) takes a pointer to an AST node and uses the getter
 * functions supplied by the user to traverse and print the tree below
 * that node.
 */
void print_ast(void *tree) {
  print_ast_formatted(tree, 0, 1);
}


/*******************************************************************************
 *                                                                             *
 *                                   HELPERS                                   *
 *                                                                             *
 ******************************************************************************/

/*
 * indent(n) : print out n spaces
 */
static void indent(int n) {
  assert(n >= 0);
  while (n-- > 0) putchar(' ');
}

#define SPACES_PER_INDENTATION_LEVEL  4

/*
 * print_ast_formatted(tree, n) takes a pointer to an AST node and uses the 
 * getter functions supplied by the user to traverse and print the tree.  The
 * second argument specifies a left-indentation level.  The third argument
 * specifies whether a newline should be printed at the end.
 */
static void print_ast_formatted(void *tree, int n, int nl) {
  NodeType ntype;
  char *name;
  void *list_hd, *list_tl;
  int i, nargs;

  int indent_amt = n * SPACES_PER_INDENTATION_LEVEL;

  if (tree == NULL) {
    return;
  }

  ntype = ast_node_type(tree);
  
  switch (ntype) {
  case FUNC_DEF:
    name = func_def_name(tree);
    printf("func_def: %s\n", name);  /* print the function's name */

    printf("  formals: ");           /* print the function's formals */
    nargs = func_def_nargs(tree);
    for (i = 1; i <= nargs; i++) {
      printf("%s", func_def_argname(tree, i));
      if (i < nargs) printf(", ");
    }

    printf("\n  body:\n");           /* print the function's body */
    print_ast_formatted(func_def_body(tree), n+1, 1);
    printf("/* func_def: %s */\n\n", name);
    break;

  case FUNC_CALL:
    indent(indent_amt);
    name = func_call_callee(tree);
    printf("%s(", name);  /* print the callee's name */
    print_ast_formatted(func_call_args(tree), 0, 0);   /* print the argument list */
    printf(")");
    if (nl != 0) {
      printf("\n");
    }
    break;

  case STMT_LIST:
    indent(indent_amt);
    printf("{\n");
    while (tree != NULL) {
      list_hd = stmt_list_head(tree);
      tree = stmt_list_rest(tree);
      print_ast_formatted(list_hd, n+1, nl);
    }
    indent(indent_amt);
    printf("}\n");
    break;

  case IF:
    indent(indent_amt); printf("if (");
    print_ast_formatted(stmt_if_expr(tree), 0, 0);
    printf("):\n");
    indent(indent_amt); printf("then:\n");
    print_ast_formatted(stmt_if_then(tree), n+1, nl);
    indent(indent_amt); printf("else:\n");
    print_ast_formatted(stmt_if_else(tree), n+1, nl);
    indent(indent_amt);
    printf("end_if\n");
    break;

  case ASSG:
    indent(indent_amt);
    printf("%s = ", stmt_assg_lhs(tree));
    print_ast_formatted(stmt_assg_rhs(tree), 0, 0);
    printf("\n");
    break;

  case WHILE:
    indent(indent_amt); printf("while (");
    print_ast_formatted(stmt_while_expr(tree), 0, 0);
    printf("):\n");
    print_ast_formatted(stmt_while_body(tree), n+1, 1);
    indent(indent_amt);
    printf("end_while\n");
    break;

  case RETURN:
    indent(indent_amt);
    printf("return: ");
    print_ast_formatted(stmt_return_expr(tree), 0, 0);
    printf("\n");
    break;

  case EXPR_LIST:
    list_tl = expr_list_rest(tree);
    print_ast_formatted(expr_list_head(tree), 0, 0);
    if (list_tl != NULL) {
      printf(", ");
    }
    print_ast_formatted(list_tl, 0, 0);
    break;

  case IDENTIFIER:
    printf("%s", expr_id_name(tree));
    break;

  case INTCONST:
    printf("%d", expr_intconst_val(tree));
    break;
    
  case UMINUS:
    printf("-(");
    print_ast_formatted(expr_operand_1(tree), 0, 0);
    printf(")");
    break;

  case EQ:
  case NE:
  case LE:
  case LT:
  case GE:
  case GT:
    print_ast_formatted(expr_operand_1(tree), 0, 0);
    printf(" %s ", opname(ntype));
    print_ast_formatted(expr_operand_2(tree), 0, 0);
    break;

  case ADD:
  case SUB:
  case MUL:
  case DIV:
    printf("(");
    print_ast_formatted(expr_operand_1(tree), 0, 0);
    printf(" %s ", opname(ntype));
    print_ast_formatted(expr_operand_2(tree), 0, 0);
    printf(")");
    break;

  case AND:
  case OR:
    printf("(");
    print_ast_formatted(expr_operand_1(tree), 0, 0);
    printf(") %s (", opname(ntype));
    print_ast_formatted(expr_operand_2(tree), 0, 0);
    printf(")");
    break;

  default:
    fprintf(stderr, "*** [%s] Unrecognized syntax tree node type %d\n",
	    __func__, ntype);
  }
}

/*
 * opname: return a string that is a readable representation of an
 * operator.
 */
char *opname(NodeType ntype) {
  switch (ntype) {
  case EQ:
    return "==";
  case NE:
    return "!=";
  case LE:
    return "<=";
  case LT:
    return "<";
  case GE:
    return ">=";
  case GT:
    return ">";
  case ADD:
    return "+";
  case SUB:        /* fall through */
  case UMINUS:
    return "-";
  case MUL:
    return "*";
  case DIV:
    return "/";
  case AND:
    return "&&";
  case OR:
    return "||";
    
  default:
      fprintf(stderr, "*** [%s] Unrecognized syntax tree node type %d\n", __func__, ntype);
      return NULL;
  }
}
