
//
// Created by Jose Juan Velasquez on 2/1/2024.
//

#include "semantics.c"
#include "ast-print.c"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

//Global variables used in parser, scanner, and semantic checking files
int token;
int var_counter;
int temp_counter;
int currentAddress;
int numOfOperations;
int paramAddress;
int paramOperations = 0;
int label_num = 0;
char *functionName;
char* prevVar;
char* prevLexemeCopy;
char* functionNameCopy;
char* copyLexeme;
extern char* lexeme;
extern int bufferSize;
extern int chk_decl_flag;      /* set to 1 to do semantic checking */
extern int print_ast_flag;     /* set to 1 to print out the AST */
extern int gen_code_flag;      /* set to 1 to generate code */
bool local;
bool fn_call_check;
bool freePrevLexeme;
bool ASTFree;
bool freeFunctionName;
bool justSearch;
bool prevVarDecl;
bool freePrevLexemeCopy;
bool freeFunctionNameCopy;
bool freeCopyLexeme;
bool addGlobal;
bool returnExists;
SymbolTable *symbolTable;
AST *ast;

//Will go through the FUNC_CALL node arg_list, find its depth and compare with original function param count
void checkParamCount(ASTnode *node, char* funcName) {
    //Make a pointer to traverse the tree's EXPR_LIST nodes
    ASTnode *args = node;
    int paramCounter = 0;
    //Traverses all the parameters of the function call until it reaches the end
    while (args != NULL) {
        //Increment counter meaning we found another parameter
        paramCounter++;
        //Move to next parameter
        args = args->child1;
    }
    //Compare final count value for paramCounter and the actual number of parameters it should have
    if (!compareFunctionCallParameters(symbolTable->next->root, funcName, paramCounter)) {
        freeExitError(node->line_counter);
    }
}

//Will traverse the built AST and find the FUNC_CALL nodes
void checkFuncCallParamCount(ASTnode * node) {
    if (node == NULL) {
        return;
    }
    //Post order traversal
    checkFuncCallParamCount(node->child0);
    checkFuncCallParamCount(node->child1);
    checkFuncCallParamCount(node->child2);

    //If we find a FUNC_CALL node, count its parameters here
    if (node->ntype == FUNC_CALL) {
        checkParamCount(node->child0, node->callee);
    }
}

//Creates the ExprList root pointer
AST* createAST() {
    AST *root = (AST *) calloc(1, sizeof(AST));
    return root;
}

//Helper function that creates and initializes the ExprList nodes
ASTnode* createASTNode() {
    ASTnode *node = (ASTnode *) calloc(1, sizeof(ASTnode));
    node->ntype = INT_MIN;
    node->num = INT_MIN;
    node->parameters = INT_MIN;
    node->callee = NULL;
    node->st_ref = NULL;
    node->codeHead = NULL;
    node->codeTail = &node->codeHead;
    node->place = NULL;
    node->child0 = NULL;
    node->child1 = NULL;
    node->child2 = NULL;
    return node;
}

//Creates the three-address code node
ThreeAddressCodeNode* createThreeAddressCodeNode() {
    ThreeAddressCodeNode *node = (ThreeAddressCodeNode *) calloc(1, sizeof(ThreeAddressCodeNode));
    node->op = INT_MIN;
    node->src1 = NULL;
    node->src2 = NULL;
    node->dest = NULL;
    node->next = NULL;
    return node;
}

//This function has code and code styles shown in Prof. Saumya Debray's lecture slides
int parse() {
    //Create root for the local symbol table
    symbolTable = createSymbolTableRoot();

    //Hardcode bufferSize to match its equivalent used in semantics.c to avoid writing errors
    bufferSize = 7;
    //Add function "println" to the global symbol table
    populateSymbolTable(function, "println", true);

    //Update println's parameter count to 1
    updateSymbolTableParameters(symbolTable->next->root, "println", 1);

    //Gets the first token to start parsing
    token = get_token();
    prog();

    //If gen_code_flag is set, generate MIPS code hardcoded functions
    if (gen_code_flag == 1) {
        printHardcodedFunctions();
    }

    match(EOF);

    //Print the Symbol Table after populating it
    //printSymbolTable();

    //Free global symbol table
    freeSymbolTable();

    return 0;
}

//Verifies if current token matches the grammar
//This function has code and code styles shown in Prof. Saumya Debray's lecture slides
void match(Token expected) {
    if (token == expected) {
        //If we have an ID, start the symbol table checking
        if (token == ID && chk_decl_flag) {
            //Add to or check the semantic tables
            manageSymbolTable();
        } else {
            //Just get the token without semantic checking
            token = get_token();
        }
    } else {
        //Prints error message and exits with code 1
        freeExitError(-1);
    }
}

//<prog> ::= <type> ID <var_or_func> <prog> | E
void prog() {
    //Flag variables indicating specific semantic checks used within the semantics.c file
    local = false;
    fn_call_check = false;
    justSearch = false;

    //Flags used to avoid segfaults when freeing their corresponding variables
    ASTFree = false;
    prevVarDecl = false;
    freeFunctionName = false;
    freePrevLexemeCopy = false;
    freeFunctionNameCopy = false;

    //Variables used for MIPS code generation
    temp_counter = 0;
    currentAddress = -4;
    numOfOperations = -4;
    paramAddress = 8;

    //Flag used know if we need to print the global variables in code generation
    addGlobal = true;
    returnExists = false;

    //<type> ID <var_or_func> <prog> | E
    if (token == kwINT) {
        //<type>
        match(kwINT);
        //Avoids errors
        if (token == ID) {
            //Save function ID to compare parameters later on
            functionName = (char *) calloc(bufferSize + 1, sizeof(char));
            functionName = strcpy(functionName, lexeme);
            freeFunctionName = true;

            //Save current function ID here as well to be used in code generation
            functionNameCopy = (char *) calloc(bufferSize + 1, sizeof(char));
            functionNameCopy = strcpy(functionNameCopy, lexeme);
            freeFunctionNameCopy = true;
        }
        //ID
        match(ID);
        //<var_or_func>
        var_or_func();
        //<prog>
        prog();
    } else {
        //E
        return;
    }
}

//<var_or_func> ::= SEMI | <decl_or_func>
void var_or_func() {
    //Single variable declaration
    if (token == SEMI) {
        //If flag is active, generate code for a single global variable
        if (gen_code_flag) {
            fprintf(stdout,".data\n_%s: .space 4\n",functionNameCopy);
        }

        //Confirming we're done updating the function's parameter count
        free(functionName);
        freeFunctionName = false;

        //Confirming we're done updating the function's MIPS equivalent
        free(functionNameCopy);
        freeFunctionNameCopy = false;

        //SEMI
        match(SEMI);
    } else if (token == COMMA || token == LPAREN) {
        //Create the root pointer of the AST
        ast = createAST();
        //Indicates to free the tree in case we exit due to an error
        ASTFree = true;
        //<decl_or_func> and creates the AST
        ast->root = decl_or_func();

        //Print the Symbol Table after populating it
        //printSymbolTable();

        //Code generation if the AST was populated
        if (gen_code_flag == 1 && ast->root != NULL) {
            mipsDriver();
        }

        //Confirming we're done using the function's name MIPS equivalent
        free(functionNameCopy);
        freeFunctionNameCopy = false;

        //Free the AST
        freeAST();

        //Free the local symbol table
        freeLocalSymbolTable();

        //Can continue as normal since no error occurred here
        ASTFree = false;
    } else {
        //Syntax error, print message
        match(SEMI);
    }
}

//<decl_or_func> ::= COMMA <id_list> SEMI | LPAREN <opt_formals> RPAREN LBRACE <opt_var_decls> <opt_stmt_list> RBRACE
ASTnode* decl_or_func() {
    if (token == COMMA) {
        //If flag is set, generate global variable list
        if (gen_code_flag) {
            printGlobalVariableList();
        }
        //Confirms we are not in the local scope when freeing
        local = false;
        //COMMA
        match(COMMA);
        //<id_list>
        id_list();
        //SEMI
        match(SEMI);
    } else if (token == LPAREN) {
        //Used in id_list(), it means we are now dealing with local variables for code generation
        addGlobal = false;

        //Create the node and make the root pointer point to it
        ASTnode *astNode = createASTNode();
        //Indicate within the AST node where it is located in the stdin
        astNode->line_counter = getLines();
        //Specify type of node
        astNode->ntype = FUNC_DEF;
        //Initialize/Reset parameter counter
        var_counter = 0;
        //Recognizes we're dealing with a local scope
        local = true;
        //Return a pointer to the name from the local symbol table
        astNode->st_ref = getIDFromSymbolTable(symbolTable->next->root, functionName);
        //LPAREN
        match(LPAREN);
        //<opt_formals>
        astNode->child1 = opt_formals();
        //Up to this point, var_counter will have the number of parameters the function has
        astNode->parameters = var_counter;
        //RPAREN
        match(RPAREN);
        //Change the parameter field for the function
        updateSymbolTableParameters(symbolTable->next->root, functionName, var_counter);
        //Confirming we're done updating the function's parameter count
        free(functionName);
        freeFunctionName = false;
        //Reset counter for later use
        var_counter = 0;
        //LBRACE
        match(LBRACE);
        //<opt_var_decls>
        opt_var_decls();
        //<opt_stmt_list>
        astNode->child0 = opt_stmt_list();
        //RBRACE
        match(RBRACE);

        //If chk_decl_flag is on, check the parameter count for each function call
        if (chk_decl_flag) {
            checkFuncCallParamCount(astNode);
        }

        //Prints the AST if flag is set
        if (print_ast_flag == 1) {
            print_ast(astNode);
        }

        //If true, we can free the copyLexeme used in arith_exp3()
        if (freeCopyLexeme) {
            freeCopyLexeme = false;
            free(copyLexeme);
        }
        //Recognizes we're out of the local scope
        local = false;
        return astNode;
    }
    return NULL;
}

//<id_list> ::= ID | ID COMMA <id_list>
void id_list() {
    //Store a copy of the current lexeme to update its address in the local symbol table
    freePrevLexemeCopy = true;
    prevLexemeCopy = (char*)calloc(bufferSize+1,sizeof(char));
    prevLexemeCopy = strcpy(prevLexemeCopy,lexeme);
    //ID
    match(ID);

    //Generate code for a global variable or update addresses for local variables
    if(gen_code_flag == 1) {
        printGlobalVariableList();
    }

    //frees the copy after we're done comparing
    free(prevLexemeCopy);
    freePrevLexemeCopy = false;
    if (token == COMMA) {
        //COMMA <id_list>
        match(COMMA);
        id_list();
    }
}

//<opt_formals> ::= <formals> | E
ASTnode *opt_formals() {
    if (token == kwINT) {
        //<formals>
        return formals();
    } else {
        //E
        return NULL;
    }
}

