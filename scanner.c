//
// Created by Jose Juan Velasquez on 1/18/2024.
//
#include "scanner.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

//Identifies if character string is a keyword or an identifier
int keywd_or_id(char *lexeme);

//Checks is current character is alphanumeric
bool isalphanum(char ch);

//Skips over white space and comment sections
void skip_whitespace_and_comments();

//Verifies an || operation
int checkOr(char ch);

//Verifies an && operation
int checkAnd(char ch);

//Verifies an != or ! operation
int checkNotEq(char ch);

//Verifies an == or = operation
int checkEq(char ch);

//Verifies an <= or < operation
int checkLessThan(char ch);

//Verifies an >= or > operation
int checkGreaterThan(char ch);

//Verifies if given character is an integer constant
bool isDigit(char ch);

//Verifies if given character is a character string
bool isAlpha(char ch);

//Returns to the parser the number lines in the code
int getLines();

char* lexeme;
bool freeLexeme;
int bufferSize;
int line = 1;

//This function has code and code styles shown in Prof. Saumya Debray's lecture slides,
int get_token() {
    /* ch: variable holding the characters from the stdin */
    char ch;
    //Skip any whitespace and comments before identifying token
    skip_whitespace_and_comments();
    ch = getchar();

    //If lexeme is pointing to allocated memory, free it
    if (freeLexeme) {
        free(lexeme);
        freeLexeme = false;
    }
    //This snippet of code is from Prof. Saumya Debray's slides
    if (ch == EOF) return EOF;
    lexeme = "(";
    if (ch == '(') return LPAREN;
    lexeme = ")";
    if (ch == ')') return RPAREN;
    lexeme = "{";
    if (ch == '{') return LBRACE;
    lexeme = "}";
    if (ch == '}') return RBRACE;
    lexeme = ";";
    if (ch == ';') return SEMI;
    lexeme = ",";
    if (ch == ',') return COMMA;
    lexeme = "+";
    if (ch == '+') return opADD;
    lexeme = "-";
    if (ch == '-') return opSUB;
    lexeme = "*";
    if (ch == '*') return opMUL;
    lexeme = "/";
    if (ch == '/') return opDIV;
    if (ch == '=') return checkEq(ch);
    if (ch == '!') return checkNotEq(ch);
    if (ch == '&') return checkAnd(ch);
    if (ch == '|') return checkOr(ch);
    if (ch == '>') return checkGreaterThan(ch);
    if (ch == '<') return checkLessThan(ch);
    if (isAlpha(ch)) return keywd_or_id(lexeme);
    if (isDigit(ch)) return INTCON;
    return UNDEF;
}

//Verifies if given character is a character string
bool isAlpha(char ch) {
    //Before it proceeds, checks to see if given character is alphabetic
    if (isalpha(ch)) {
        /* bufferSize: the size of the current buffer, which will grow each time a char is found
         * buffer: the space in memory that will hold the stdin */
        bufferSize = 1;
        char *buffer = (char *) calloc(bufferSize + 1, sizeof(char));

        //Populate first block of memory then get next character from the stdin
        buffer[bufferSize - 1] = ch;
        ch = getchar();

        //Loop while we keep finding alphanumeric characters or the '_' character
        while (isalphanum(ch) || ch == '_') {
            //Realloc to place more characters into the buffer
            bufferSize++;
            buffer = (char *) realloc(buffer, bufferSize + 1);
            buffer[bufferSize - 1] = ch;
            ch = getchar();
        }
        //Assign the last slot in the buffer memory to the terminate symbol
        buffer[bufferSize] = '\0';

        //Confirm we do not need to return a character for the next iteration to stdin
        if (ch != EOF) {
            ungetc(ch, stdin);
        }
        //Allocate memory and have the given lexeme pointer point to it
        lexeme = (char *) calloc(bufferSize + 1, sizeof(char));
        //Copy the contents of the buffer to lexeme and free the buffer
        lexeme = strcpy(lexeme, buffer);
        free(buffer);
        //Let the scanner know it should free the lexeme's allocated memory
        freeLexeme = true;
        //Returns true if we successfully stored the given stdin token to the lexeme
        return true;
    }
    //Returns false if given character is not alphabetic
    return false;
}

//Verifies if given character is an integer constant
bool isDigit(char ch) {
    //Before it proceeds, checks to see if given character is numeric
    if (isdigit(ch)) {
        /* bufferSize: the size of the current buffer, which will grow each time a char is found
         * buffer: the space in memory that will hold the stdin */
        bufferSize = 1;
        char *buffer = (char *) calloc(bufferSize + 1, sizeof(char));

        //Populate first block of memory then get next character from the stdin
        buffer[bufferSize - 1] = ch;
        ch = getchar();

        //Loop while we keep finding numeric characters
        while (isdigit(ch)) {
            //Realloc to place more characters into the buffer
            bufferSize++;
            buffer = (char *) realloc(buffer, bufferSize + 1);
            buffer[bufferSize - 1] = ch;
            ch = getchar();
        }
        //Assign the last slot in the buffer memory to the terminate symbol
        buffer[bufferSize] = '\0';

        //Confirm we do not need to return a character for the next iteration to stdin
        if (ch != EOF) {
            ungetc(ch, stdin);
        }

        //Allocate memory and have the given lexeme pointer point to it
        lexeme = (char *) calloc(bufferSize + 1, sizeof(char));
        //Copy the contents of the buffer to lexeme and free the buffer
        lexeme = strcpy(lexeme, buffer);
        free(buffer);
        //Let the scanner know it should free the lexeme's allocated memory
        freeLexeme = true;
        //Returns true if we successfully stored the given stdin token to the lexeme
        return true;
    }
    //Returns false if given character is not a numeric value
    return false;
}

