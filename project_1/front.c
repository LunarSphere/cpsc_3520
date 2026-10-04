/*
Kevius Tribble
CPSC 3520 Programming Systems
Project 1: Lexical Analyzer 
Reads an input file and produces a list of tokens and lexemes.
Added support for the following:
- Skipping '#' preprocessor directives
- skipping '//' line comments
- skipping '/*' block comments
- reporting lexical errors 
- handling of various operators and keywords
- character and string literals 
- numerical literals 
- keywords 
- symbols and operators
*/


#include <ctype.h>
#include <stdio.h>
#include <string.h>

/*
 * Fixed token codes for Mini-Project 1.
 * Keep these names and numbers unchanged. A later parser can use the same
 * contract when it calls lex().
 *
 * Your name and summary of changes:
 */
enum TokenCode {
    TOK_EOF = -1,
    INT_LIT = 10,
    IDENT = 11,
    ASSIGN_OP = 20,
    ADD_OP = 21,
    SUB_OP = 22,
    MULT_OP = 23,
    DIV_OP = 24,
    LEFT_PAREN = 25,
    RIGHT_PAREN = 26,
    LEFT_BRACE = 27,
    RIGHT_BRACE = 28,
    SEMICOLON = 29,
    COMMA = 30,
    KW_INT = 31,
    KW_VOID = 32,
    KW_MAIN = 33,
    KW_RETURN = 34,
    FLOAT_LIT = 35,
    EQ_OP = 36,
    NE_OP = 37,
    KW_FLOAT = 38,
    KW_IF = 39,
    KW_ELSE = 40,
    KW_FOR = 41,
    KW_WHILE = 42,
    KW_DO = 43,
    KW_BREAK = 44,
    KW_CONTINUE = 45,
    KW_CHAR = 46,
    KW_DOUBLE = 47,
    KW_CONST = 48,
    KW_SWITCH = 49,
    KW_CASE = 50,
    KW_DEFAULT = 51,
    LT_OP = 52,
    GT_OP = 53,
    LE_OP = 54,
    GE_OP = 55,
    NOT_OP = 56,
    AND_OP = 57,
    OR_OP = 58,
    INC_OP = 59,
    DEC_OP = 60,
    MOD_OP = 61,
    ADD_ASSIGN = 62,
    SUB_ASSIGN = 63,
    MUL_ASSIGN = 64,
    DIV_ASSIGN = 65,
    CHAR_LIT = 66,
    STRING_LIT = 67,
    COLON = 68,
    LEX_ERROR = 98
};

#define LETTER 0
#define DIGIT 1
#define UNKNOWN 99
#define MAX_LEXEME_LENGTH 99

int charClass;
char lexeme[MAX_LEXEME_LENGTH + 1];
int nextChar;
int lexLen;
int nextToken;
int lineNumber = 1;
int lexemeLine = 1;
/* Set by addChar() when it drops a character because the lexeme buffer is
 * full. scanNextToken() resets it for every token. */
int lexemeTooLong = 0;
FILE *in_fp;

static void addChar(void);
static void getChar(void);
/* Provided for your lookahead: non-static so the starter itself compiles
 * warning-free before you use it. */
int peekChar(void);
static void getNonBlank(void);
static int lookup(int ch);
static int keywordToken(const char *word);
/* Provided error reporter: prints the exact format the grader expects. */
static int lexError(const char *message);
int lex(void);
const char *tokenName(int tokenCode);

