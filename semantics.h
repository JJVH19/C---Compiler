//
// Created by Jose Juan Velasquez on 2/6/2024.
//

#include <stdbool.h>
#include "scanner.c"
#ifndef MILESTONE_1_SEMANTICS_H
#define MILESTONE_1_SEMANTICS_H

#endif //MILESTONE_1_SEMANTICS_H

//Defines the types for the symbol tables
typedef enum {
    variable,
    function,
} TYPE;

//The actual Symbol table
typedef struct SymbolTableNode{
    //The ID
    char *name;
    //The INTCONST
    int num;
    //Type should always be variable
    TYPE type;
    //MIPS assembly location address
    int address;
    //Parameter amount of a function
    int parameters;
    //True is global, false is local
    bool scope;
    struct SymbolTableNode* next;
} SymbolTableNode;

//The root to the GlobalScope LinkedList
typedef struct SymbolTable {
    SymbolTableNode *root;
    struct SymbolTable* next;
} SymbolTable;

//Helper function to manage semantic errors
void freeExitError(int counter);

//Adds to the correspondent symbol or throws error if it detects a semantic error
void checkAndPopulateSymbolTables(TYPE type, char* name);

//Initiates the semantic checking
void manageSymbolTable();

//Manages tables if we're dealing with a function call or not
void manage_fn_call_check(char *name);

//Confirms if current variable is not in the local symbol table
void manage_local_normal_variable(TYPE type, char *name);

//Confirms if current variable is not in the global symbol table
void manage_global_normal_variable(TYPE type, char *name);

///////////////////////////////////////////////////////////////////////////
//Create symbol table
SymbolTable * createSymbolTableRoot();

//Creates the nodes for the global symbol table
SymbolTableNode* createSymbolTableNode(char* name, int num, TYPE type,int address,int parameters,bool scope);

//Add node to the symbol table and fill in the local or global field depending on the bool global
void populateSymbolTable(TYPE type, char* name,bool scope);

//add the node to the symbol table
void addSymbol(SymbolTableNode* node, TYPE type, char* name, bool global);

//Deallocate memory from the Local symbol table
void freeSymbolTable();

//Only frees the local scope nodes within the symbol table
void freeLocalSymbolTable();

//Frees the while symbol table
void freeSymbolTableNode(SymbolTableNode *node);

//Prints the symbol table for debugging
void printSymbolTable();

//Prints the symbol table nodes
void printSymbolTableNode(SymbolTableNode* node);

//Function will create the root pointer to the global symbol table
SymbolTable* createSymbolTableRoot();

bool searchSymbolTable(SymbolTableNode* node, TYPE type, char* name);

//Will update the parameter field indicating the number of parameters passed into a function
void updateSymbolTableParameters(SymbolTableNode * node, char* name, int parameters);

//Will verify if function call and global function have the same number of parameters
bool compareFunctionCallParameters(SymbolTableNode * node, char* name, int parameters);

//Gets the ID reference from the global symbol table
char* getIDFromSymbolTable(SymbolTableNode * node, char* name);

//Gets the Local Symbol table node
SymbolTableNode *getScopeNode(SymbolTableNode *node, char* name);