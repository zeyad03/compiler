#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "lexer.h"
#include "parser.h"

Token currentToken;

// Function prototypes
void error(char *msg, SyntaxErrors errorCode);
ParserInfo classDeclar();
ParserInfo memberDeclar();
ParserInfo classVarDeclar();
ParserInfo type();
ParserInfo subroutineDeclar();
ParserInfo paramList();
ParserInfo subroutineBody();
ParserInfo statement();
ParserInfo varDeclStmt();
ParserInfo letStmt();
ParserInfo ifStmt();
ParserInfo whileStmt();
ParserInfo doStmt();
ParserInfo subroutineCall();
ParserInfo expressionList();
ParserInfo returnStmt();
ParserInfo expression();
ParserInfo relationalExpression();
ParserInfo arithmeticExpression();
ParserInfo term();
ParserInfo factor();
ParserInfo operand();

int InitParser(char* file_name) {
    InitLexer(file_name);
    currentToken = GetNextToken();
    return 1;
}

ParserInfo Parse() {
    ParserInfo pi;
    pi.er = none;
    pi = classDeclar();
    return pi;
}

int StopParser() {
    StopLexer();
    return 1;
}

ParserInfo classDeclar()
{
    ParserInfo pi;
    if (currentToken.tp == RESWORD && !strcmp(currentToken.lx, "class"))
    {
        currentToken = GetNextToken();
        if (currentToken.tp == ID)
        {
            currentToken = GetNextToken();
            if (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, "{"))
            {
                currentToken = GetNextToken();
                pi = memberDeclar();
                if (pi.er != none) return pi;
            } else {
                pi.er = openBraceExpected;
                pi.tk = currentToken;
                error("open brace expected", pi.er);
            }
            currentToken = GetNextToken();
            if (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, "}"))
            {
                currentToken = GetNextToken();
            } else {
                pi.er = closeBraceExpected;
                pi.tk = currentToken;
                error("close brace expected", pi.er);
            }
        } else {
            pi.er = idExpected;
            pi.tk = currentToken;
            error("id expected", pi.er);
        }
    } else {
        pi.er = classExpected;
        pi.tk = currentToken;
        error("keyword class expected", pi.er);
    }
    return pi;
}

ParserInfo memberDeclar()
{
    ParserInfo pi;
    pi.er = none;
    if (currentToken.tp == RESWORD && (!strcmp(currentToken.lx, "static") || !strcmp(currentToken.lx, "field")))
    {
        pi = classVarDeclar();
    } else if (currentToken.tp == RESWORD && (!strcmp(currentToken.lx, "constructor") ||!strcmp(currentToken.lx, "function") || !strcmp(currentToken.lx, "method"))) {
        pi = subroutineDeclar();
    } else {
        Token nextToken = PeekNextToken();
        if (nextToken.tp == ID)
        {
            pi.er = classVarErr;
            pi.tk = currentToken;
            error("109. class varibale declaration error", pi.er);
        } else if (nextToken.tp == RESWORD) {
            pi.er = subroutineDeclarErr;
            pi.tk = currentToken;
            error("113. class varibale declaration error", pi.er);
        } else {
            pi.er = memberDeclarErr;
            pi.tk = currentToken;
            error("117. invalid member declaration", pi.er);
        }
    }
    return pi;
}

ParserInfo classVarDeclar()
{
    ParserInfo pi;
    pi.er = none;
    currentToken = GetNextToken();
    pi = type();
    if (pi.er != none) return pi;
    if (currentToken.tp == ID)
    {
        while (currentToken.tp == ID)
        {
            currentToken = GetNextToken();
            if (!strcmp(currentToken.lx, ",")) {
                currentToken = GetNextToken();
            }
        }
    } else {
        pi.er = idExpected;
        pi.tk = currentToken;
        error("141. id expected", pi.er);
    }
    if (!strcmp(currentToken.lx, ";")) {
        currentToken = GetNextToken();
    } else {
        pi.er = semicolonExpected;
        pi.tk = currentToken;
        error("148. expected semicolon", pi.er);
    }
    return pi;
}