//<formals> ::= <type> ID COMMA <formals> | <type> ID
ASTnode* formals() {
    //<type>
    match(kwINT);
    prevVarDecl = true;
    //Create copy of current lexeme ID to use later
    prevVar = (char *) calloc(bufferSize + 1, sizeof(char));
    prevVar = strcpy(prevVar, lexeme);

    //ID
    match(ID);
    //Increment counter
    var_counter++;
    //Create node for formal parameter
    ASTnode *astNode = createASTNode();
    //Indicate within the AST node where it is located in the stdin
    astNode->line_counter = getLines();
    //Find the name in the symbol table
    astNode->st_ref = getIDFromSymbolTable(symbolTable->root, prevVar);
    if (astNode->st_ref == NULL) {
        //If not in local table, search global table
        astNode->st_ref = getIDFromSymbolTable(symbolTable->next->root, prevVar);
    }
    //We're done with prevVar, free it
    free(prevVar);
    prevVarDecl = false;
    if (token == COMMA) {
        //COMMA <formals>
        match(COMMA);
        astNode->child0 = formals();
    }
    return astNode;
}

//<opt_var_decls> ::= <var_decl> <opt_var_decls> | E
void opt_var_decls() {
    if (token == kwINT) {
        //<var_decl> <opt_var_decls>
        var_decl();
        opt_var_decls();
    } else {
        //E
        return;
    }
}

//<var_decl> := <type> <id_list> SEMI
void var_decl() {
    //<type>
    match(kwINT);
    //<id_list>
    id_list();
    //SEMI
    match(SEMI);
}

//<opt_stmt_list> ::= <stmt> <opt_stmt_list> | E
ASTnode* opt_stmt_list() {
    //Lets the function know if it should continue or return
    if (token == ID || token == kwWHILE ||
        token == kwIF || token == kwRETURN ||
        token == LBRACE || token == SEMI) {
        //Create node and assign its type
        ASTnode *astNode = createASTNode();
        //Indicate within the AST node where it is located in the stdin
        astNode->line_counter = getLines();
        astNode->ntype = STMT_LIST;

        //This flag will make sure that from here on out,
        // we should only focus on checking, not declaring variables
        justSearch = true;
        //<stmt>
        astNode->child0 = stmt();
        //<opt_stmt_list>
        astNode->child1 = opt_stmt_list();
        return astNode;
    } else {
        //At this point, we're done for just checking variables
        justSearch = false;
        //E
        return NULL;
    }
}

//<stmt> ::= <while_stmt>
//           | <if_stmt>
//           | ID <assg_or_fn_call>
//           | <return_stmt>
//           | LBRACE <opt_stmt_list> RBRACE
//           | SEMI
ASTnode* stmt() {
    ASTnode *astNode;
    //ID <assg_or_fn_call>
    if (token == ID) {
        //Save ID to compare parameters later on
        functionName = (char *) calloc(bufferSize + 1, sizeof(char));
        functionName = strcpy(functionName, lexeme);
        freeFunctionName = true;

        //ID
        match(ID);
        //astNode will be of type ASSG or FUNC_CALL
        astNode = assg_or_fn_call();

        //Confirming we're done using the ID, free it
        free(functionName);
        freeFunctionName = false;
    }
        //<while_stmt>
    else if (token == kwWHILE) {
        //astNode will be of type WHILE
        astNode = while_stmt();
    }
        //<if_stmt>
    else if (token == kwIF) {
        //astNode will be of type IF
        astNode = if_stmt();
    }
        //<return_stmt>
    else if (token == kwRETURN) {
        //astNode will be of type RETURN
        astNode = return_stmt();
    }
        //LBRACE <opt_stmt_list> RBRACE
    else if (token == LBRACE) {
        match(LBRACE);
        astNode = opt_stmt_list();
        match(RBRACE);
    } else {
        //SEMI
        match(SEMI);
        astNode = NULL;
    }
    return astNode;
}

//<while_stmt> ::= kwWHILE LPAREN <bool_exp> RPAREN <stmt>
ASTnode* while_stmt() {
    //Create node and assign its type
    ASTnode *astNode = createASTNode();
    //Indicate within the AST node where it is located in the stdin
    astNode->line_counter = getLines();
    //Declare the AST node's type
    astNode->ntype = WHILE;
    //kwWHILE
    match(kwWHILE);
    //LPAREN
    match(LPAREN);
    //<bool_exp>
    astNode->child0 = bool_exp();
    //RPAREN
    match(RPAREN);
    //<stmt>
    astNode->child1 = stmt();
    return astNode;
}

//<if_stmt> ::= kwIF LPAREN <bool_exp> RPAREN <stmt>
//            | kwIF LPAREN <bool_exp> RPAREN <stmt> kwELSE <stmt>
ASTnode* if_stmt() {
    //Create node and assign its type
    ASTnode *astNode = createASTNode();
    //Indicate within the AST node where it is located in the stdin
    astNode->line_counter = getLines();
    //Declare the AST node type
    astNode->ntype = IF;
    //kwIF
    match(kwIF);
    //LPAREN
    match(LPAREN);
    //<bool_exp>
    astNode->child0 = bool_exp();
    //RPAREN
    match(RPAREN);
    //<stmt>
    astNode->child1 = stmt();
    //kwELSE
    if (token == kwELSE) {
        match(kwELSE);
        //<stmt>
        astNode->child2 = stmt();
    }
    return astNode;
}

//<return_stmt> ::= kwRETURN SEMI | kwRETURN <arith_exp> SEMI
ASTnode* return_stmt() {
    //Create node and assign its type
    ASTnode *astNode = createASTNode();
    //Indicate within the AST node where it is located in the stdin
    astNode->line_counter = getLines();
    //Declare the AST node type
    astNode->ntype = RETURN;
    //kwRETURN
    match(kwRETURN);
    if (token == SEMI) {
        //SEMI
        match(SEMI);
    } else {
        //<arith_exp>
        astNode->child0 = arith_exp();
        //SEMI
        match(SEMI);
    }
    return astNode;
}

//<assg_or_fn_call> ::= <assg_stmt> | <fn_call> SEMI
ASTnode* assg_or_fn_call() {
    ASTnode *astNode = NULL;
    //<assg_stmt>
    if (token == opASSG) {
        astNode = createASTNode();
        //Indicate within the AST node where it is located in the stdin
        astNode->line_counter = getLines();

        //Create left hand side node
        astNode->child0 = createASTNode();
        //Indicate within the AST node where it is located in the stdin
        astNode->child0->line_counter = getLines();

        //Find the name in the symbol table
        astNode->child0->st_ref = getIDFromSymbolTable(symbolTable->root, functionName);
        if (astNode->child0->st_ref == NULL) {
            //If not in local table, search global table
            astNode->child0->st_ref = getIDFromSymbolTable(symbolTable->next->root, functionName);
        }
        //Assign node types for both the left hand side and current node
        astNode->ntype = ASSG;
        astNode->child0->ntype = IDENTIFIER;

        //We can continue to manage symbol table as normal
        fn_call_check = false;
        //<assg_stmt>
        astNode->child1 = assg_stmt();
    }
        //<fn_call> SEMI
    else if (token == LPAREN) {
        //Recognizes we should only search the global scope
        fn_call_check = true;
        //<fn_call>
        astNode = fn_call(functionName);

        //We can continue to manage symbol table as normal
        fn_call_check = false;
        //SEMI
        match(SEMI);
    } else {
        //Syntax error if it reached here
        match(ID);
    }
    return astNode;
}

//<assg_stmt> ::= opASSG <arith_exp> SEMI
ASTnode* assg_stmt() {
    ASTnode *astNode;
    //opASSG
    match(opASSG);
    //<arith_exp>
    astNode = arith_exp();
    //SEMI
    match(SEMI);
    return astNode;
}

//<fn_call> ::= LPAREN <opt_expr_list> RPAREN
ASTnode* fn_call(char* currentLexeme) {
    //Create and fill the FUNC_CALL AST node
    ASTnode *astNode = createASTNode();
    //Indicate within the AST node where it is located in the stdin
    astNode->line_counter = getLines();
    //Declare the AST node type
    astNode->ntype = FUNC_CALL;
    astNode->callee = getIDFromSymbolTable(symbolTable->next->root, currentLexeme);
    //LPAREN
    match(LPAREN);
    //<opt_expr_list>
    astNode->child0 = opt_expr_list();
    //RPAREN
    match(RPAREN);
    return astNode;
}

//<opt_expr_list> ::= E | <expr_list>
ASTnode* opt_expr_list() {
    //This indicates that the beginning of a param could be an ID, INTCON, UMINUS or LPAREN
    if (token == ID || token == INTCON || token == opSUB || token == LPAREN) {
        //<expr_list>
        return expr_list();
    } else {
        //E
        return NULL;
    }
}

//<expr_list> ::= <arith_exp> COMMA <expr_list> | <arith_exp>
ASTnode* expr_list() {
    //Create node for arg_list
    ASTnode *astNode = createASTNode();
    //Indicate within the AST node where it is located in the stdin
    astNode->line_counter = getLines();
    //Declare the AST node type
    astNode->ntype = EXPR_LIST;
    //<arith_exp>
    astNode->child0 = arith_exp();
    //COMMA <expr_list>
    if (token == COMMA) {
        match(COMMA);
        astNode->child1 = expr_list();
    }
    return astNode;
}

//<bool_exp> ::= <bool_exp1> <opt_or>
ASTnode* bool_exp() {
    ASTnode *left;
    left = bool_exp1();
    return opt_or(left);
}

//<opt_or> ::= || <bool_exp1> <opt_or> | E
ASTnode* opt_or(ASTnode* left) {
    ASTnode *right, *ast0;
    if (token == opOR) {
        match(opOR);
        right = bool_exp1();
        //Connect the given left and right children into a parent node, ast0
        ast0 = mk_node(OR, left, right);
        //Keep connecting node into ast0
        return opt_or(ast0);
    } else {
        //E
        return left;
    }
}

//<bool_exp1> ::= <bool_exp2> <opt_and>
ASTnode* bool_exp1() {
    ASTnode *left = bool_exp2();
    return opt_and(left);
}

//<opt_and> ::= && <bool_exp2> <opt_and> | E
ASTnode* opt_and(ASTnode* left) {
    ASTnode *right, *ast0;
    if (token == opAND) {
        match(opAND);
        right = bool_exp2();
        //Connect the given left and right children into a parent node, ast0
        ast0 = mk_node(AND, left, right);
        //Keep connecting node into ast0
        return opt_and(ast0);
    } else {
        //E
        return left;
    }
}

//<bool_exp2> ::= <arith_expr> <relop> <arith_exp>
ASTnode* bool_exp2() {
    ASTnode *astNode = createASTNode();
    //Indicate within the AST node where it is located in the stdin
    astNode->line_counter = getLines();
    //Create the parent node being the relop, and its children being the arithmetic expressions
    astNode->child0 = arith_exp();
    astNode->ntype = relop();
    astNode->child1 = arith_exp();
    return astNode;
}

//<arith_exp> ::= <arith_exp1> <opt_add_sub>
ASTnode* arith_exp() {
    ASTnode *left = arith_exp1();
    return opt_add_sub(left);
}