static int lookup(int ch) {
    switch (ch) {
        case '(':
            addChar();
            nextToken = LEFT_PAREN;
            break;
        case ')':
            addChar();
            nextToken = RIGHT_PAREN;
            break;
        case '+':
        if (peekChar() == '+') {
                addChar();
                getChar();
                addChar();
                nextToken = INC_OP;
            } else if (peekChar() == '=') {
                addChar();
                getChar();
                addChar();
                nextToken = ADD_ASSIGN;
            } else {
                addChar();
                nextToken = ADD_OP;
            }
            break;
        case '-':
            addChar();
            if (peekChar() == '-') {
                getChar();
                addChar();
                nextToken = DEC_OP;
            } else if (peekChar() == '=') {
                getChar();
                addChar();
                nextToken = SUB_ASSIGN;
            } else {
                nextToken = SUB_OP;
            }
            break;
        case '*':
            addChar();
            if (peekChar() == '=') {
                getChar();
                addChar();
                nextToken = MUL_ASSIGN;
            } else {
                nextToken = MULT_OP;
            }
            break;
        case '/':
            addChar();
            if (peekChar() == '=') {
                getChar();
                addChar();
                nextToken = DIV_ASSIGN;
            } else {
                nextToken = DIV_OP;
            }
            break;

        /* TODO: Add the remaining single-character tokens: { } ; , : %.
         * TODO: Add the two-character operators. Each needs one character
         *   of lookahead with peekChar(): '==' vs '=', '!=' vs '!' (a lone
         *   '!' is NOT_OP), '<=' vs '<', '>=' vs '>', '&&' (a lone '&' is
         *   an illegal character), '||' (a lone '|' is illegal), '++',
         *   '--', '+=', '-=', '*=', '/='. Only call getChar() for
         *   characters that belong to the token; scanNextToken() calls
         *   getChar() once after lookup() returns.
         *   (A '-' that starts a negative number never reaches lookup():
         *   that decision is made in scanNextToken(), not here. '/' is also
         *   handled there, where comments are skipped.) */

        case '{':
            addChar();
            nextToken = LEFT_BRACE;
            break;
        case '}':
            addChar();
            nextToken = RIGHT_BRACE;
            break;
        case ';':
            addChar();
            nextToken = SEMICOLON;
            break;
        case ',':
            addChar();
            nextToken = COMMA;
            break;
        case ':':
            addChar();
            nextToken = COLON;
            break;
        case '%':
            addChar();
            nextToken = MOD_OP;
            break;
        case '=':
            addChar();
            if (peekChar() == '=') {
                getChar();
                addChar();
                nextToken = EQ_OP;
            } else {
                nextToken = ASSIGN_OP;
            }
            break;
        case '!':
            addChar();
            if (peekChar() == '=') {
                getChar();
                addChar();
                nextToken = NE_OP;
            } else {
                nextToken = NOT_OP;
            }
            break;
        case '<':
            addChar();
            if (peekChar() == '=') {
                getChar();
                addChar();
                nextToken = LE_OP;
            } else {
                nextToken = LT_OP;
            }
            break;
        case '>':
            addChar();
            if (peekChar() == '=') {
                getChar();
                addChar();
                nextToken = GE_OP;
            } else {
                nextToken = GT_OP;
            }
            break;
        case '&':
            addChar();
            if (peekChar() == '&') {
                getChar();
                addChar();
                nextToken = AND_OP;
            } else {
                nextToken = lexError("illegal character");
            }
            break;
        case '|':
            addChar();
            if (peekChar() == '|') {
                getChar();
                addChar();
                nextToken = OR_OP;
            } else {
                nextToken = lexError("illegal character");
            }
            break;
        default:
            /* Example of reporting an error: consume the character, then
             * call lexError() with the matching message from the handout's
             * error table. */
            addChar();
            nextToken = lexError("illegal character");
            break;
    }
    return nextToken;
}

/*
 * lexError - report a lexical error on stderr in the exact format the
 * grader expects:
 *     Line <line number>: <message>: <lexeme>
 * Pass one of the messages from the handout's error table, e.g.
 * lexError("illegal character"). Returns LEX_ERROR. Scanning continues
 * from the next unconsumed character.
 */
static int lexError(const char *message) {
    fprintf(stderr, "Line %d: %s: %s\n", lexemeLine, message, lexeme);
    return LEX_ERROR;
}