//Verifies an >= or > operation
int checkGreaterThan(char ch) {
    //Get the next character from stdin after the '>' symbol
    ch = getchar();
    //If that next character is an '=' character, '>=' token is confirmed
    if (ch == '=') {
        lexeme = ">=";
        return opGE;
    }

    //At this point, '>' token is confirmed
    lexeme = ">";
    //Return extra character to stdin
    ungetc(ch, stdin);
    return opGT;
}

//Verifies an <= or < operation
int checkLessThan(char ch) {
    //Get the next character from stdin after the '<' symbol
    ch = getchar();
    //If that next character is an '=' character, '<=' token is confirmed
    if (ch == '=') {
        lexeme = "<=";
        return opLE;
    }

    //At this point, '<' token is confirmed
    lexeme = "<";
    //Return extra character to stdin
    ungetc(ch, stdin);
    return opLT;
}

//Verifies an == or = operation
int checkEq(char ch) {
    //Get the next character from stdin after the '=' symbol
    ch = getchar();
    //If that next character is an '=' character, '==' token is confirmed
    if (ch == '=') {
        lexeme = "==";
        return opEQ;
    }

    //At this point, '=' token is confirmed
    lexeme = "=";
    //Return extra character to stdin
    ungetc(ch, stdin);
    return opASSG;
}

//Verifies an != or ! operation
int checkNotEq(char ch) {
    //Get the next character from stdin after the '!' symbol
    ch = getchar();
    //If that next character is an '=' character, '!=' token confirmed
    if (ch == '=') {
        lexeme = "!=";
        return opNE;
    }
    //Logical not has been confirmed, parser will confirm if it's used correctly
    lexeme = "!";
    //Return extra character to stdin
    ungetc(ch, stdin);
    return opNOT;
}

//Verifies an && operation
int checkAnd(char ch) {
    //Get the next character from stdin after the '&' symbol
    ch = getchar();
    //If that next character is an '&' character, '&&' token is confirmed
    if (ch == '&') {
        lexeme = "&&";
        return opAND;
    }
    //No symbol was recognized after the first '&' return undefined
    lexeme = "&";
    //Return extra character to stdin
    ungetc(ch, stdin);
    return UNDEF;
}

//Verifies an || operation
int checkOr(char ch) {
    //Get the next character from stdin after the '|' symbol
    ch = getchar();
    //If that next character is an '&' character, '||' token is confirmed
    if (ch == '|') {
        lexeme = "||";
        return opOR;
    }

    //No symbol was recognized after the first '&' return undefined
    lexeme = "|";
    //Return extra character to stdin
    ungetc(ch, stdin);
    return UNDEF;
}

//Checks is current character is alphanumeric
bool isalphanum(char ch) {
    return isdigit(ch) || isalpha(ch);
}

//Identifies if character string is a keyword or an identifier
int keywd_or_id(char *lexeme) {
    //String comparison for known keywords
    if (strcmp(lexeme, "if") == 0) {
        return kwIF;
    } else if (strcmp(lexeme, "else") == 0) {
        return kwELSE;
    } else if (strcmp(lexeme, "while") == 0) {
        return kwWHILE;
    } else if (strcmp(lexeme, "int") == 0) {
        return kwINT;
    } else if (strcmp(lexeme, "return") == 0) {
        return kwRETURN;
    } else {
        //If it does not match any given keyword, it is an identifier
        return ID;
    }
}

//Skips over white space and comment sections
void skip_whitespace_and_comments() {
    /*ch: the variable holding the characters from stdin
     * temp: a temporary variable that will hold the initial '/' from stdin */
    char ch;
    char temp;
    //Get the next character from stdin
    ch = getchar();

    //Skipping over any type of whitespace
    while (ch == ' ' || ch == '\n' || ch == '\t') {
        //Gets the current lines of the code
        if (ch == '\n') {
            line++;
        }
        ch = getchar();
    }

    /* Code implemented from the FSA for C comments from Prof. Saumya Debray's lecture slides */

    //Confirms if initial character is a '/'
    if (ch == '/') {
        //Store the '/' into the temp variable for later
        temp = ch;
        ch = getchar();

        //Confirm if next character is a '*'
        if (ch == '*') {
            //At this point, we confirmed we have a comment so discard following characters
            while (true) {
                ch = getchar();
                //If current character is a '*', confirm it is not the ending of the comment
                if (ch == '*') {
                    //Confirms that there are more '*' characters
                    while (ch == '*') {
                        ch = getchar();
                        //If current character is '/' the comment ended
                        if (ch == '/') {
                            //Recurse into the function in case a comment follows another comment
                            skip_whitespace_and_comments();
                            return;
                        }
                    }
                }
            }
        } else {
            //No '*' found after initial '/', return both characters to stdin
            ungetc(ch, stdin);
            ungetc(temp, stdin);
            return;
        }
    } else {
        //No comment section detected, return the character to the stdin
        ungetc(ch, stdin);
        return;
    }
}

//Returns to the parser the number lines in the code
int getLines() {
    return line;
}