//<opt_add_sub> ::= opADD <arith_exp1> <opt_add_sub> | opSUB <arith_exp1> <opt_add_sub> | E
ASTnode* opt_add_sub(ASTnode* left) {
    ASTnode *right, *ast0;
    //opADD <arith_exp1> <opt_add_sub>
    if (token == opADD) {
        match(opADD);
        right = arith_exp1();
        //Connect the given left and right children into a parent node, ast0
        ast0 = mk_node(ADD, left, right);
        //Keep connecting node into ast0
        return opt_add_sub(ast0);
    }
        //opSUB <arith_exp1> <opt_add_sub>
    else if (token == opSUB) {
        match(opSUB);
        right = arith_exp1();
        //Connect the given left and right children into a parent node, ast0
        ast0 = mk_node(SUB, left, right);
        //Keep connecting node into ast0
        return opt_add_sub(ast0);
    } else {
        //E
        return left;
    }
}

//<arith_exp1> ::= <arith_exp2> <opt_mul_div>
ASTnode* arith_exp1() {
    ASTnode *left = arith_exp2();
    return opt_mul_div(left);
}

//<opt_mul_div> ::= opMUL <arith_exp2> <opt_mul_div> | opDIV <arith_exp2> <opt_mul_div> | E
ASTnode* opt_mul_div(ASTnode* left) {
    ASTnode *right, *ast0;
    //opMUL <arith_exp2> <opt_mul_div>
    if (token == opMUL) {
        match(opMUL);
        right = arith_exp2();
        //Connect the given left and right children into a parent node, ast0
        ast0 = mk_node(MUL, left, right);
        //Keep connecting node into ast0
        return opt_mul_div(ast0);
    }
        //opDIV <arith_exp2> <opt_mul_div>
    else if (token == opDIV) {
        match(opDIV);
        right = arith_exp2();
        //Connect the given left and right children into a parent node, ast0
        ast0 = mk_node(DIV, left, right);
        //Keep connecting node into ast0
        return opt_mul_div(ast0);
    } else {
        //E
        return left;
    }
}

//<arith_exp2> ::= opSUB <arith_exp2> | <arith_exp3>
ASTnode* arith_exp2() {
    ASTnode *astNode;
    //opSUB <arith_exp2>
    if (token == opSUB) {
        match(opSUB);
        astNode = createASTNode();
        //Indicate within the AST node where it is located in the stdin
        astNode->line_counter = getLines();

        //Declare the node type and recurse until we find the ID
        astNode->ntype = UMINUS;
        astNode->child0 = arith_exp2();
        return astNode;
    } else {
        //<arith_exp3>
        return arith_exp3();
    }
}

//<arith_exp3> ::= ID <opt_func_call> | INTCON | LPAREN <arith_exp> RPAREN
ASTnode* arith_exp3() {
    ASTnode *astNode = NULL;
    //ID <opt_func_call>
    if (token == ID) {
        //Enters first condition if we already allocated memory for the place saver for the ID
        //Enters second condition if it is the first time we will allocate memory to store a copy of current ID
        if (freeCopyLexeme) {
            copyLexeme = (char *) realloc(copyLexeme, bufferSize + 1);
            copyLexeme = strcpy(copyLexeme, lexeme);
        } else {
            copyLexeme = (char *) calloc(bufferSize + 1, sizeof(char));
            copyLexeme = strcpy(copyLexeme, lexeme);
            freeCopyLexeme = true;
        }

        //ID
        match(ID);

        //From here it will return a node if corresponding to IDENTIFIER or FUNC_CALL
        astNode = opt_func_call(copyLexeme);
    }
        //INTCON
    else if (token == INTCON) {
        astNode = createASTNode();
        //Indicate within the AST node where it is located in the stdin
        astNode->line_counter = getLines();

        //Declare node type and store the INTCON in the AST node
        astNode->ntype = INTCONST;
        astNode->num = atoi(lexeme);
        match(INTCON);
    }
        //LPAREN <arith_exp> RPAREN
    else if (token == LPAREN) {
        match(LPAREN);
        astNode = arith_exp();
        match(RPAREN);
    } else {
        //Syntax error if it gets here
        match(ID);
    }
    return astNode;
}

//<opt_func_call> ::= <fn_call>
ASTnode* opt_func_call(char* currentLexeme) {
    //<fn_call>
    if (token == LPAREN) {
        ASTnode *astNode;
        //Let fn_call create the node
        astNode = fn_call(currentLexeme);
        return astNode;
    } else {
        //ID
        ASTnode *astNode = createASTNode();
        //Indicate within the AST node where it is located in the stdin
        astNode->line_counter = getLines();
        astNode->ntype = IDENTIFIER;
        //Get the ID reference from the local symbol table
        astNode->st_ref = getIDFromSymbolTable(symbolTable->root, currentLexeme);
        if (astNode->st_ref == NULL) {
            //If not in the local symbol table, check the global symbol table
            astNode->st_ref = getIDFromSymbolTable(symbolTable->next->root, currentLexeme);
        }
        return astNode;
    }
}

//Makes an AST node connecting the left child and the right child to a parent node, which will return
ASTnode* mk_node(NodeType ntype, ASTnode *left, ASTnode *right) {
    ASTnode *node = createASTNode();
    //Indicate within the AST node where it is located in the stdin
    node->line_counter = getLines();
    node->ntype = ntype;
    node->child0 = left;
    node->child1 = right;
    return node;
}

//<relop> ::= opEQ | opNE | opLE | opLT | opGE | opGT
NodeType relop() {
    //opEQ
    if (token == opEQ) {
        match(opEQ);
        return EQ;
    }
    //opNE
    if (token == opNE) {
        match(opNE);
        return NE;
    }
    //opLE
    if (token == opLE) {
        match(opLE);
        return LE;
    }
    //opLT
    if (token == opLT) {
        match(opLT);
        return LT;
    }
    //opGE
    if (token == opGE) {
        match(opGE);
        return GE;
    } else {
        //opGT
        match(opGT);
        return GT;
    }
}

/////////////////////////////////////************************* MIPS GENERATION ************************/////////////////////////////////////


//Initializes three address code generation and MIPS code printing
void mipsDriver() {
    //Initialize/reset temp variable counter for naming
    temp_counter = 0;
    //Populates the three-address code structure
    traverseAST(ast->root);

    //printThreeAddressCodeNode(ast->root->codeHead,0);
    //printSymbolTable();

    //Function will print in stdout MIPS code after generating the three address instructions
    generateMIPSCode(ast->root->codeHead,functionNameCopy);

    //If flag is false, it means func_def never had a return, so hardcode one in
    if(!returnExists){
        fprintf(stdout,
                "li $v0, 0\n"
                "la $sp, 0($fp)\n"
                "lw $ra, 0($sp)\n"
                "lw $fp, 4($sp)\n"
                "la $sp, 8($sp)\n"
                "jr $ra\n");
    }
}

//Creates and populates a new ThreeAddressCodeNode instruction
ThreeAddressCodeNode* newinstr(InstrType opType, SymbolTableNode *src1, SymbolTableNode * src2, SymbolTableNode *dest) {
    ThreeAddressCodeNode *instr = createThreeAddressCodeNode();
    instr->op = opType;
    instr->src1 = src1;
    instr->src2 = src2;
    instr->dest = dest;
    return instr;
}

//Creates temp variable and stores it in the local symbol table
SymbolTableNode * newtemp(TYPE type) {
    //Creates temp variable
    char _temp[10];
    //Adds the temp concatenated with the number of current temps to the stream
    sprintf(_temp, "$temp%d", temp_counter);

    //Adds the newly made temp variable to the local symbol table
    populateSymbolTable(type, _temp, false);
    temp_counter++;

    //Get the node of the new temp variable to update its address
    SymbolTableNode *newNode = getScopeNode(symbolTable->root, _temp);
    newNode->address = currentAddress;
    //Update counters accordingly
    currentAddress -= 4;
    numOfOperations -= 4;

    //Return the node of the temp variable
    return newNode;
}

//Creates the Labels used in code generation
ThreeAddressCodeNode *newlabel() {
    //Creates temp variable
    char labelNum[10];
    //Adds the temp concatenated with the number of current temps to the stream
    sprintf(labelNum, "L%d", label_num);

    //Adds the new label into the symbol table
    populateSymbolTable(variable, labelNum, false);
    label_num++;

    //Pull the label from the symbol table, create the new instruction and return it
    SymbolTableNode *labelNumber = getScopeNode(symbolTable->root, labelNum);
    ThreeAddressCodeNode *label = newinstr(LABEL, labelNumber, NULL, NULL);
    return label;
}

//Traverses the AST and creates the three-address code instructions as we traverse
void traverseAST(ASTnode *astNode) {
    //Reached the end of the AST
    if (astNode == NULL) {
        return;
    }
    //Post-Order Traversal
    traverseAST(astNode->child0);
    traverseAST(astNode->child1);
    traverseAST(astNode->child2);

    //Will make three-address instructions depending on the type of operator we're dealing with
    codeGen_stmt(astNode);
}