static int keywordToken(const char *word) {
    /* TODO: Return the fixed token for each keyword: int, float, void,
     * char, double, const, main, return, if, else, for, while, do, break,
     * continue, switch, case, default. Anything else is IDENT. */
    (void)word;
    if (strcmp(word, "int") == 0) {
        return KW_INT;
    }
    if (strcmp(word, "float") == 0) {
        return KW_FLOAT;
    }
    if (strcmp(word, "void") == 0) {
        return KW_VOID;
    }
    if (strcmp(word, "char") == 0) {
        return KW_CHAR;
    }
    if (strcmp(word, "double") == 0) {
        return KW_DOUBLE;
    }
    if (strcmp(word, "const") == 0) {
        return KW_CONST;
    }
    if (strcmp(word, "main") == 0) {
        return KW_MAIN;
    }
    if (strcmp(word, "return") == 0) {
        return KW_RETURN;
    }
    if (strcmp(word, "if") == 0) {
        return KW_IF;
    }
    if (strcmp(word, "else") == 0) {
        return KW_ELSE;
    }
    if (strcmp(word, "for") == 0) {
        return KW_FOR;
    }
    if (strcmp(word, "while") == 0) {
        return KW_WHILE;
    }
    if (strcmp(word, "do") == 0) {
        return KW_DO;
    }
    if (strcmp(word, "break") == 0) {
        return KW_BREAK;
    }
    if (strcmp(word, "continue") == 0) {
        return KW_CONTINUE;
    }
    if (strcmp(word, "switch") == 0) {
        return KW_SWITCH;
    }
    if (strcmp(word, "case") == 0) {
        return KW_CASE;
    }
    if (strcmp(word, "default") == 0) {
        return KW_DEFAULT;
    }
    return IDENT;
}

static void addChar(void) {
    if (lexLen < MAX_LEXEME_LENGTH) {
        lexeme[lexLen++] = (char)nextChar;
        lexeme[lexLen] = '\0';
    } else {
        /* Buffer is full: discard this character and flag the overflow
         * so the scanner can report it after consuming the whole lexeme. */
        lexemeTooLong = 1;
    }
}

static void getChar(void) {
    if (nextChar == '\n') {
        lineNumber++;
    }

    nextChar = getc(in_fp);
    if (nextChar == EOF) {
        charClass = EOF;
    } else if (isalpha((unsigned char)nextChar)) {
        charClass = LETTER;
    } else if (isdigit((unsigned char)nextChar)) {
        charClass = DIGIT;
    } else {
        charClass = UNKNOWN;
    }
}

static void getNonBlank(void) {
    while (nextChar != EOF && isspace((unsigned char)nextChar)) {
        getChar();
    }
}

/*
 * peekChar - look at the next input character without consuming it.
 * Unlike getChar(), it leaves nextChar, charClass, and the line count
 * untouched. Use it when one character of lookahead is needed.
 */
int peekChar(void) {
    int c = getc(in_fp);
    if (c != EOF) {
        ungetc(c, in_fp);
    }
    return c;
}

 /* 
 * scanCharLit - scans a character literal.
 * Returns the token code for a character literal (CHAR_LIT) if successful, 
 * or calls lexError() with the appropriate message if there is an error.
 */
static int scanCharLit(void) {
    addChar(); 
    getChar();
    if (nextChar == '\\') { 
        addChar();
        getChar();
        if (nextChar != EOF) {
            addChar();
            getChar();
        } else {
            return lexError("unterminated character literal");
        }
    } else if (nextChar != '\'' && nextChar != EOF) {
        addChar();
        getChar();
    } else {
        return lexError("empty character literal");
    }

    if (nextChar == '\'') {
        addChar(); 
        getChar();
        return CHAR_LIT;
    } else {
        return lexError("unterminated character literal");
    }
}