ParserInfo subroutineDeclar()
{
    ParserInfo pi;
    pi.er = none;
    Token nextToken;
    currentToken = GetNextToken();
    pi = type();
    if (pi.er != none) pi.er = none;
    if (pi.er == none || !strcmp(currentToken.lx, "void"))
    {
        if (!strcmp(currentToken.lx, "void"))
            currentToken = GetNextToken();
        if (currentToken.tp == ID)
        {
            currentToken = GetNextToken();
            if (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, "("))
            {
                nextToken = PeekNextToken();
                if (strcmp(nextToken.lx, ")")) {
                    pi = paramList();
                } else {
                    currentToken = nextToken;
                }
                if (strcmp(currentToken.lx, ")")) {
                    pi.er = closeParenExpected;
                    pi.tk = currentToken;
                    error("176. close paren exptected", pi.er);
                }
                if (pi.er == none) {
                    currentToken = GetNextToken();
                    pi = subroutineBody();
                }
            } else {
                pi.er = openParenExpected;
                pi.tk = currentToken;
                error("186. expected open paran", pi.er);
            }
        } else {
            pi.er = idExpected;
            pi.tk = currentToken;
            error("191. id expteded", pi.er);
        }
    } else {
        pi.er = subroutineDeclarErr;
        pi.tk = currentToken;
        error("196. subroutine declaration error", pi.er);
    }
    return pi;
}

ParserInfo paramList()
{
    ParserInfo pi;
    pi.er = none;
    while (1)
    {
        currentToken = GetNextToken();
        pi = type();
        if (pi.er != none) return pi;
        if (!strcmp(currentToken.lx, ")")) {
            currentToken = GetNextToken();
            break;
        } else if (!strcmp(currentToken.lx, ",")) {
            currentToken = GetNextToken();
        } else {
            pi.er = syntaxError;
            pi.tk = currentToken;
            error("218. syntax error", pi.er);
        }
    }
    return pi;
}

ParserInfo type()
{
    ParserInfo pi;
    pi.er = none;
    if ((currentToken.tp == RESWORD && 
        (!strcmp(currentToken.lx, "int") || !strcmp(currentToken.lx, "char") 
        || !strcmp(currentToken.lx, "boolean"))) || currentToken.tp == ID) {
        currentToken = GetNextToken();
    } else if (!strcmp(currentToken.lx, "void")) {
        return pi;
    } else {
        pi.er = illegalType;
        pi.tk = currentToken;
        error("235. illegal type", pi.er);
    }
    return pi;
}

ParserInfo subroutineBody()
{
    ParserInfo pi;
    pi.er = none;
    currentToken = GetNextToken();
    if (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, "{"))
    {
        while (currentToken.tp == RESWORD) {
			pi = statement();
            if (pi.er != none) {
                return pi;
            }
        }
        if (!strcmp(currentToken.lx, "}")) {
            currentToken = GetNextToken();
        } else {
            pi.er = closeBraceExpected;
            pi.tk = currentToken;
            error("262. expected close brace", pi.er);
        }
    } else {
        pi.er = openBraceExpected;
        pi.tk = currentToken;
        error("267. expected open brace", pi.er);
    }
    return pi;
}

ParserInfo statement()
{
    ParserInfo pi;
    pi.er = none;
    if (currentToken.tp == RESWORD && !strcmp(currentToken.lx, "var")) {
        //pi = varDeclStmt();
    } else if (currentToken.tp == RESWORD && !strcmp(currentToken.lx, "let")) {
        //pi = letStmt();
    } else if (currentToken.tp == RESWORD && !strcmp(currentToken.lx, "if")) {
        //pi = ifStmt();
    } else if (currentToken.tp == RESWORD && !strcmp(currentToken.lx, "while")) {
        //pi = whileStmt();
    } else if (currentToken.tp == RESWORD && !strcmp(currentToken.lx, "do")) {
        //pi = doStmt();
    } else if (currentToken.tp == RESWORD && !strcmp(currentToken.lx, "return")) {
        //pi = returnStmt();
    } else {
        pi.er = syntaxError;
        pi.tk = currentToken;
    }
    return pi;
}

void error(char *msg, SyntaxErrors errorCode) {
    printf("\n---------- ERROR ----------\n");
    printf("\nParser Error: %s\n", msg);
    printf("Line: %d, Lexeme: %s\n", currentToken.ln, currentToken.lx);
    printf("Error code: %i\n", errorCode);
    printf("\n---------------------------\n\n");
    return;
}