//Generates three-address code instructions for the statements
void codeGen_stmt(ASTnode *node) {
    //Pointer used to get the func_def formal parameters
    ASTnode *ptr;

    //Labels used for three-address code
    ThreeAddressCodeNode *LThen;
    ThreeAddressCodeNode *LElse;
    ThreeAddressCodeNode *LAfter;
    ThreeAddressCodeNode *LTop;
    ThreeAddressCodeNode *LBody;

    //Labels stored within the symbol table
    SymbolTableNode *LThenLabel;
    SymbolTableNode *LElseLabel;
    SymbolTableNode *LAfterLabel;
    SymbolTableNode *LTopLabel;
    SymbolTableNode *LBodyLabel;

    switch (node->ntype) {
        case ASSG:
            if (node->child0 != NULL && node->child1 != NULL) {
                //Generate three-address code instructions for the LHS
                codeGen_expr(node->child0);

                //Generate three-address code instructions for the RHS
                codeGen_expr(node->child1);

                //Enters first condition if both children are variables
                //Enters second condition if LHS is a variable and RHS is an INTCONST
                if (node->child0->codeHead == NULL && node->child1->codeHead == NULL) {
                    //parent.head = newInstr()
                    node->codeHead = newinstr(ASSGN, node->child1->place, NULL, node->child0->place);
                } else {
                    //parentNode.head = child1.head
                    node->codeHead = node->child1->codeHead;

                    //child1.tail.next = newInstr()
                    (*node->child1->codeTail)->next = newinstr(ASSGN, node->child1->place, NULL, node->child0->place);

                    //parentNode.tail = child1.codeTail.next
                    node->codeTail = &((*node->child1->codeTail)->next);
                }
            }
            break;

        case STMT_LIST:
            //Enters first condition if both children are not NULL
            //Enters second condition if RHS is NULL
            if (node->child0 != NULL && node->child1 != NULL) {
                //parent.head = LHS.head
                node->codeHead = node->child0->codeHead;
                //LHS.tail.next = RHS.head
                (*node->child0->codeTail)->next = node->child1->codeHead;

                //parent.tail = RHS.tail
                node->codeTail = node->child1->codeTail;
            } else if (node->child0 != NULL) {
                //parent.head = LHS.head
                node->codeHead = node->child0->codeHead;
                //parent.head = LHS.Tail
                node->codeTail = node->child0->codeTail;
            }
            break;

        case FUNC_DEF:
            //Enters first condition if both children are not NULL
            //Enters second condition if RHS is NULL
            if (node->child1 != NULL) {
                ptr = node->child1;
                //Will set the formal parameters addresses from rightmost to leftmost
                setFormalParameterAddresses(ptr);
            }
            if (node->child0 != NULL) {
                //parent.head = LHS.head
                node->codeHead = node->child0->codeHead;
            }
            break;

        case FUNC_CALL:
            //Enters first condition if we have 1 or more parameters in func_call
            //Enters second condition if function call has 0 parameters
            if (node->child0 != NULL) {
                //Generates code for the parameters of the function call
                ///codeGen_expr(node->child0);
                //parent.head = child0.hd
                node->codeHead = node->child0->codeHead;

                //Gets the function call node from global scope symbol table
                node->place = getScopeNode(symbolTable->next->root, node->callee);
                //Makes the tail of child0->next another instruction
                (*node->child0->codeTail)->next = newinstr(CALL, node->place, node->place, NULL);

                //parent.tail = child0.tail.next
                node->codeTail = &((*node->child0->codeTail)->next);
            } else {
                //Gets the function call node from global scope symbol table
                node->place = getScopeNode(symbolTable->next->root, node->callee);
                node->codeHead = newinstr(CALL, node->place, node->place, NULL);
            }
            //Create a temp variable in which the return value will be stored at
            node->place = newtemp(variable);
            //Concat a new instruction to the three-address code list
            (*node->codeTail)->next = newinstr(RETRIEVE, NULL, NULL, node->place);
            //Connect the parent node to the new end of the three-address code list
            node->codeTail = &((*node->codeTail)->next);
            break;

        case EXPR_LIST:
            //Make sure the actual parameter is not NULL
            assert(node->child0 != NULL);

            //Generate code for the actual parameter
            codeGen_expr(node->child0);

            //Enters first condition if the func_call has only one parameter either left or at all
            //Enters second condition if the func_call has more than 1 parameter
            if (node->child1 == NULL) {
                //Enters first condition if the parameter is just a variable
                //Enters second condition if parameter is an INCONST or more three-address code
                if (node->child0->codeHead == NULL) {
                    //parent.head = newinstr(PARAM)
                    node->codeHead = newinstr(PARAM, node->child0->place, NULL, NULL);
                } else {
                    //parent.head = LHS.head
                    node->codeHead = node->child0->codeHead;
                    //LHS.next = newinstr(PARAM)
                    (*node->child0->codeTail)->next = newinstr(PARAM, node->child0->place, NULL, NULL);
                    //parent.next = LHS.next
                    node->codeTail = &((*node->child0->codeTail)->next);
                }
            } else {
                //Enters first condition if the parameter is just a variable
                //Enters second condition if parameter is an INCONST or more three-address code
                if (node->child0->codeHead == NULL) {
                    //parent.head = RHS.head
                    node->codeHead = node->child1->codeHead;
                    //RHS.next = newinstr(PARAM)
                    (*node->child1->codeTail)->next = newinstr(PARAM, node->child0->place, NULL, NULL);
                    //parent.next = RHS.next
                    node->codeTail = &(*node->child1->codeTail)->next;
                } else {
                    //parent.head = LHS.head
                    node->codeHead = node->child0->codeHead;
                    //LHS.next = RHS.head
                    (*node->child0->codeTail)->next = node->child1->codeHead;
                    //RHS.next = newinstr(PARAM)
                    (*node->child1->codeTail)->next = newinstr(PARAM, node->child0->place, NULL, NULL);
                    //parent.next = RHS.next
                    node->codeTail = &(*node->child1->codeTail)->next;
                }

            }
            break;

        case IF:
            //Create the Three address code labels
            LThen = newlabel();
            LElse = newlabel();
            LAfter = newlabel();

            //Since they were stored in the symbol table, get them from the symbol table
            LThenLabel = getScopeNode(symbolTable->root, LThen->src1->name);
            LElseLabel = getScopeNode(symbolTable->root, LElse->src1->name);
            LAfterLabel = getScopeNode(symbolTable->root, LAfter->src1->name);

            //Make the three address code instruction for the boolean operation of the IF statement
            codeGen_Bool(node->child0, LThenLabel, LElseLabel);

            //Avoids case when empty or only ;
            if (node->child1 != NULL) {
                //Generate the three address instructions for the body of the THEN section of the IF statement
                codeGen_stmt(node->child1);
            }

            //Avoids case when empty or only ;
            if (node->child2 != NULL) {
                //Generate the three address instructions for the body of the ELSE section of the IF statement
                codeGen_stmt(node->child2);
            }

            //Makes sure the bool expression is not NULL
            assert(node->child0 != NULL);
            //parent.head = booleanNode.head
            node->codeHead = node->child0->codeHead;

            //Enters first condition if we have an if statement with a THEN statement
            //Enters second condition if we do not have a THEN statement, but we do have an ELSE
            //Enters third condition if we only have a boolean expression for the conditional
            if (node->child1 != NULL) {
                //booleanNode.tail.next = L_then
                (*node->child0->codeTail)->next = LThen;
                //L_then.next = S1.head
                LThen->next = node->child1->codeHead;
                //S1.tail.next = newInstr()
                ThreeAddressCodeNode *tempInstr = newinstr(GOTO, NULL, NULL, LAfterLabel);
                (*node->child1->codeTail)->next = tempInstr;

                //Enters first condition if we have an ELSE stmt
                //Enters second condition if we have a dangling IF
                if (node->child2 != NULL) {
                    //parent.tail.next = L_else
                    tempInstr->next = LElse;
                    //L_else.next = S2.head
                    LElse->next = node->child2->codeHead;
                    //S2.tail.next = L_after
                    (*node->child2->codeTail)->next = LAfter;
                    //Make the tail of the parent node point to the end of this list of instructions for STMT to get
                    node->codeTail = &((*node->child2->codeTail)->next);
                } else {
                    //parent.tail.next = L_Else
                    tempInstr->next = LElse;
                    //LElse.next = LAfter
                    LElse->next = LAfter;
                    //Make the tail of the parent node point to the end of this list of instructions for STMT to get
                    node->codeTail = &(LElse->next);
                }
            } else if (node->child2 != NULL) {
                //Makes sure the bool expression is not NULL
                assert(node->child0 != NULL);
                //booleanNode.tail.next = L_then
                (*node->child0->codeTail)->next->next = LThen;
                //LThen.next = LElse
                LThen->next = LElse;
                //LElse.next = S2.head
                LElse->next = node->child2->codeHead;
                //S1.tail.next = newInstr()
                ThreeAddressCodeNode *tempInstr = newinstr(GOTO, NULL, NULL, LAfterLabel);
                (*node->child2->codeTail)->next = tempInstr;
                //S2.tail.next = L_after
                tempInstr->next = LAfter;
                //Make the tail of the parent node point to the end of this list of instructions for STMT to get
                node->codeTail = &(tempInstr->next);
            } else {
                //Makes sure the bool expression is not NULL
                assert(node->child0 != NULL);
                //booleanNode.tail.next = L_then
                (*node->child0->codeTail)->next = LThen;
                //LThen.next = LElse
                LThen->next = LElse;
                //LElse.next = GOTO
                ThreeAddressCodeNode *tempInstr = newinstr(GOTO, NULL, NULL, LAfterLabel);
                LElse->next = tempInstr;
                //GOTO.next = LAfter
                tempInstr->next = LAfter;
                //Make the tail of the parent node point to the end of this list of instructions for STMT to get
                node->codeTail = &(tempInstr->next);
            }
            break;

        case WHILE:
            //Create the Three address code labels
            LTop = newlabel();
            LBody = newlabel();
            LAfter = newlabel();

            //Since they were stored in the symbol table, get them from the symbol table
            LTopLabel = getScopeNode(symbolTable->root, LTop->src1->name);
            LBodyLabel = getScopeNode(symbolTable->root, LBody->src1->name);
            LAfterLabel = getScopeNode(symbolTable->root, LAfter->src1->name);

            //Make the three address code instruction for the boolean operation of the WHILE
            codeGen_Bool(node->child0, LBodyLabel, LAfterLabel);

            //Avoids case when empty or only ;
            if (node->child1 != NULL) {
                //Generate the three address instructions for the body of the body section of the WHILE loop
                codeGen_stmt(node->child1);
            }

            //Enters first condition if while loop has a boolean expression and a body
            //Enters second condition if while loop has a boolean but not a body
            if (node->child0 != NULL && node->child1 != NULL) {
                //parent.head = LTop
                node->codeHead = LTop;
                //LTop.next = child0.head
                LTop->next = node->child0->codeHead;
                //child0.tail.next = LBody
                (*node->child0->codeTail)->next = LBody;
                //LBody.next = child1.head
                LBody->next = node->child1->codeHead;
                //child1.tail.next = newInstr(GOTO)
                ThreeAddressCodeNode *tempStmt = newinstr(GOTO, NULL, NULL, LTopLabel);
                (*node->child1->codeTail)->next = tempStmt;
                //newInstr(GOTO)->next = LAfter
                tempStmt->next = LAfter;
                //Make the tail of the parent node point to the end of this list of instructions for STMT to get
                node->codeTail = &((tempStmt)->next);
            } else {
                //Makes sure the bool expression is not NULL
                assert(node->child0 != NULL);
                //parent.head = LTop
                node->codeHead = LTop;
                //LTop.next = child0.head
                LTop->next = node->child0->codeHead;
                //child0.tail.next = LBody
                (*node->child0->codeTail)->next = LBody;
                //LBody.next = child1.head
                LBody->next = LAfter;
                //Make the tail of the parent node point to the end of this list of instructions for STMT to get
                node->codeTail = &(LBody->next);
            }
            break;

        case RETURN:
            //Enters first condition if the return statement is empty
            //Enters second condition if return statement is only one variable
            //Enters third condition if return statement returns an arith_expr
            if (node->child0 == NULL) {
                //Just create a return instruction that returns 0 by default
                node->codeHead = newinstr(RET, NULL, NULL, NULL);
            } else if(node->child0->ntype == IDENTIFIER) {
                codeGen_expr(node->child0);
                //Just create a return instruction that returns the variable's place
                node->codeHead = newinstr(RET,NULL,NULL,node->child0->place);
            } else {
                //Generate code for the return expression
                codeGen_expr(node->child0);
                node->codeHead = node->child0->codeHead;
                (*node->child0->codeTail)->next = newinstr(RET, NULL, NULL, node->child0->place);
                node->codeTail = &((*node->child0->codeTail)->next);
            }
            returnExists = true;
            break;

        default:
            break;
    }
}