/*
scanNumber - scans a numeric literal, which may be an integer or a floating-point number.
hasDot: indicates whether a decimal point has already been encountered (1 for yes, 0 for no).
Returns the token code for an integer literal (INT_LIT) or a floating-point
literal (FLOAT_LIT) if successful, or calls lexError() with the appropriate message if there is an error.
*/
   static int scanNumber(int hasDot) {
       while (charClass == DIGIT) { 
        addChar(); 
        getChar(); 
    }
       if (!hasDot && nextChar == '.') {
           addChar(); getChar();
           hasDot = 1;
           if (charClass != DIGIT)
               return lexError("malformed floating-point literal");
           while (charClass == DIGIT) { addChar(); getChar(); }
       }
       if (nextChar == '.') {
           while (charClass == DIGIT || nextChar == '.') { addChar(); getChar(); }
           return lexError("too many decimal points in number");
       }
       if (lexemeTooLong)
           return lexError("lexeme is longer than 99 characters");
       return hasDot ? FLOAT_LIT : INT_LIT;
   }


/*
 * scanString - scans a string literal.
 * Returns the token code for a string literal (STRING_LIT) if successful,
 * or calls lexError() with the appropriate message if there is an error.
 */
static int scanString(void) {
    addChar(); 
    getChar();
    while (nextChar != '"' && nextChar != EOF) {
        if (nextChar == '\\') { 
            addChar();
            getChar();
            if (nextChar != EOF) {
                addChar();
                getChar();
            }
        } else {
            addChar();
            getChar();
        }
    }
    if (nextChar == '"') {
        addChar(); 
        getChar();
        return STRING_LIT;
    } else {
        return lexError("unterminated string literal");
    }
}
static int scanNextToken(void) {
    /* The loop lets us skip things that produce no tokens (comments and
     * preprocessor directives) and keep scanning for the next real token.
     * Use "continue;" after skipping one of them. */
    for (;;) {
        lexLen = 0;
        lexeme[0] = '\0';
        lexemeTooLong = 0;
        getNonBlank();
        lexemeLine = lineNumber;

        /* TODO: Skip preprocessor directives: when nextChar is '#', read
         * to the end of the line (as with // comments) and continue.
         * A line like #include <stdio.h> then produces no tokens. */

        if (nextChar == '#') {
            /* Skip the rest of the line */
            while (nextChar != '\n' && nextChar != EOF) {
                getChar();
            }
            continue;
        }

        /* TODO: Handle '/': peek with peekChar(). "//" starts a line
         * comment and a slash followed by a star starts a block comment;
         * skip each without producing a token and continue. An
         * unterminated block comment is one error: set the lexeme to "/"
         * "*" (the two characters, no space) and return
         * lexError("unterminated block comment"). A '/' followed by '='
         * is DIV_ASSIGN; any other '/' is DIV_OP (lookup() already
         * handles the plain case, so only intercept the comment and '/='
         * cases here). */
        if (nextChar == '/') {
            int peek = peekChar();
            if (peek == '/') {
                // line comment 
                while (nextChar != '\n' && nextChar != EOF) {
                    getChar();
                }
                continue;
            } else if (peek == '*') {
                // block comment 
                getChar(); 
                getChar(); 
                while (1) {
                    if (nextChar == EOF) {
                        strcpy(lexeme, "/*");
                        return lexError("unterminated block comment");
                    } else if (nextChar == '*') {
                        getChar();
                        if (nextChar == '/') {
                            getChar(); 
                            break;
                        }
                    } else {
                        getChar();
                    }
                }
                continue;
            } else if (peek == '=') {
                addChar(); 
                getChar(); 
                addChar();
                nextToken = DIV_ASSIGN;
                return nextToken;
            } else {
                lookup(nextChar);
                getChar();
                return nextToken;
            }
        }

        /* TODO: A '-' immediately followed by a digit starts a negative
         * numeric literal (-1 is one INT_LIT, -1.5 is one FLOAT_LIT), and
         * a '-' followed by '.' plus a digit starts one too (-.7 is one
         * FLOAT_LIT): addChar() the characters that belong to the number
         * and scan it exactly like the DIGIT case (use the shared helper
         * below). Otherwise let lookup() choose SUB_OP, DEC_OP (--),
         * or SUB_ASSIGN (-=). Assign the helper result to nextToken. */

        if (nextChar == '-') {
            int peek = peekChar();
            if (isdigit(peek)) {
                addChar(); 
                getChar(); 
                nextToken = scanNumber(1);
                return nextToken;
            } else if (peek == '.') {
                addChar(); 
                getChar(); 
                peek = peekChar();
                if (isdigit(peek)) {
                    addChar(); 
                    getChar(); 
                    nextToken = scanNumber(1);
                    return nextToken;
                } else {
                    lookup(nextChar);
                    getChar();
                    return nextToken;
                }
            } else {
                lookup(nextChar);
                getChar();
                return nextToken;
            }
        }

        /* TODO: A '.' immediately followed by a digit starts a floating
         * literal (.5 is one FLOAT_LIT): scan it with the same helper.
         * Any other '.' is an illegal character (lookup() default). */

        if (nextChar == '.') {
            int peek = peekChar();
            if (isdigit(peek)) {
                addChar();
                getChar();
                nextToken = scanNumber(0);
                return nextToken;
            } else {
                lookup(nextChar);
                getChar();
                return nextToken;
            }
        }

        /* TODO: A '"' starts a string literal and '\'' starts a character
         * literal. Write helpers scanString() and scanCharLit() that
         * handle backslash escapes and report the handout's errors for
         * unterminated, empty, and invalid literals, then dispatch to
         * them here, assign their result to nextToken, and return it (do NOT call
         * getChar() afterwards: the helper leaves nextChar on the first
         * unconsumed character, just like the number helper). */

        if (nextChar == '"') {
            nextToken = scanString();
            return nextToken;
        }
        if (nextChar == '\'') {
            nextToken = scanCharLit();
            return nextToken;
        }



        switch (charClass) {
            case LETTER:
                /* TODO: Classify '_' as LETTER in getChar() so it can
                 * start or continue an identifier in this same loop. */
                if (nextChar == '_') {
                    addChar();
                    getChar();
                }
                addChar();
                getChar();
                while (charClass == LETTER || charClass == DIGIT) {
                    addChar();
                    getChar();
                }
                /* TODO: If lexemeTooLong is set, the identifier ran past 99
                 * characters. It has been fully consumed and the buffer holds
                 * the first 99: return lexError("lexeme is longer than 99 characters"). */
                if (lexemeTooLong) {
                    return lexError("lexeme is longer than 99 characters");
                }
                nextToken = keywordToken(lexeme);
                return nextToken;

            case DIGIT:
                addChar();
                getChar();
                while (charClass == DIGIT) {
                    addChar();
                    getChar();
                }
                /* TODO: Recognize FLOAT_LIT and malformed numbers. Write a
                 * helper scanNumber() used here, for negative literals,
                 * and for leading-dot literals: it consumes an optional
                 * fraction ".digits" (digits.digits is one FLOAT_LIT; a
                 * dot with no digit after it, as in 7., is one error:
                 * lexError("malformed floating-point literal")); a second
                 * dot, as in 1.2.3, is one error after consuming the whole
                 * dotted run: lexError("too many decimal points in number").
                 * Also check lexemeTooLong for over-long numbers, as above.
                 * The helper must leave nextChar on the first unconsumed
                 * character. Assign its result to nextToken and return it.
                 * A first dot without a digit takes priority: 1..2 is
                 * an error for 1., followed by FLOAT_LIT(.2). */
                nextToken = scanNumber(0);
                return nextToken;
            case UNKNOWN:
                lookup(nextChar);
                getChar();
                return nextToken;

            case EOF:
                nextToken = TOK_EOF;
                strcpy(lexeme, "EOF");
                return nextToken;

            default:
                strcpy(lexeme, "?");
                return lexError("illegal character");
        }
    }
}

