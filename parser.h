//
// Created by Jose Juan Velasquez on 2/15/2024.
//

#include "ast.h"

#ifndef MILESTONE_1_PARSER_H
#define MILESTONE_1_PARSER_H

#endif //MILESTONE_1_PARSER_H

//The types used within the three-address code instructions
typedef enum InstrType{
    ASSGN, //0
    PARAM, //1
    CALL, //2
    RETRIEVE, //3
    IF_EQ, //4
    IF_NE, //5
    IF_LE, //6
    IF_LT, //7
    IF_GE, //8
    IF_GT, //9
    GOTO, //10
    LABEL, //11
    RET, //12
    UNARY_MINUS, //13
    PLUS, //14
    MINUS, //15
    MULTIPLY, //16
    DIVIDE, //17
    LEAVE //18
}InstrType;

//The nodes for the three-address code instructions
typedef struct ThreeAddressCodeNode{
    InstrType op;
    SymbolTableNode *src1;
    SymbolTableNode *src2;
    SymbolTableNode * dest;
    struct ThreeAddressCodeNode *next;
} ThreeAddressCodeNode;

//The nodes for the AST
typedef struct ASTnode {
    //What type of node
    NodeType ntype;
    //Variables, function calls
    char* st_ref;
    //Func_call name
    char* callee;
    //Integer constants;
    int num;
    //Parameter counter
    int parameters;
    //The current line this node is currently at within the stdin
    int line_counter;
    //Pointer reference to the Three address instr head
    ThreeAddressCodeNode *codeHead;
    //Pointer reference to the Three address instr tail
    ThreeAddressCodeNode **codeTail;
    //Pointer to a temp variable
    SymbolTableNode *place;
    //Children pointers
    struct ASTnode *child0, *child1, *child2;
} ASTnode;

//The AST root
typedef struct AST {
    ASTnode *root;
} AST;

//The parser getting called
int parse();

//<prog> ::= <func_defn> <prog> | E
void prog();

//<opt_formals> ::= <formals> | E
ASTnode* opt_formals();

//<opt_var_decls> ::= <var_decl> <opt_var_decls> | E
void opt_var_decls();

//<opt_stmt_list> ::= <stmt> <opt_stmt_list> | E
ASTnode* opt_stmt_list();

/*<stmt> ::= <fn_call> SEMI
 *          | <while_stmt>
 *          | <if_stmt>
 *          | <assg_or_fn_call>
 *          | <return_stmt>
 *          | LBRACE <opt_stmt_list> RBRACE
 *          | SEMI
 */
ASTnode* stmt();

//<fn_call> ::= ID LPAREN <opt_expr_list> RPAREN
ASTnode* fn_call(char* currentLexeme);

//<opt_expr_list> ::= E | <expr_list>
ASTnode* opt_expr_list();

//<expr_list> ::= <arith_exp> COMMA <expr_list> | <arith_exp>
ASTnode* expr_list();

//Verifies if current token matches the grammar
void match(Token expected);

//<formals> ::= <type> ID COMMA <formals> | <type> ID
ASTnode* formals();

//<var_or_func> ::= SEMI | <decl_or_func>
void var_or_func();

//<decl_or_func> ::= COMMA <id_list> SEMI
//                 | LPAREN <opt_formals> RPAREN LBRACE <opt_var_decls> <opt_stmt_list> RBRACE
ASTnode* decl_or_func();

//<id_list> ::= ID | ID COMMA <id_list>
void id_list();

//<var_decl> := <type> <id_list> SEMI
void var_decl();

//<while_stmt> ::= kwWHILE LPAREN <bool_exp> RPAREN <stmt>
ASTnode* while_stmt();

//<if_stmt> ::= kwIF LPAREN <bool_exp> RPAREN <stmt>
//            | kwIF LPAREN <bool_exp> RPAREN <stmt> kwELSE <stmt>
ASTnode* if_stmt();

//<return_stmt> ::= kwRETURN SEMI | kwRETURN <arith_exp> SEMI
ASTnode* return_stmt();

//<assg_or_fn_call> ::= ID <assg_stmt> | ID <fn_call>
ASTnode* assg_or_fn_call();

