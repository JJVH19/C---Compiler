//
// Created by Jose Juan Velasquez on 2/6/2024.
//

#include "semantics.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

char *prevLexeme;
extern char* functionName;
extern char* prevVar;
extern char* prevLexemeCopy;
extern char* functionNameCopy;
extern char* copyLexeme;
extern int token;
extern bool local;
extern bool fn_call_check;
extern bool freePrevLexeme;
extern bool freeFunctionName;
extern bool justSearch;
extern bool ASTFree;
extern bool prevVarDecl;
extern bool freePrevLexemeCopy;
extern bool freeFunctionNameCopy;
extern bool freeCopyLexeme;
extern SymbolTable* symbolTable;

//Function will create the root pointer to the global symbol table
SymbolTable * createSymbolTableRoot() {
    SymbolTable *localScope = (SymbolTable *) calloc(1, sizeof(SymbolTable));
    SymbolTable *globalScope = (SymbolTable *) calloc(1, sizeof(SymbolTable));
    localScope->root = NULL;
    localScope->next = globalScope;
    globalScope->root = NULL;
    globalScope->next = NULL;
    return localScope;
}

//Creates the nodes for the global symbol table
SymbolTableNode* createSymbolTableNode(char* name, int num, TYPE type,int address,int parameters,bool scope) {
    SymbolTableNode *node = (SymbolTableNode *) calloc(1, sizeof(SymbolTableNode));
    //The ID
    node->name = (char *) calloc(strlen(name) + 1, sizeof(char));
    strcpy(node->name, name);
    //The INTCONST
    node->num = num;
    //Type should always be variable
    node->type = type;
    //MIPS assembly location address
    node->address = address;
    //Parameter amount of a function
    node->parameters = parameters;
    //True is global, false is local
    node->scope = scope;
    node->next = NULL;
    return node;
}

//Initiates the semantic checking
void manageSymbolTable() {
    //In case of an ERROR, let the freeExitError() know to free it there
    freePrevLexeme = true;

    //Save previous lexeme ID to store in symbol tables later on as well as prevToken
    prevLexeme = (char *) calloc(bufferSize + 1, sizeof(char));
    prevLexeme = strcpy(prevLexeme, lexeme);
    //Get new token to know what type of ID we're dealing with (variable or function)
    token = get_token();

    //Enters first condition if we are dealing with a function ID
    //Enters second condition if we are dealing with either a bool expression or an operator assign ID
    //Enters third condition if we are dealing with anything else, must be of variable type
    if (token == LPAREN) {
        //If we find a func_call ID, used as a func_call, but was already declared in local scope, error
        if (!searchSymbolTable(symbolTable->root, variable, prevLexeme)) {
            freeExitError(-1);
        }
        //Confirms the ID is of type function
        if (!searchSymbolTable(symbolTable->next->root, variable, prevLexeme)) {
            freeExitError(-1);
        } else {
            //Confirming we are dealing with a function
            checkAndPopulateSymbolTables(function, prevLexeme);
        }
    } else if (token == opEQ || token == opNE || token == opLE || token == opLT || token == opGE || token == opGT ||
               token == opASSG || token == opADD || token == opOR) {
        //If we cannot find the current ID within the symbol table, error
        if (searchSymbolTable(symbolTable->root, variable, prevLexeme)
            && searchSymbolTable(symbolTable->next->root, variable, prevLexeme)) {
            freeExitError(-1);
        }
    } else {
        //Confirming we are dealing with a variable
        checkAndPopulateSymbolTables(variable, prevLexeme);
    }
    //Free the previous lexeme after it was stored in the symbol table
    free(prevLexeme);
    freePrevLexeme = false;
}

//Adds to the correspondent symbol or throws error if it detects a semantic error
void checkAndPopulateSymbolTables(TYPE type, char* name) {
    //Enters first condition if we should search/manage the local symbol table
    //Enters second condition if we should search/manage the global symbol table
    if (local) {
        //Enters first condition if we're dealing with a func_call
        //Enters second condition if we're dealing with a normal variable ID
        if (fn_call_check) {
            //Recognizes if we're dealing with a function call
            manage_fn_call_check(name);
        } else {
            //Confirms if current variable is not in the local symbol table
            manage_local_normal_variable(type, name);
        }
    } else {
        //Confirm the variable name does not exist as a global function
        if (!(searchSymbolTable(symbolTable->next->root, type, prevLexeme))) {
            freeExitError(-1);
        } else {
            //Confirms if current variable is not in the global symbol table
            manage_global_normal_variable(type, name);
        }
    }
}

//Confirms if current variable is not in the global symbol table
void manage_global_normal_variable(TYPE type, char *name) {
    //In this case, we DO NOT want to find the current global variable ID in the global symbol table
    if (searchSymbolTable(symbolTable->next->root, type, name)) {
        //Add the variable to the table
        populateSymbolTable(type, name, true);
    } else {
        //Prints error message and exits with code 1
        freeExitError(-1);
    }
}