/* Keep the returned token and public state consistent, including errors. */
int lex(void) {
    nextToken = scanNextToken();
    return nextToken;
}

const char *tokenName(int tokenCode) {
    switch (tokenCode) {
        case TOK_EOF: return "TOK_EOF";
        case INT_LIT: return "INT_LIT";
        case IDENT: return "IDENT";
        case ASSIGN_OP: return "ASSIGN_OP";
        case ADD_OP: return "ADD_OP";
        case SUB_OP: return "SUB_OP";
        case MULT_OP: return "MULT_OP";
        case DIV_OP: return "DIV_OP";
        case LEFT_PAREN: return "LEFT_PAREN";
        case RIGHT_PAREN: return "RIGHT_PAREN";
        case LEFT_BRACE: return "LEFT_BRACE";
        case RIGHT_BRACE: return "RIGHT_BRACE";
        case SEMICOLON: return "SEMICOLON";
        case COMMA: return "COMMA";
        case KW_INT: return "KW_INT";
        case KW_VOID: return "KW_VOID";
        case KW_MAIN: return "KW_MAIN";
        case KW_RETURN: return "KW_RETURN";
        case FLOAT_LIT: return "FLOAT_LIT";
        case EQ_OP: return "EQ_OP";
        case NE_OP: return "NE_OP";
        case KW_FLOAT: return "KW_FLOAT";
        case KW_IF: return "KW_IF";
        case KW_ELSE: return "KW_ELSE";
        case KW_FOR: return "KW_FOR";
        case KW_WHILE: return "KW_WHILE";
        case KW_DO: return "KW_DO";
        case KW_BREAK: return "KW_BREAK";
        case KW_CONTINUE: return "KW_CONTINUE";
        case KW_CHAR: return "KW_CHAR";
        case KW_DOUBLE: return "KW_DOUBLE";
        case KW_CONST: return "KW_CONST";
        case KW_SWITCH: return "KW_SWITCH";
        case KW_CASE: return "KW_CASE";
        case KW_DEFAULT: return "KW_DEFAULT";
        case LT_OP: return "LT_OP";
        case GT_OP: return "GT_OP";
        case LE_OP: return "LE_OP";
        case GE_OP: return "GE_OP";
        case NOT_OP: return "NOT_OP";
        case AND_OP: return "AND_OP";
        case OR_OP: return "OR_OP";
        case INC_OP: return "INC_OP";
        case DEC_OP: return "DEC_OP";
        case MOD_OP: return "MOD_OP";
        case ADD_ASSIGN: return "ADD_ASSIGN";
        case SUB_ASSIGN: return "SUB_ASSIGN";
        case MUL_ASSIGN: return "MUL_ASSIGN";
        case DIV_ASSIGN: return "DIV_ASSIGN";
        case CHAR_LIT: return "CHAR_LIT";
        case STRING_LIT: return "STRING_LIT";
        case COLON: return "COLON";
        case LEX_ERROR: return "LEX_ERROR";
        default: return "UNKNOWN_TOKEN";
    }
}

#ifndef SCANNER_NO_MAIN
int main(int argc, char *argv[]) {
    const char *inputName = argc > 1 ? argv[1] : "front.in";

    in_fp = fopen(inputName, "r");
    if (in_fp == NULL) {
        fprintf(stderr, "ERROR - cannot open %s\n", inputName);
        return 1;
    }

    nextChar = 0;
    getChar();
    do {
        int producedToken = lex();
        printf("Next token is: %d, Next lexeme is %s\n", producedToken, lexeme);
    } while (nextToken != TOK_EOF);

    fclose(in_fp);
    return 0;
}
#endif
