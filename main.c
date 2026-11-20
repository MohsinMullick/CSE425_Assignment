/* Programming Assignment Part 1
   Adapted from the lecture's getChar/addChar/lookup/lex and
   expr/term/factor recursive-descent examples.
   Custom additions: underscores in identifiers, decimal numbers,
   unary +/- and explicit lexical/syntax diagnostics.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_INPUT 1024
#define MAX_LEXEME 128

#define LETTER 0
#define DIGIT 1
#define UNKNOWN 99
#define END_OF_INPUT -1

#define NUMBER 10
#define IDENT 11
#define ADD_OP 21
#define SUB_OP 22
#define MULT_OP 23
#define DIV_OP 24
#define LEFT_PAREN 25
#define RIGHT_PAREN 26
#define INVALID -2

static char input[MAX_INPUT];
static size_t pos = 0;
static int charClass, nextChar, nextToken;
static char lexeme[MAX_LEXEME];
static size_t lexLen;
static int failed = 0;

/* Get next character and determine its class */
static void getChar(void) {
    nextChar = (unsigned char)input[pos];

    if (nextChar == '\0')
        charClass = END_OF_INPUT;
    else if (isalpha(nextChar) || nextChar == '_')
        charClass = LETTER;
    else if (isdigit(nextChar))
        charClass = DIGIT;
    else
        charClass = UNKNOWN;
}

/* Add character to the current lexeme */
static void addChar(void) {
    if (lexLen + 1 >= MAX_LEXEME) {
        fprintf(stderr, "Lexical error: lexeme too long.\n");
        exit(EXIT_FAILURE);
    }

    lexeme[lexLen++] = (char)nextChar;
    lexeme[lexLen] = '\0';
}

static void advance(void) {
    ++pos;
    getChar();
}

/* Ignore whitespace */
static void getNonBlank(void) {
    while (nextChar != '\0' &&
           isspace((unsigned char)nextChar)) {
        advance();
    }
}

/* Recognize operators and parentheses */
static int lookup(int ch) {
    switch (ch) {
        case '(': return LEFT_PAREN;
        case ')': return RIGHT_PAREN;
        case '+': return ADD_OP;
        case '-': return SUB_OP;
        case '*': return MULT_OP;
        case '/': return DIV_OP;
        default: return INVALID;
    }
}

/* Convert token code to readable name */
static const char *tokenName(int code) {
    switch (code) {
        case NUMBER: return "NUMBER";
        case IDENT: return "IDENT";
        case ADD_OP: return "ADD_OP";
        case SUB_OP: return "SUB_OP";
        case MULT_OP: return "MULT_OP";
        case DIV_OP: return "DIV_OP";
        case LEFT_PAREN: return "LEFT_PAREN";
        case RIGHT_PAREN: return "RIGHT_PAREN";
        case END_OF_INPUT: return "EOF";
        default: return "INVALID";
    }
}

/* Lexical Analyzer */
static int lex(void) {
    getNonBlank();

    lexLen = 0;
    lexeme[0] = '\0';

    if (charClass == LETTER) {

        do {
            addChar();
            advance();
        } while (charClass == LETTER ||
                 charClass == DIGIT);

        nextToken = IDENT;

    } else if (charClass == DIGIT) {

        do {
            addChar();
            advance();
        } while (charClass == DIGIT);

        if (nextChar == '.') {
            addChar();
            advance();

            if (charClass != DIGIT) {
                nextToken = INVALID;
            } else {
                do {
                    addChar();
                    advance();
                } while (charClass == DIGIT);

                nextToken = NUMBER;
            }
        } else {
            nextToken = NUMBER;
        }

        if (charClass == LETTER) {
            do {
                addChar();
                advance();
            } while (charClass == LETTER ||
                     charClass == DIGIT);

            nextToken = INVALID;
        }

    } else if (charClass == END_OF_INPUT) {

        strcpy(lexeme, "EOF");
        nextToken = END_OF_INPUT;

    } else {

        int ch = nextChar;
        addChar();
        advance();
        nextToken = lookup(ch);
    }

    printf("  %-14s %-12s (%d)\n",
           tokenName(nextToken),
           lexeme,
           nextToken);

    if (nextToken == INVALID)
        failed = 1;

    return nextToken;
}

/* Error handling */
static void syntaxError(const char *expected) {
    if (!failed) {
        printf("Syntax error: expected %s, found '%s'.\n",
               expected, lexeme);
    }
    failed = 1;
}

/* Recursive-Descent Parser */
static void expr(void);
static void factor(void);

/* unary = [ '+' | '-' ], factor */
static void unary(void) {
    if (failed)
        return;

    if (nextToken == ADD_OP ||
        nextToken == SUB_OP) {
        lex();
    }

    factor();
}

/* term = unary, { ('*' | '/'), unary } */
static void term(void) {
    unary();

    while (!failed &&
           (nextToken == MULT_OP ||
            nextToken == DIV_OP)) {
        lex();
        unary();
    }
}

/* expr = term, { ('+' | '-'), term } */
static void expr(void) {
    term();

    while (!failed &&
           (nextToken == ADD_OP ||
            nextToken == SUB_OP)) {
        lex();
        term();
    }
}

/* factor = NUMBER | IDENT | '(' expr ')' */
static void factor(void) {
    if (failed)
        return;

    if (nextToken == NUMBER ||
        nextToken == IDENT) {

        lex();

    } else if (nextToken == LEFT_PAREN) {

        lex();
        expr();

        if (failed)
            return;

        if (nextToken == RIGHT_PAREN)
            lex();
        else
            syntaxError("')'");

    } else {
        syntaxError("number, identifier or '('");
    }
}

/* Main Function */
int main(void) {

    printf("Enter an arithmetic expression: ");

    if (!fgets(input, sizeof(input), stdin))
        return EXIT_FAILURE;

    size_t n = strlen(input);

    if (n && input[n - 1] == '\n') {
        input[n - 1] = '\0';
    } else if (n == sizeof(input) - 1) {
        puts("Input error: expression too long.");
        return EXIT_FAILURE;
    }

    getChar();

    puts("\nTOKENS:");

    lex();
    expr();

    if (!failed && nextToken != END_OF_INPUT)
        syntaxError("end of expression");

    printf("\nRESULT: %s expression\n",
           failed ? "INVALID" : "VALID");

    return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