//Creates code for function call expression list
void codeGen_expr(ASTnode *node) {
    switch (node->ntype) {
        case IDENTIFIER:
            //Gets a pointer to the local symbol table's entry
            node->place = getScopeNode(symbolTable->root, node->st_ref);
            if (node->place == NULL) {
                //If it isn't in the local symbol table, check the global
                node->place = getScopeNode(symbolTable->next->root, node->st_ref);
            }
            //Since we're dealing with an identifier, we can already reference it in the symbol table, so no need for code gen
            node->codeHead = NULL;
            break;

        case INTCONST:
            //Creates new temp variable in the symbol tree and stores the name in the place field
            node->place = newtemp(variable);
            //Within the temp variable within place, update the INTCONST field to the current one
            node->place->num = node->num;
            *node->codeTail = newinstr(ASSGN, node->place, NULL, node->place);
            break;

        case UMINUS:
            //Makes sure the node is not NULL
            assert(node->child0 != NULL);

            //Recurse until we find the ID or INTCONST
            codeGen_expr(node->child0);

            //Create new temp variable to store the negation in
            node->place = newtemp(variable);

            //Enters first condition if LHS is a variable
            //Enters second condition of LHS is an INTCONST
            if (node->child0->codeHead == NULL) {
                //Connect everything to the parent.head
                node->codeHead = newinstr(UNARY_MINUS, node->child0->place, NULL, node->place);
            } else {
                //Connect parent.head to $temp variable
                node->codeHead = node->child0->codeHead;
                //$temp.next = newinstr(UNARY_MINUS)
                (*node->child0->codeTail)->next = newinstr(UNARY_MINUS, node->child0->place, NULL, node->place);
                //Connect the parent node to end of the new three-address code
                node->codeTail = &((*node->child0->codeTail)->next);
            }
            break;

        case ADD:
            //Makes sure the children are not NULL to avoid errors
            assert(node->child0 != NULL);
            assert(node->child1 != NULL);

            //Generate code for the LHS and RHS
            codeGen_expr(node->child0);
            codeGen_expr(node->child1);

            //Generate temp variable that will store the addition of the LHS and RHS
            node->place = newtemp(variable);

            //Enters first condition if both children are variables
            //Enters second condition if only the RHS is a variable
            //Enters third condition if only the LHS is a variable
            //Enters fourth condition if both LHS and RHS are INTCONST
            if (node->child0->codeHead == NULL && node->child1->codeHead == NULL) {
                //Since both LHS and RHS are variables, just connect them and return
                node->codeHead = newinstr(PLUS, node->child0->place, node->child1->place, node->place);
            } else if (node->child1->codeHead == NULL) {
                //Connect the parent.head to the LHS.head
                node->codeHead = node->child0->codeHead;
                //LHS.next = newinstr(PLUS)
                (*node->child0->codeTail)->next = newinstr(PLUS, node->child0->place, node->child1->place, node->place);
                //Connect the parent tail with the LHS tail
                node->codeTail = &((*node->child0->codeTail)->next);
            } else if (node->child0->codeHead == NULL) {
                //Connect the parent.head to the RHS.head
                node->codeHead = node->child1->codeHead;
                //RHS.next = newinstr(PLUS)
                (*node->child1->codeTail)->next = newinstr(PLUS, node->child0->place, node->child1->place, node->place);
                //Connect the parent tail with the RHS tail
                node->codeTail = &((*node->child1->codeTail)->next);
            } else {
                //Connect the parent.head to the LHS.head
                node->codeHead = node->child0->codeHead;
                //LHS.next = RHS.head
                (*node->child0->codeTail)->next = node->child1->codeHead;
                //RHS.next = newinstr(PLUS)
                (*node->child1->codeTail)->next = newinstr(PLUS, node->child0->place, node->child1->place, node->place);
                //Connect the parent tail with the RHS tail
                node->codeTail = &((*node->child1->codeTail)->next);
            }
            break;

        case SUB:
            //Makes sure the children are not NULL to avoid errors
            assert(node->child0 != NULL);
            assert(node->child1 != NULL);

            //Generate code for the LHS and RHS
            codeGen_expr(node->child0);
            codeGen_expr(node->child1);

            //Generate temp variable that will store the difference of the LHS and RHS
            node->place = newtemp(variable);

            //Enters first condition if both children are variables
            //Enters second condition if only the RHS is a variable
            //Enters third condition if only the LHS is a variable
            //Enters fourth condition if both LHS and RHS are INTCONST
            if (node->child0->codeHead == NULL && node->child1->codeHead == NULL) {
                //Since both LHS and RHS are variables, just connect them and return
                node->codeHead = newinstr(MINUS, node->child0->place, node->child1->place, node->place);
            } else if (node->child1->codeHead == NULL) {
                //Connect the parent.head to the LHS.head
                node->codeHead = node->child0->codeHead;
                //LHS.next = newinstr(PLUS)
                (*node->child0->codeTail)->next = newinstr(MINUS, node->child0->place, node->child1->place,
                                                           node->place);
                //Connect the parent tail with the LHS tail
                node->codeTail = &((*node->child0->codeTail)->next);
            } else if (node->child0->codeHead == NULL) {
                //Connect the parent.head to the RHS.head
                node->codeHead = node->child1->codeHead;
                //RHS.next = newinstr(PLUS)
                (*node->child1->codeTail)->next = newinstr(MINUS, node->child0->place, node->child1->place,
                                                           node->place);
                //Connect the parent tail with the RHS tail
                node->codeTail = &((*node->child1->codeTail)->next);
            } else {
                //Connect the parent.head to the LHS.head
                node->codeHead = node->child0->codeHead;
                //LHS.next = RHS.head
                (*node->child0->codeTail)->next = node->child1->codeHead;
                //RHS.next = newinstr(PLUS)
                (*node->child1->codeTail)->next = newinstr(MINUS, node->child0->place, node->child1->place,
                                                           node->place);
                //Connect the parent tail with the RHS tail
                node->codeTail = &((*node->child1->codeTail)->next);
            }
            break;

        case MUL:
            //Makes sure the children are not NULL to avoid errors
            assert(node->child0 != NULL);
            assert(node->child1 != NULL);

            //Generate code for the LHS and RHS
            codeGen_expr(node->child0);
            codeGen_expr(node->child1);

            //Generate temp variable that will store the product of the LHS and RHS
            node->place = newtemp(variable);

            //Enters first condition if both children are variables
            //Enters second condition if only the RHS is a variable
            //Enters third condition if only the LHS is a variable
            //Enters fourth condition if both LHS and RHS are INTCONST
            if (node->child0->codeHead == NULL && node->child1->codeHead == NULL) {
                //Since both LHS and RHS are variables, just connect them and return
                node->codeHead = newinstr(MULTIPLY, node->child0->place, node->child1->place, node->place);
            } else if (node->child1->codeHead == NULL) {
                //Connect the parent.head to the LHS.head
                node->codeHead = node->child0->codeHead;
                //LHS.next = newinstr(PLUS)
                (*node->child0->codeTail)->next = newinstr(MULTIPLY, node->child0->place, node->child1->place,
                                                           node->place);
                //Connect the parent tail with the LHS tail
                node->codeTail = &((*node->child0->codeTail)->next);
            } else if (node->child0->codeHead == NULL) {
                //Connect the parent.head to the RHS.head
                node->codeHead = node->child1->codeHead;
                //RHS.next = newinstr(PLUS)
                (*node->child1->codeTail)->next = newinstr(MULTIPLY, node->child0->place, node->child1->place,
                                                           node->place);
                //Connect the parent tail with the RHS tail
                node->codeTail = &((*node->child1->codeTail)->next);
            } else {
                //Connect the parent.head to the LHS.head
                node->codeHead = node->child0->codeHead;
                //LHS.next = RHS.head
                (*node->child0->codeTail)->next = node->child1->codeHead;
                //RHS.next = newinstr(PLUS)
                (*node->child1->codeTail)->next = newinstr(MULTIPLY, node->child0->place, node->child1->place,
                                                           node->place);
                //Connect the parent tail with the RHS tail
                node->codeTail = &((*node->child1->codeTail)->next);
            }
            break;

        case DIV:
            //Makes sure the children are not NULL to avoid errors
            assert(node->child0 != NULL);
            assert(node->child1 != NULL);

            //Generate code for the LHS and RHS
            codeGen_expr(node->child0);
            codeGen_expr(node->child1);

            //Generate temp variable that will store the division of the LHS and RHS
            node->place = newtemp(variable);

            //Enters first condition if both children are variables
            //Enters second condition if only the RHS is a variable
            //Enters third condition if only the LHS is a variable
            //Enters fourth condition if both LHS and RHS are INTCONST
            if (node->child0->codeHead == NULL && node->child1->codeHead == NULL) {
                //Since both LHS and RHS are variables, just connect them and return
                node->codeHead = newinstr(DIVIDE, node->child0->place, node->child1->place, node->place);
            } else if (node->child1->codeHead == NULL) {
                //Connect the parent.head to the LHS.head
                node->codeHead = node->child0->codeHead;
                //LHS.next = newinstr(PLUS)
                (*node->child0->codeTail)->next = newinstr(DIVIDE, node->child0->place, node->child1->place,
                                                           node->place);
                //Connect the parent tail with the LHS tail
                node->codeTail = &((*node->child0->codeTail)->next);
            } else if (node->child0->codeHead == NULL) {
                //Connect the parent.head to the RHS.head
                node->codeHead = node->child1->codeHead;
                //RHS.next = newinstr(PLUS)
                (*node->child1->codeTail)->next = newinstr(DIVIDE, node->child0->place, node->child1->place,
                                                           node->place);
                //Connect the parent tail with the RHS tail
                node->codeTail = &((*node->child1->codeTail)->next);
            } else {
                //Connect the parent.head to the LHS.head
                node->codeHead = node->child0->codeHead;
                //LHS.next = RHS.head
                (*node->child0->codeTail)->next = node->child1->codeHead;
                //RHS.next = newinstr(PLUS)
                (*node->child1->codeTail)->next = newinstr(DIVIDE, node->child0->place, node->child1->place,
                                                           node->place);
                //Connect the parent tail with the RHS tail
                node->codeTail = &((*node->child1->codeTail)->next);
            }
            break;

        default:
            break;
    }
}