//Confirms if current variable is not in the local symbol table
void manage_local_normal_variable(TYPE type, char *name) {
    //Enters first condition if we're only searching for the ID in the symbol table
    //Enters second condition if we need to add an ID to the symbol table
    if (justSearch) {
        //First search within the local scope if variable ID exists there, if not there, check global scope
        if (searchSymbolTable(symbolTable->root, type, name)
            && searchSymbolTable(symbolTable->next->root, type, name)) {
            //If not in local scope nor global scope, error
            freeExitError(-1);
        }
    } else {
        //In this case, we DO NOT want to find the current lexeme in the local symbol table
        if (searchSymbolTable(symbolTable->root, type, name)) {
            //Add the variable to the table
            populateSymbolTable(type, name, false);
        } else {
            //Prints error message and exits with code 1
            freeExitError(-1);
        }
    }
}

//Manages tables if we're dealing with a function call in the parameters ONLY
void manage_fn_call_check(char *name) {
    //Enters first condition to check if current ID is a function ID
    //Enters second condition to search within the local and global scope if variable ID exists there
    if (!searchSymbolTable(symbolTable->next->root, function, name)) {
        //If it is, check if the next character, current lexeme, is an LPAREN
        if (strcmp(lexeme, "(") != 0) {
            //If it is not, then it is a type error
            freeExitError(-1);
        } else {
            //Otherwise, we can exit checking
            return;
        }
    } else if (searchSymbolTable(symbolTable->root, variable, name)
               && searchSymbolTable(symbolTable->next->root, variable, name)) {
        //If not in local scope not global scope, error
        freeExitError(-1);
    }
}

//Helper function to manage semantic errors
void freeExitError(int counter) {
    //Enters first condition if the error is not within the param count check
    //Enters second condition if the error is due to a param count check
    if (counter == -1) {
        fprintf(stderr, "ERROR LINE %d\n", getLines());
    } else {
        fprintf(stderr, "ERROR LINE %d\n", counter);
    }

    //Avoids segmentation faults in case AST is empty or not allocated
    if (ASTFree) {
        freeAST();
    }
    //Avoids segmentation faults in case lexeme is empty or not allocated
    if (freeLexeme) {
        free(lexeme);
    }
    //Avoids segmentation faults in case prevLexeme is empty or not allocated
    if (freePrevLexeme) {
        free(prevLexeme);
    }
    //Avoids segmentation faults in case functionName is empty or not allocated
    if (freeFunctionName) {
        free(functionName);
    }
    //Avoids segmentation faults in case functionNameCopy is empty or not allocated
    if (freeFunctionNameCopy) {
        free(functionNameCopy);
    }
    //Avoids segmentation faults in case prevVar is empty or not allocated
    if (prevVarDecl) {
        free(prevVar);
    }
    //Avoids segmentation faults in case prevLexemeCopy is empty or not allocated
    if (freePrevLexemeCopy) {
        free(prevLexemeCopy);
    }
    //Avoids segmentation faults in case copyLexeme is empty or not allocated
    if (freeCopyLexeme) {
        free(copyLexeme);
    }
    //Free the whole symbol table
    freeSymbolTable();
    exit(1);
}

//Add node to the symbol table and fill in the local or global field depending on the bool global
void populateSymbolTable(TYPE type, char* name,bool scope) {
    //Enters first condition if we're adding to the global scope
    //Enters second condition if we're adding to the local scope
    if (scope) {
        if (symbolTable->next->root == NULL) {
            symbolTable->next->root = createSymbolTableNode(name, INT_MIN, type, INT_MIN, INT_MIN, scope);
        } else {
            addSymbol(symbolTable->next->root, type, name, scope);
        }
    } else {
        if (symbolTable->root == NULL) {
            symbolTable->root = createSymbolTableNode(name, INT_MIN, type, INT_MIN, INT_MIN, scope);
        } else {
            addSymbol(symbolTable->root, type, name, scope);
        }
    }
}

//Add nodes to the symbol table
void addSymbol(SymbolTableNode* node, TYPE type, char* name, bool global) {
    if (node->next != NULL) {
        //Recurse to find the end of the symbol table to add node
        addSymbol(node->next, type, name, global);
    } else {
        //Create the new symbol table entry, global indicating if current ID is a global or local ID
        node->next = createSymbolTableNode(name, INT_MIN, type, INT_MIN, INT_MIN, global);
    }
}

//Deallocate memory from the symbol table
void freeSymbolTable() {
    if (symbolTable->next->root != NULL) {
        //Call helper function to free all nodes
        freeSymbolTableNode(symbolTable->next->root);
        symbolTable->next->root = NULL;
    }
    if (symbolTable->root != NULL) {
        //Call helper function to free all nodes
        freeSymbolTableNode(symbolTable->root);
        symbolTable->root = NULL;
    }
    free(symbolTable->next);
    free(symbolTable);
}