//<assg_stmt> ::= opASSG <arith_exp> SEMI
ASTnode* assg_stmt();

/*** These functions were implemented based on the pseudo code found within Prof. Debray's lecture slides ***/

//<arith_exp> ::= <arith_exp1> <opt_add_sub>
ASTnode* arith_exp();

//<opt_add_sub> ::= opADD <arith_exp1> <opt_add_sub> | opSUB <arith_exp1> <opt_add_sub> | E
ASTnode* opt_add_sub(ASTnode* left);

//<arith_exp1> ::= <arith_exp2> <opt_mul_div>
ASTnode* arith_exp1();

//<opt_mul_div> ::= opMUL <arith_exp2> <opt_mul_div> | opDIV <arith_exp2> <opt_mul_div> | E
ASTnode* opt_mul_div(ASTnode* left);

//<arith_exp2> ::= opSUB <arith_exp2> | <arith_exp3>
ASTnode* arith_exp2();

//<arith_exp3> ::= ID <opt_func_call> | INTCON | LPAREN <arith_exp> RPAREN
ASTnode* arith_exp3();

//<opt_func_call> ::= <fn_call> | E
ASTnode* opt_func_call(char* currentLexeme);

//<bool_exp> ::= <bool_exp1> <opt_or>
ASTnode* bool_exp();

//<opt_or> ::= || <bool_exp1> <opt_or> | E
ASTnode* opt_or(ASTnode* left);

//<bool_exp1> ::= <bool_exp2> <opt_and>
ASTnode* bool_exp1();

//<opt_and> ::= && <bool_exp2> <opt_and> | E
ASTnode* opt_and(ASTnode* left);

//<bool_exp2> ::= <arith_expr> <relop> <arith_exp>
ASTnode* bool_exp2();

//<relop> ::= opEQ | opNE | opLE | opLT | opGE | opGT
NodeType relop();

//Makes an AST node connecting the left child and the right child to a parent node, which will return
ASTnode *mk_node(NodeType ntype, ASTnode *left, ASTnode *right);

/*** End of functions implemented with Prof. Debray's pseudocode idea from the lecture slides ***/

//Creates the ast root pointer
AST* createAST();

//Helper function that creates and initializes the ast nodes
ASTnode* createASTNode();

//Creates the three-address code node
ThreeAddressCodeNode *createThreeAddressCodeNode();

//Frees the AST after we're done
void freeAST();

//Helper function to free all the nodes of the AST
void freeASTnode(ASTnode *node);

/////////////////////////////////////************************* MIPS GENERATION ************************/////////////////////////////////////

//Traverses the AST and creates the three-address code instructions as we traverse
void traverseAST(ASTnode *astNode);

//Traverses the AST and creates the three-address code instructions as we traverse
ThreeAddressCodeNode *newinstr(InstrType opType, SymbolTableNode *src1, SymbolTableNode *src2, SymbolTableNode *dest);

//Creates temp variable and stores it in the local symbol table
SymbolTableNode * newtemp(TYPE type);

//Creates the Labels used in code generation
ThreeAddressCodeNode *newlabel();

//Generates three-address code instructions for the statements
void codeGen_stmt(ASTnode *node);

//Generates three-address code instructions for the expressions
void codeGen_expr(ASTnode *node);

//Generates code for the boolean expressions
void codeGen_Bool(ASTnode* node, SymbolTableNode *trueDest, SymbolTableNode* falseDest);

//Initializes three address code generation and MIPS code printing
void mipsDriver();

//Prints code for the hardcoded println and main functions in MIPS
void printHardcodedFunctions();

//Function will generate MIPS assembly code from the three-address instruction list
void generateMIPSCode(ThreeAddressCodeNode* head, char* funcNameMIPS);

//Prints code for a list of global variables
void printGlobalVariableList();

//Prints the list of three-address code instructions for debugging
void printThreeAddressCodeNode(ThreeAddressCodeNode* node, int pla);

//Sets the address for the parameters from rightmost to leftmost parameters
void setFormalParameterAddresses(ASTnode *node);

//Helper function to free all the nodes of the three-address code
void freeThreeAddressCodeNode(ThreeAddressCodeNode *node);