//Generates code for the boolean expressions
void codeGen_Bool(ASTnode* node, SymbolTableNode *trueDest, SymbolTableNode* falseDest) {
    //Makes sure we actually have an LHS and RHS
    assert(node->child0 != NULL);
    assert(node->child1 != NULL);

    //LHS code generation
    codeGen_expr(node->child0);

    //RHS code generation
    codeGen_expr(node->child1);
    ThreeAddressCodeNode *L1;
    SymbolTableNode *L1Label;
    switch (node->ntype) {
        case AND:
            //Make sure that the LHS and RHS are not NULL to avoid errors
            assert(node->child0 != NULL);
            assert(node->child1 != NULL);

            //Create new label and get it from the symbol table
            L1 = newlabel();
            L1Label = getScopeNode(symbolTable->root, L1->src1->name);

            //Generate code for the LHS and RHS of the && using short circuit evaluation
            codeGen_Bool(node->child0, L1Label, falseDest);
            codeGen_Bool(node->child1, trueDest, falseDest);

            //Connect the parent.head to the LHS.head
            node->codeHead = node->child0->codeHead;
            //LHS.next = L1
            (*node->child0->codeTail)->next = L1;
            //L1.next = RHS.head
            L1->next = node->child1->codeHead;
            //Connect the parent tail with the RHS tail
            node->codeTail = (node->child1->codeTail);
            break;

        case OR:
            //Make sure that the LHS and RHS are not NULL to avoid errors
            assert(node->child0 != NULL);
            assert(node->child1 != NULL);

            //Create new label and get it from the symbol table
            L1 = newlabel();
            L1Label = getScopeNode(symbolTable->root, L1->src1->name);

            //Generate code for the LHS and RHS of the || using short circuit evaluation
            codeGen_Bool(node->child0, trueDest, L1Label);
            codeGen_Bool(node->child1, trueDest, falseDest);

            //Connect the parent.head to the LHS.head
            node->codeHead = node->child0->codeHead;
            //LHS.next = L1
            (*node->child0->codeTail)->next = L1;
            //L1.next = RHS.head
            L1->next = node->child1->codeHead;
            //Connect the parent tail with the RHS tail
            node->codeTail = (node->child1->codeTail);
            break;

        default:
            //Enters first condition if both children are variables
            //Enters second condition if LHS is a variable and RHS is an INTCONST or both are INTCONSTS
            if (node->child0->codeHead == NULL && node->child1->codeHead == NULL) {
                if (node->ntype == EQ) {
                    //Enters if relop is ==
                    node->codeHead = newinstr(IF_EQ, node->child0->place, node->child1->place, trueDest);
                } else if (node->ntype == NE) {
                    //Enters if relop is !=
                    node->codeHead = newinstr(IF_NE, node->child0->place, node->child1->place, trueDest);
                } else if (node->ntype == LE) {
                    //Enters if relop is <=
                    node->codeHead = newinstr(IF_LE, node->child0->place, node->child1->place, trueDest);
                } else if (node->ntype == LT) {
                    //Enters if relop is <
                    node->codeHead = newinstr(IF_LT, node->child0->place, node->child1->place, trueDest);
                } else if (node->ntype == GE) {
                    //Enters if relop is >=
                    node->codeHead = newinstr(IF_GE, node->child0->place, node->child1->place, trueDest);
                } else {
                    //Enters if relop is >
                    node->codeHead = newinstr(IF_GT, node->child0->place, node->child1->place, trueDest);
                }
            } else {
                //Enters firtst condition if LHS and RHS are INTCONSTS to connect them properly
                //Enters second condition if only the RHS is an INTCONST
                if (node->child0->codeHead != NULL && node->child1->codeHead != NULL) {
                    //parentNode.head = child0.head
                    node->codeHead = node->child0->codeHead;
                    //child0.tail.next = child1.head
                    (*node->child0->codeTail)->next = node->child1->codeHead;
                } else {
                    //parentNode.head = child1.head
                    node->codeHead = node->child1->codeHead;
                }

                //Enters each condition according to the current relop type
                if (node->ntype == EQ) {
                    //Enters if relop is ==
                    (*node->child1->codeTail)->next = newinstr(IF_EQ, node->child0->place, node->child1->place,
                                                               trueDest);
                } else if (node->ntype == NE) {
                    //Enters if relop is !=
                    (*node->child1->codeTail)->next = newinstr(IF_NE, node->child0->place, node->child1->place,
                                                               trueDest);
                } else if (node->ntype == LE) {
                    //Enters if relop is <=
                    (*node->child1->codeTail)->next = newinstr(IF_LE, node->child0->place, node->child1->place,
                                                               trueDest);
                } else if (node->ntype == LT) {
                    //Enters if relop is <
                    (*node->child1->codeTail)->next = newinstr(IF_LT, node->child0->place, node->child1->place,
                                                               trueDest);
                } else if (node->ntype == GE) {
                    //Enters if relop is >=
                    (*node->child1->codeTail)->next = newinstr(IF_GE, node->child0->place, node->child1->place,
                                                               trueDest);
                } else {
                    //Enters if relop is >
                    (*node->child1->codeTail)->next = newinstr(IF_GT, node->child0->place, node->child1->place,
                                                               trueDest);
                }
                //parentNode.tail = child1.codeTail.next
                node->codeTail = &((*node->child1->codeTail)->next);
            }
            //code.tail.next = newInstr()
            (*node->codeTail)->next = newinstr(GOTO, NULL, NULL, falseDest);
            node->codeTail = &((*node->codeTail)->next);
            break;
    }
}


//Sets the address for the parameters from leftmost to rightmost parameters
void setFormalParameterAddresses(ASTnode *node) {
    if (node == NULL) {
        return;
    }

    //Get the formal parameter variable from local symbol table
    SymbolTableNode *tempParam = getScopeNode(symbolTable->root, node->st_ref);
    //Update the address
    tempParam->address = paramAddress;
    paramAddress += 4;

    setFormalParameterAddresses(node->child0);
}

//Prints code for the hardcoded println and main functions in MIPS
void printHardcodedFunctions() {
    fprintf(stdout, "\n.align 2\n"
                    ".data\n"
                    "_nl: .asciiz \"\\n\"\n"
                    "\n.align 2\n"
                    ".text\n"
                    "# println: print out an integer followed by a newline\n"
                    "_println:\n"
                    "li $v0, 1\n"
                    "lw $a0, 0($sp)\n"
                    "syscall\n"
                    "li $v0, 4\n"
                    "la $a0, _nl\n"
                    "syscall\n"
                    "jr $ra\n\n");
    fprintf(stdout, "# dumping global symbol table into mips\n"
                    ".data\n"
                    "\n"
                    "# hard coded main call\n"
                    ".align 2\n"
                    ".text\n"
                    "main: j _main\n");
}

