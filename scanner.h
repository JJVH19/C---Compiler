/*
 * File: scanner.h
 * Author: Saumya Debray
 * Purpose: Lists tokens and their values for use in the CSC 453 project
 */

#ifndef __SCANNER_H__
#define __SCANNER_H__

/*
 * The enum Token defines integer values for the various tokens.  These
 * are the values returned by the scanner.
 */
typedef enum {
  UNDEF     /* undefined */,
  ID        /* identifier: e.g., x, abc, p_q_12 */,
  INTCON    /* integer constant: e.g., 12345 */,
  LPAREN    /* '(' : Left parenthesis */,
  RPAREN    /* ')' : Right parenthesis */,
  LBRACE    /* '{' : Left curly brace */,
  RBRACE    /* '}' : Right curly brace */,
  COMMA     /* ',' : Comma */,
  SEMI      /*	;  : Semicolon */,
  kwINT     /*	int */,
  kwIF      /*	if */,
  kwELSE    /*	else */,
  kwWHILE   /*	while */,
  kwRETURN  /*	return */,
  opASSG    /*	= : Assignment */,
  opADD     /*	+ : addition */,
  opSUB     /*	– : subtraction */,
  opMUL     /*	* : multiplication */,
  opDIV     /*	/ : division */,
  opEQ      /*	== : Op: equals */,
  opNE      /*	!= : op: not-equals */,
  opGT      /*	>  : Op: greater-than */,
  opGE      /*	>= : Op: greater-or-equal */,
  opLT      /*	<  : Op: less-than */,
  opLE      /*	<= : Op: less-or-equal */,
  opAND     /*	&& : Op: logical-and */,
  opOR      /*	|| : Op: logical-or */,
  opNOT     /* ! : Op: logical-not */
} Token;

#endif  /* __SCANNER_H__ */