//Only frees the local scope nodes from the symbol table
void freeLocalSymbolTable() {
    if (symbolTable->root != NULL) {
        freeSymbolTableNode(symbolTable->root);
        symbolTable->root = NULL;
    }
}

//Deallocate the nodes in the symbol table
void freeSymbolTableNode(SymbolTableNode *node) {
    if (node == NULL) {
        free(node);
        return;
    } else {
        freeSymbolTableNode(node->next);
        free(node->name);
        free(node);
    }
}

//Helper function that searches each node to find a global symbol match
bool searchSymbolTable(SymbolTableNode * node, TYPE type, char* name) {
    if (node == NULL) {
        //No match was found, meaning no variable was previously declared globally
        return true;
    } else {
        //To make it here, the globalScopeNode field must not be NULL
        if (node->type == type && strcmp(node->name, name) == 0) {
            //Global variable was already declared
            return false;
        } else {
            //Keep searching for a match
            return searchSymbolTable(node->next, type, name);
        }
    }
}

//Prints the global symbol table
void printSymbolTable() {
    printf("\n\t\t\t********Symbol Table********\n\t\t**Type 0 = Variable and Type 1 = Function **\n\n");
    printSymbolTableNode(symbolTable->root);
    printSymbolTableNode(symbolTable->next->root);
}

//Prints the nodes within the symbol table
void printSymbolTableNode(SymbolTableNode *node) {
    if (node == NULL) {
        return;
    }
    if (node->scope) {
        printf("\t**GLOBAL SCOPE** type: '%d' ID: '%s' Address is %d Parameters: '%d'\n", node->type, node->name,
               node->address, node->parameters);
    } else {
        printf("\t**LOCAL SCOPE** type: '%d' ID: '%s' Address is %d\n", node->type, node->name, node->address);
    }
    printSymbolTableNode(node->next);
}

//Will update the parameter field indicating the number of parameters passed into a function
void updateSymbolTableParameters(SymbolTableNode * node, char* name, int parameters) {
    if (node == NULL) {
        //Nothing updates here
        return;
    } else {
        //To make it here, the globalScopeNode field must not be NULL
        if (strcmp(node->name, name) == 0) {
            //Update parameter value to specify how many parameters the function has
            node->parameters = parameters;
            //Change was successful
            return;
        } else {
            //Keep searching for a match
            updateSymbolTableParameters(node->next, name, parameters);
        }
    }
}

//Will verify if function call and global function have the same number of parameters
bool compareFunctionCallParameters(SymbolTableNode * node, char* name, int parameters) {
    if (node == NULL) {
        //No function was found, semantic error
        return false;
    } else {
        //To make it here, the globalScopeNode field must not be NULL
        if (strcmp(node->name, name) == 0) {
            //return true if number of parameters of function call matches the global scope function
            return node->parameters == parameters;
        } else {
            //Keep searching for a match
            return compareFunctionCallParameters(node->next, name, parameters);
        }
    }
}

//Gets the ID reference from the global symbol table
char* getIDFromSymbolTable(SymbolTableNode * node, char* name) {
    if (node == NULL) {
        //Name was not found
        return NULL;
    } else {
        //To make it here, the globalScopeNode field must not be NULL
        if (strcmp(node->name, name) == 0) {
            //return true if number of parameters of function call matches the global scope function
            return node->name;
        } else {
            //Keep searching for a match
            return getIDFromSymbolTable(node->next, name);
        }
    }
}

//Updates the Local Symbol table's address
void updateLocalSymbolAddress(SymbolTableNode * node, char* name, int address) {
    if (node == NULL) {
        //Nothing was updated
        return;
    } else {
        //To make it here, the globalScopeNode field must not be NULL
        if (strcmp(node->name, name) == 0) {
            node->address = address;
        } else {
            return updateLocalSymbolAddress(node->next, name, address);
        }
    }
}

//Gets the Local Symbol table's address
int getSymbolAddress(SymbolTableNode * node, char* name) {
    if (node == NULL) {
        //Node was not found, return default value
        return INT_MIN;
    } else {
        //To make it here, the globalScopeNode field must not be NULL
        if (strcmp(node->name, name) == 0) {
            return node->address;
        } else {
            //Keep searching for a match
            return getSymbolAddress(node->next, name);
        }
    }
}

//Gets the Local Symbol table node
SymbolTableNode* getScopeNode(SymbolTableNode *node, char* name) {
    if (node == NULL) {
        return NULL;
    } else {
        //Confirms we found the node we're looking for
        if (strcmp(node->name, name) == 0) {
            return node;
        } else {
            //Keep searching until we find a match
            return getScopeNode(node->next, name);
        }
    }
}