//Function will generate MIPS assembly code from the three-address instruction list
void generateMIPSCode(ThreeAddressCodeNode* head, char* funcNameMIPS) {
    //Create pointer to traverse the three address code instructions list
    ThreeAddressCodeNode *ptr;
    SymbolTableNode *tempScopeNodeSrc;
    SymbolTableNode *tempScopeNodeDest;

    //Generates the FUNC_DEF MIPS CODE
    fprintf(stdout, "\n# Entering %s function\n"
                    ".text\n_%s:\n"
                    "la $sp, -8($sp)\n"
                    "sw $fp, 4($sp)\n"
                    "sw $ra, 0($sp)\n"
                    "la $fp, 0($sp)\n"
                    "la $sp, %d($sp)\n\n", funcNameMIPS, funcNameMIPS, numOfOperations);

    //Set the pointer to be the head of the three-address code instructions
    ptr = head;
    //Loop through the linked list of three-address code instructions and generate MIPS code
    while (ptr != NULL) {
        switch (ptr->op) {
            case ASSGN:
                fprintf(stdout, "# ASSGN\n");
                //When assigning, make sure to know if we're dealing with an INTCONST and the src
                if (ptr->dest->num != INT_MIN) {
                    //$temp variables are only in local scope, no need to check in global
                    tempScopeNodeSrc = getScopeNode(symbolTable->root, ptr->src1->name);
                    //Generate code for the INTCONST assign to the $temp variable
                    fprintf(stdout, "li $t0, %d\n"
                                    "sw $t0, %d($fp)\n\n",
                            ptr->src1->num,
                            tempScopeNodeSrc->address);
                } else {
                    //Check the local scope first
                    tempScopeNodeSrc = getScopeNode(symbolTable->root, ptr->src1->name);
                    if (tempScopeNodeSrc == NULL) {
                        //If not in local, get it from global
                        tempScopeNodeSrc = getScopeNode(symbolTable->next->root, ptr->src1->name);

                        //Generate code for the global variable as the first part of the ASSGN in MIPS
                        fprintf(stdout, "lw $t0, _%s\n", tempScopeNodeSrc->name);
                    } else {
                        //Generate code for the local variable as the first part of the ASSGN in MIPS
                        fprintf(stdout, "lw $t0, %d($fp)\n", tempScopeNodeSrc->address);
                    }

                    //Check local scope for the destination variable
                    tempScopeNodeDest = getScopeNode(symbolTable->root, ptr->dest->name);
                    if (tempScopeNodeDest == NULL) {
                        //If not in local, get it from global and adjust code generation
                        tempScopeNodeDest = getScopeNode(symbolTable->next->root, ptr->dest->name);

                        //Generate code for the global variable as the second part of the ASSGN in MIPS
                        fprintf(stdout, "sw $t0, _%s\n\n", tempScopeNodeDest->name);
                    } else {
                        //Generate code for the local variable as the second part of the ASSGN in MIPS
                        fprintf(stdout, "sw $t0, %d($fp)\n\n", tempScopeNodeDest->address);
                    }
                }
                break;

            case PARAM:
                fprintf(stdout, "# PARAM\n");
                //Check the local scope first
                tempScopeNodeSrc = getScopeNode(symbolTable->root, ptr->src1->name);
                if (tempScopeNodeSrc == NULL) {
                    //check global scope
                    tempScopeNodeSrc = getScopeNode(symbolTable->next->root, ptr->src1->name);
                    //Generate code for the global variable as the first part of the FUNC_CALL in MIPS
                    fprintf(stdout, "lw $t0 _%s\n", tempScopeNodeSrc->name);
                } else {
                    //Generate code for the global variable as the first part of the FUNC_CALL in MIPS
                    fprintf(stdout, "lw $t0 %d($fp)\n", tempScopeNodeSrc->address);
                }
                //If we're dealing with an INTCONST PARAM, generate code to store it in its $temp variable
                if (ptr->src1->num != INT_MIN) {
                    fprintf(stdout, "li $t0, %d\n"
                                    "sw $t0, %d($fp)\n", ptr->src1->num, tempScopeNodeSrc->address);
                }
                //Generate code to store the parameters to the stack and move the stack pointer to the next available space
                fprintf(stdout, "la $sp -4($sp)\n");
                fprintf(stdout, "sw $t0 0($sp)\n");

                //Increment counter used in the CALL case to allocate memory accordingly
                paramOperations += 4;
                break;

            case RETRIEVE:
                fprintf(stdout, "# RETRIEVE\n");
                //Check local scope for the destination variable since it is always going to be a temp variable
                tempScopeNodeDest = getScopeNode(symbolTable->root, ptr->dest->name);

                //Generate code to store the return value to that temp variable
                fprintf(stdout, "sw $v0, %d($fp)\n\n", tempScopeNodeDest->address);
                break;

            case CALL:
                //Print out the MIPS code for the call instruction
                fprintf(stdout, "# CALL\njal _%s\n"
                                "la $sp, %d($sp)\n\n",
                        ptr->src1->name,
                        paramOperations);

                //Reset counter for the next FUNC_CALL
                paramOperations = 0;
                break;

            case GOTO:
                fprintf(stdout, "# GOTO \nj _%s\n", ptr->dest->name);
                break;

            case LABEL:
                fprintf(stdout, "# Label\n_%s:\n", ptr->src1->name);
                break;

            case IF_EQ:
                fprintf(stdout, "# Conditional IF_EQ\n");
                //Confirm if src1 is a global variable or not
                if (ptr->src1->scope) {
                    fprintf(stdout, "lw $t0, _%s\n", ptr->src1->name);
                } else {
                    fprintf(stdout, "lw $t0, %d($fp)\n", ptr->src1->address);
                }

                //Confirm if src2 is a global variable or not
                if (ptr->src2->scope) {
                    fprintf(stdout, "lw $t1, _%s\n", ptr->src2->name);
                } else {
                    fprintf(stdout, "lw $t1, %d($fp)\n", ptr->src2->address);
                }

                //Print the MIPS condition
                fprintf(stdout, "beq $t0, $t1, _%s\n", ptr->dest->name);
                break;

            case IF_NE:
                fprintf(stdout, "# Conditional IF_NE\n");
                //Confirm if src1 is a global variable or not
                if (ptr->src1->scope) {
                    fprintf(stdout, "lw $t0, _%s\n", ptr->src1->name);
                } else {
                    fprintf(stdout, "lw $t0, %d($fp)\n", ptr->src1->address);
                }

                //Confirm if src2 is a global variable or not
                if (ptr->src2->scope) {
                    fprintf(stdout, "lw $t1, _%s\n", ptr->src2->name);
                } else {
                    fprintf(stdout, "lw $t1, %d($fp)\n", ptr->src2->address);
                }

                //Print the MIPS condition
                fprintf(stdout, "bne $t0, $t1, _%s\n", ptr->dest->name);
                break;

            case IF_LE:
                fprintf(stdout, "# Conditional IF_LE\n");
                //Confirm if src1 is a global variable or not
                if (ptr->src1->scope) {
                    fprintf(stdout, "lw $t0, _%s\n", ptr->src1->name);
                } else {
                    fprintf(stdout, "lw $t0, %d($fp)\n", ptr->src1->address);
                }

                //Confirm if src2 is a global variable or not
                if (ptr->src2->scope) {
                    fprintf(stdout, "lw $t1, _%s\n", ptr->src2->name);
                } else {
                    fprintf(stdout, "lw $t1, %d($fp)\n", ptr->src2->address);
                }

                //Print the MIPS condition
                fprintf(stdout, "ble $t0, $t1, _%s\n", ptr->dest->name);
                break;

            case IF_LT:
                fprintf(stdout, "# Conditional IF_LT\n");
                //Confirm if src1 is a global variable or not
                if (ptr->src1->scope) {
                    fprintf(stdout, "lw $t0, _%s\n", ptr->src1->name);
                } else {
                    fprintf(stdout, "lw $t0, %d($fp)\n", ptr->src1->address);
                }

                //Confirm if src2 is a global variable or not
                if (ptr->src2->scope) {
                    fprintf(stdout, "lw $t1, _%s\n", ptr->src2->name);
                } else {
                    fprintf(stdout, "lw $t1, %d($fp)\n", ptr->src2->address);
                }

                //Print the MIPS condition
                fprintf(stdout, "blt $t0, $t1, _%s\n", ptr->dest->name);
                break;

            case IF_GE:
                fprintf(stdout, "# Conditional IF_GE\n");
                //Confirm if src1 is a global variable or not
                if (ptr->src1->scope) {
                    fprintf(stdout, "lw $t0, _%s\n", ptr->src1->name);
                } else {
                    fprintf(stdout, "lw $t0, %d($fp)\n", ptr->src1->address);
                }

                //Confirm if src2 is a global variable or not
                if (ptr->src2->scope) {
                    fprintf(stdout, "lw $t1, _%s\n", ptr->src2->name);
                } else {
                    fprintf(stdout, "lw $t1, %d($fp)\n", ptr->src2->address);
                }

                //Print the MIPS condition
                fprintf(stdout, "bge $t0, $t1, _%s\n", ptr->dest->name);
                break;

            case IF_GT:
                fprintf(stdout, "# Conditional IF_GT\n");
                //Confirm if src1 is a global variable or not
                if (ptr->src1->scope) {
                    fprintf(stdout, "lw $t0, _%s\n", ptr->src1->name);
                } else {
                    fprintf(stdout, "lw $t0, %d($fp)\n", ptr->src1->address);
                }

                //Confirm if src2 is a global variable or not
                if (ptr->src2->scope) {
                    fprintf(stdout, "lw $t1, _%s\n", ptr->src2->name);
                } else {
                    fprintf(stdout, "lw $t1, %d($fp)\n", ptr->src2->address);
                }

                //Print the MIPS condition
                fprintf(stdout, "bgt $t0, $t1, _%s\n", ptr->dest->name);
                break;

            case UNARY_MINUS:
                fprintf(stdout, "# UMINUS Operation\n");
                //When assigning, make sure to know if we're dealing with an INTCONST in the src
                if (ptr->dest->num != INT_MIN) {
                    //$temp variables are only in local scope, no need to check in global
                    tempScopeNodeSrc = getScopeNode(symbolTable->root, ptr->src1->name);
                    //Generate code for the negation of the INTCONST assign to the $temp variable
                    fprintf(stdout, "li $t0, %d\n"
                                    "neg $t1, $t0\n"
                                    "sw $t1, %d($fp)\n\n",
                            ptr->src1->num,
                            tempScopeNodeSrc->address);
                } else {
                    //Check the local scope first
                    tempScopeNodeSrc = getScopeNode(symbolTable->root, ptr->src1->name);
                    if (tempScopeNodeSrc == NULL) {
                        //If not in local, get it from global
                        tempScopeNodeSrc = getScopeNode(symbolTable->next->root, ptr->src1->name);

                        //Generate code for the global variable as the first part of the ASSGN in MIPS
                        fprintf(stdout, "lw $t0, _%s\n"
                                        "neg $t1, $t0\n", tempScopeNodeSrc->name);
                    } else {
                        //Generate code for the local variable as the first part of the ASSGN in MIPS
                        fprintf(stdout, "lw $t0, %d($fp)\n"
                                        "neg $t1, $t0\n", tempScopeNodeSrc->address);
                    }

                    //Check local scope for the destination variable
                    tempScopeNodeDest = getScopeNode(symbolTable->root, ptr->dest->name);
                    if (tempScopeNodeDest == NULL) {
                        //If not in local, get it from global and adjust code generation
                        tempScopeNodeDest = getScopeNode(symbolTable->next->root, ptr->dest->name);

                        //Generate code for the negation of the global variable
                        fprintf(stdout, "sw $t1, _%s\n\n", tempScopeNodeDest->name);
                    } else {
                        //Generate code for the negation of the local variable
                        fprintf(stdout, "sw $t1, %d($fp)\n\n", tempScopeNodeDest->address);
                    }
                }
                break;

            case PLUS:
                fprintf(stdout, "# PLUS Operation\n");
                //Confirm if src1 is a global variable or not
                if (ptr->src1->scope) {
                    fprintf(stdout, "lw $t0, _%s\n", ptr->src1->name);
                } else {
                    fprintf(stdout, "lw $t0, %d($fp)\n", ptr->src1->address);
                }

                //Confirm if src2 is a global variable or not
                if (ptr->src2->scope) {
                    fprintf(stdout, "lw $t1, _%s\n", ptr->src2->name);
                } else {
                    fprintf(stdout, "lw $t1, %d($fp)\n", ptr->src2->address);
                }

                fprintf(stdout, "add $t2, $t0, $t1\n");
                //Check local scope for the destination variable
                tempScopeNodeDest = getScopeNode(symbolTable->root, ptr->dest->name);
                if (tempScopeNodeDest == NULL) {
                    //If not in local, get it from global and adjust code generation
                    tempScopeNodeDest = getScopeNode(symbolTable->next->root, ptr->dest->name);

                    //Generate code for the global variable as the second part of the ASSGN in MIPS
                    fprintf(stdout, "sw $t2, _%s\n\n", tempScopeNodeDest->name);
                } else {
                    //Generate code for the local variable as the second part of the ASSGN in MIPS
                    fprintf(stdout, "sw $t2, %d($fp)\n\n", tempScopeNodeDest->address);
                }
                break;

            case MINUS:
                fprintf(stdout, "# MINUS Operation\n");
                //Confirm if src1 is a global variable or not
                if (ptr->src1->scope) {
                    fprintf(stdout, "lw $t0, _%s\n", ptr->src1->name);
                } else {
                    fprintf(stdout, "lw $t0, %d($fp)\n", ptr->src1->address);
                }

                //Confirm if src2 is a global variable or not
                if (ptr->src2->scope) {
                    fprintf(stdout, "lw $t1, _%s\n", ptr->src2->name);
                } else {
                    fprintf(stdout, "lw $t1, %d($fp)\n", ptr->src2->address);
                }

                fprintf(stdout, "sub $t2, $t0, $t1\n");
                //Check local scope for the destination variable
                tempScopeNodeDest = getScopeNode(symbolTable->root, ptr->dest->name);
                if (tempScopeNodeDest == NULL) {
                    //If not in local, get it from global and adjust code generation
                    tempScopeNodeDest = getScopeNode(symbolTable->next->root, ptr->dest->name);

                    //Generate code for the global variable as the second part of the ASSGN in MIPS
                    fprintf(stdout, "sw $t2, _%s\n\n", tempScopeNodeDest->name);
                } else {
                    //Generate code for the local variable as the second part of the ASSGN in MIPS
                    fprintf(stdout, "sw $t2, %d($fp)\n\n", tempScopeNodeDest->address);
                }
                break;

            case MULTIPLY:
                fprintf(stdout, "# MULTIPLY Operation\n");
                //Confirm if src1 is a global variable or not
                if (ptr->src1->scope) {
                    fprintf(stdout, "lw $t0, _%s\n", ptr->src1->name);
                } else {
                    fprintf(stdout, "lw $t0, %d($fp)\n", ptr->src1->address);
                }

                //Confirm if src2 is a global variable or not
                if (ptr->src2->scope) {
                    fprintf(stdout, "lw $t1, _%s\n", ptr->src2->name);
                } else {
                    fprintf(stdout, "lw $t1, %d($fp)\n", ptr->src2->address);
                }

                fprintf(stdout, "mul $t2, $t0, $t1\n");
                //Check local scope for the destination variable
                tempScopeNodeDest = getScopeNode(symbolTable->root, ptr->dest->name);
                if (tempScopeNodeDest == NULL) {
                    //If not in local, get it from global and adjust code generation
                    tempScopeNodeDest = getScopeNode(symbolTable->next->root, ptr->dest->name);

                    //Generate code for the global variable as the second part of the ASSGN in MIPS
                    fprintf(stdout, "sw $t2, _%s\n\n", tempScopeNodeDest->name);
                } else {
                    //Generate code for the local variable as the second part of the ASSGN in MIPS
                    fprintf(stdout, "sw $t2, %d($fp)\n\n", tempScopeNodeDest->address);
                }
                break;

            case DIVIDE:
                fprintf(stdout, "# DIVIDE Operation\n");
                //Confirm if src1 is a global variable or not
                if (ptr->src1->scope) {
                    fprintf(stdout, "lw $t0, _%s\n", ptr->src1->name);
                } else {
                    fprintf(stdout, "lw $t0, %d($fp)\n", ptr->src1->address);
                }

                //Confirm if src2 is a global variable or not
                if (ptr->src2->scope) {
                    fprintf(stdout, "lw $t1, _%s\n", ptr->src2->name);
                } else {
                    fprintf(stdout, "lw $t1, %d($fp)\n", ptr->src2->address);
                }

                fprintf(stdout, "div $t2, $t0, $t1\n");
                //Check local scope for the destination variable
                tempScopeNodeDest = getScopeNode(symbolTable->root, ptr->dest->name);
                if (tempScopeNodeDest == NULL) {
                    //If not in local, get it from global and adjust code generation
                    tempScopeNodeDest = getScopeNode(symbolTable->next->root, ptr->dest->name);

                    //Generate code for the global variable as the second part of the ASSGN in MIPS
                    fprintf(stdout, "sw $t2, _%s\n\n", tempScopeNodeDest->name);
                } else {
                    //Generate code for the local variable as the second part of the ASSGN in MIPS
                    fprintf(stdout, "sw $t2, %d($fp)\n\n", tempScopeNodeDest->address);
                }
                break;

            case RET:
                fprintf(stdout, "# Exiting function\n");
                if (ptr->dest == NULL) {
                    //Prints out the rest of the return
                    fprintf(stdout, "li $v0, 0\n");
                } else {
                    //Check local scope for the destination variable
                    tempScopeNodeDest = getScopeNode(symbolTable->root, ptr->dest->name);
                    if (tempScopeNodeDest == NULL) {
                        //If not in local, get it from global and adjust code generation
                        tempScopeNodeDest = getScopeNode(symbolTable->next->root, ptr->dest->name);

                        //Generate code for the global variable as the second part of the RETURN in MIPS
                        fprintf(stdout, "lw $v0, _%s\n", tempScopeNodeDest->name);
                    } else {
                        //Generate code for the local variable as the second part of the RETURN in MIPS
                        fprintf(stdout, "lw $v0, %d($fp)\n", tempScopeNodeDest->address);
                    }
                }
                //Prints out the rest of the return
                fprintf(stdout, "la $sp, 0($fp)\n"
                                "lw $ra, 0($sp)\n"
                                "lw $fp, 4($sp)\n"
                                "la $sp, 8($sp)\n"
                                "jr $ra\n\n");
                break;

            default:
                break;
        }
        ptr = ptr->next;
    }
}

//Prints code for a list of global variables
void printGlobalVariableList() {
    //Enters first condition if we're dealing with a global variable list declaration
    //Enters second condition if we're dealing with a local variable
    if (addGlobal) {
        //Enters first condition if we're printing the first global variable
        //Enters second condition if we're printing the rest of the global variables
        if (freeFunctionName) {
            //Prints the .data for the first global variable declared
            fprintf(stdout, ".data\n_%s: .space 4\n", functionName);

            //Confirming we're done with the functionName variable
            free(functionName);
            freeFunctionName = false;
        } else {
            //Prints the rest of the global variables stored within the prevLexemeCopy
            fprintf(stdout, "_%s: .space 4\n", prevLexemeCopy);
        }
    } else {
        //Update local variable's addresses
        updateLocalSymbolAddress(symbolTable->root, prevLexemeCopy, currentAddress);
        currentAddress -= 4;
        numOfOperations -= 4;
    }
}

//Frees the AST after we're done
void freeAST() {
    if (ast->root != NULL) {
        //Free the three-address code instructions first
        freeThreeAddressCodeNode(ast->root->codeHead);
        //Free the rest of the AST
        freeASTnode(ast->root);
    }
    free(ast);
}

//Helper function to free all the nodes of the AST
void freeASTnode(ASTnode *node) {
    if (node == NULL) {
        free(node);
        return;
    }
    freeASTnode(node->child0);
    freeASTnode(node->child1);
    freeASTnode(node->child2);
    free(node);
}

//Helper function to free all the nodes of the three-address code
void freeThreeAddressCodeNode(ThreeAddressCodeNode* node){
    if(node == NULL){
        free(node);
        return;
    }
    freeThreeAddressCodeNode(node->next);
    free(node);
}

//Prints the list of three-address code instructions for debugging
void printThreeAddressCodeNode(ThreeAddressCodeNode* node, int pla){
    if(node == NULL){
        return;
    }
    printf("node num %d: Op is %d\n",pla, node->op);
    printThreeAddressCodeNode(node->next,pla+1);
}

/*********************************** Functions from here on are an implementation from Dr. Debray to print the AST ***********************************/

/*
 * ptr: an arbitrary non-NULL AST pointer; ast_node_type() returns the node type
 * for the AST node ptr points to.
 */

NodeType ast_node_type(void *ptr){
    ASTnode* node = ptr;
    assert(node != NULL);
    return node->ntype;
}

/*
 * ptr: pointer to an AST for a function definition; func_def_name() returns
 * a pointer to the function name (a string) of the function definition AST that
 * ptr points to.
 */
char * func_def_name(void *ptr){
    ASTnode* node = ptr;
    assert(node != NULL);
    return node->st_ref;
}

/*
 * ptr: pointer to an AST for a function definition; func_def_nargs() returns
 * the number of formal parameters for that function.
 */
int func_def_nargs(void *ptr){
    ASTnode* node = ptr;
    assert(node != NULL);
    return node->parameters;
}

/*
 * ptr: pointer to an AST for a function definition, n is an integer. If n > 0
 * and n <= no. of arguments for the function, then func_def_argname() returns
 * a pointer to the name (a string) of the nth formal parameter for that function;
 * the first formal parameter corresponds to n == 1.  If the value of n is outside
 * these parameters, the behavior of this function is undefined.
 */
char *func_def_argname(void *ptr, int n){
    ASTnode* node = ptr;
    assert(func_def_nargs(node) > 0);
    assert(n <= func_def_nargs(node));
    ASTnode *temp = node->child1;
    for(int i = 1; i < n; i++){
        temp = temp->child0;
    }
    return temp->st_ref;
}

/*
 * ptr: pointer to an AST for a function definition; func_def_body() returns
 * a pointer to the AST that is the function body of the function that ptr
 * points to.
 */
void * func_def_body(void *ptr){
    ASTnode* node = ptr;
    assert(node != NULL);
    return node->child0;
}

/*
 * ptr: pointer to an AST node for a function call; func_call_callee() returns
 * a pointer to a string that is the name of the function being called.
 */
char * func_call_callee(void *ptr){
    ASTnode* node = ptr;
    assert(node != NULL);
    return node->callee;
}

/*
 * ptr: pointer to an AST node for a function call; func_call_args() returns
 * a pointer to the AST that is the list of arguments to the call.
 */
void * func_call_args(void *ptr){
    ASTnode* node = ptr;
    assert(node != NULL);
    return node->child0;
}

/*
 * ptr: pointer to an AST node for a statement list; stmt_list_head() returns
 * a pointer to the AST of the statement at the beginning of this list.
 */
void * stmt_list_head(void *ptr){
    ASTnode* node = ptr;
    assert(node != NULL);
    return node->child0;
}

/*
 * ptr: pointer to an AST node for a statement list; stmt_list_rest() returns
 * a pointer to the AST of the rest of this list (i.e., the pointer to the
 * next node in the list).
 */
void * stmt_list_rest(void *ptr){
    ASTnode* node = ptr;
    assert(node != NULL);
    return node->child1;
}

/*
 * ptr: pointer to an AST node for an expression list; expr_list_head() returns
 * a pointer to the AST of the expression at the beginning of this list.
 */
void * expr_list_head(void *ptr){
    ASTnode* node = ptr;
    assert(node != NULL);
    return node->child0;
}

/*
 * ptr: pointer to an AST node for an expression list; expr_list_rest() returns
 * a pointer to the AST of the rest of this list (i.e., the pointer to the
 * next node in the list).
 */
void * expr_list_rest(void *ptr){
    ASTnode* node = ptr;
    assert(node != NULL);
    return node->child1;
}

/*
 * ptr: pointer to an AST node for an IDENTIFIER; expr_id_name() returns a
 * pointer to the name of the identifier (a string).
 */
char *expr_id_name(void *ptr){
    ASTnode* node = ptr;
    assert(node != NULL);
    return node->st_ref;
}

/*
 * ptr: pointer to an AST node for an INTCONST; expr_intconst_val() returns the
 * integer value of the constant.
 */
int expr_intconst_val(void *ptr){
    ASTnode* node = ptr;
    assert(node != NULL);
    return node->num;
}

/*
 * ptr: pointer to an AST node for an arithmetic or boolean expression.
 * expr_operand_1() returns a pointer to the AST of the first operand.
 */
void * expr_operand_1(void *ptr){
    ASTnode* node = ptr;
    assert(node != NULL);
    return node->child0;
}

/*
 * ptr: pointer to an AST node for an arithmetic or boolean expression.
 * expr_operand_2() returns a pointer to the AST of the second operand.
 */
void * expr_operand_2(void *ptr){
    ASTnode* node = ptr;
    assert(node != NULL);
    return node->child1;
}

/*
 * ptr: pointer to an AST node for an IF statement.  stmt_if_expr() returns
 * a pointer to the AST for the expression tested by the if statement.
 */
void * stmt_if_expr(void *ptr){
    ASTnode* node = ptr;
    assert(node != NULL);
    return node->child0;
}

/*
 * ptr: pointer to an AST node for an IF statement.  stmt_if_then() returns
 * a pointer to the AST for the then-part of the if statement, i.e., the
 * statement to be executed if the condition is true.
 */
void * stmt_if_then(void *ptr){
    ASTnode* node = ptr;
    assert(node != NULL);
    return node->child1;
}

/*
 * ptr: pointer to an AST node for an IF statement.  stmt_if_else() returns
 * a pointer to the AST for the else-part of the if statement, i.e., the
 * statement to be executed if the condition is false.
 */
void * stmt_if_else(void *ptr){
    ASTnode* node = ptr;
    assert(node != NULL);
    return node->child2;
}

/*
 * ptr: pointer to an AST node for an assignment statement.  stmt_assg_lhs()
 * returns a pointer to the name of the identifier on the LHS of the
 * assignment.
 */
char *stmt_assg_lhs(void *ptr){
    ASTnode* node = ptr;
    assert(node != NULL);
    return node->child0->st_ref;
}

/*
 * ptr: pointer to an AST node for an assignment statement.  stmt_assg_rhs()
 * returns a pointer to the AST of the expression on the RHS of the assignment.
 */
void *stmt_assg_rhs(void *ptr){
    ASTnode* node = ptr;
    assert(node != NULL);
    return node->child1;
}

/*
 * ptr: pointer to an AST node for a while statement.  stmt_while_expr()
 * returns a pointer to the AST of the expression tested by the while statement.
 */
void *stmt_while_expr(void *ptr){
    ASTnode* node = ptr;
    assert(node != NULL);
    return node->child0;
}

/*
 * ptr: pointer to an AST node for a while statement.  stmt_while_body()
 * returns a pointer to the AST of the body of the while statement.
 */
void *stmt_while_body(void *ptr){
    ASTnode* node = ptr;
    assert(node != NULL);
    return node->child1;
}

/*
 * ptr: pointer to an AST node for a return statement.  stmt_return_expr()
 * returns a pointer to the AST of the expression whose value is returned.
 */
void *stmt_return_expr(void *ptr){
    ASTnode* node = ptr;
    assert(node != NULL);
    return node->child0;
}