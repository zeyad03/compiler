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
ParserInfo varDeclarStatement();
ParserInfo letStatement();
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

int InitParser(char* file_name)
{
    InitLexer(file_name);
    currentToken = GetNextToken();
    return 1;
}

ParserInfo Parse()
{
    ParserInfo pi;
    pi.er = none;
    pi = classDeclar();
    return pi;
}

int StopParser()
{
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
                while (strcmp(currentToken.lx, "}")) {
                    currentToken = GetNextToken();
                    pi = memberDeclar();
                    if (pi.er != none) return pi;
                }
                if (!strcmp(currentToken.lx, "}"))
                {
                    currentToken = GetNextToken();
                } else {
                    pi.er = closeBraceExpected;
                    pi.tk = currentToken;
                    error("close brace expected", pi.er);
                }
            } else {
                pi.er = openBraceExpected;
                pi.tk = currentToken;
                error("open brace expected", pi.er);
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
    while (currentToken.tp == RESWORD)
    {
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
        currentToken = GetNextToken();
        if (!strcmp(currentToken.lx, ","))
        {
            while (strcmp(currentToken.lx, ";")) {
                currentToken = GetNextToken();
                if (currentToken.tp == ID) {
                    currentToken = GetNextToken();
                } else {
                    pi.er = idExpected;
                    pi.tk = currentToken;
                    error("142. id expected", pi.er);
                }
            }
            currentToken = GetNextToken();
        } else if (!strcmp(currentToken.lx, ";")) {
            currentToken = GetNextToken();
        } else {
            pi.er = semicolonExpected;
            pi.tk = currentToken;
            error("151. expected semi colon", pi.er);
        }
    } else {
        pi.er = idExpected;
        pi.tk = currentToken;
        error("156. syntax error", pi.er);
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
                    currentToken = GetNextToken();
                    currentToken = GetNextToken();
                }
                if (pi.er == none) {
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
    currentToken = GetNextToken();
    while (currentToken.tp != SYMBOL)
    {
        pi = type();
        currentToken = GetNextToken();
        if (pi.er != none) return pi;
        if (!strcmp(currentToken.lx, ")")) {
            break;
        } else if (!strcmp(currentToken.lx, ",")) {
            currentToken = GetNextToken();
        } else {
            pi.er = syntaxError;
            pi.tk = currentToken;
            error("218. syntax error", pi.er);
        }
    }
    if (!strcmp(currentToken.lx, ")"))
    {
        currentToken = GetNextToken();
    } else {
        pi.er = closeParenExpected;
        pi.tk = currentToken;
        error("235. close paren exptected error", pi.er);
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
    if (!strcmp(currentToken.lx, "{"))
    {
        currentToken = GetNextToken();
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
    if (currentToken.tp == RESWORD) {
        if (!strcmp(currentToken.lx, "var")) {
            pi = varDeclarStatement();
        } else if (!strcmp(currentToken.lx, "let")) {
            pi = letStatement();
        } else if (!strcmp(currentToken.lx, "if")) {
            pi = ifStmt();
        } else if (!strcmp(currentToken.lx, "while")) {
            pi = whileStmt();
        } else if (!strcmp(currentToken.lx, "do")) {
            pi = doStmt();
        } else if (!strcmp(currentToken.lx, "return")) {
            pi = returnStmt();
        }
    } else {
        pi.er = syntaxError;
        pi.tk = currentToken;
        error("292. syntax error", pi.er);
    }
    return pi;
}

ParserInfo varDeclarStatement()
{
    ParserInfo pi;
    pi.er = none;
    currentToken = GetNextToken();
    pi = type();
    if (pi.er != none) return pi;
    if (currentToken.tp == ID)
    {
        currentToken = GetNextToken();
        if (!strcmp(currentToken.lx, ","))
        {
            while (strcmp(currentToken.lx, ";")) {
                currentToken = GetNextToken();
                if (currentToken.tp == ID) {
                    currentToken = GetNextToken();
                } else {
                    pi.er = idExpected;
                    pi.tk = currentToken;
                    error("316. id expected", pi.er);
                }
            }
            currentToken = GetNextToken();
        } else if (!strcmp(currentToken.lx, ";")) {
            currentToken = GetNextToken();
        } else {
            pi.er = semicolonExpected;
            pi.tk = currentToken;
            error("325. expected semi colon", pi.er);
        }
    } else {
        pi.er = idExpected;
        pi.tk = currentToken;
        error("330. syntax error", pi.er);
    }
    return pi;
}

ParserInfo letStatement()
{
    ParserInfo pi;
    pi.er = none;
    currentToken = GetNextToken();
    if (currentToken.tp == ID)
    {
        currentToken = GetNextToken();
        if (!strcmp(currentToken.lx, "[")) {
            currentToken = GetNextToken();
            pi = expression();
            if (!strcmp(currentToken.lx, "]")) {
                currentToken = GetNextToken();
                if (!strcmp(currentToken.lx, "=")) {
                    currentToken = GetNextToken();
                    pi = expression();
                } else {
                    pi.er = equalExpected;
                    pi.tk = currentToken;
                    error("373. equal expected", pi.er);
                }
            } else {
                pi.er = closeBracketExpected;
                pi.tk = currentToken;
                error("378. close bracket expected", pi.er);
            }
        } else if (!strcmp(currentToken.lx, "=")) {
            currentToken = GetNextToken();
            pi = expression();
            if (!strcmp(currentToken.lx, ";")) {
                currentToken = GetNextToken();
            } else {
                pi.er = semicolonExpected;
                pi.tk = currentToken;
                error("389. semicolon expected", pi.er);
            }
        } else {
            pi.er = equalExpected;
            pi.tk = currentToken;
            error("394. equal expected", pi.er);
        }
    } else {
        pi.er = idExpected;
        pi.tk = currentToken;
        error("391. id expected", pi.er);
    }
    printf("token: %s\n", currentToken.lx);
    printf("error: %i\n\n", pi.er);
    return pi;
}

ParserInfo expression()
{
    ParserInfo pi;
    pi.er = none;
    pi = relationalExpression();
    if (pi.er != none) return pi;
    while (!strcmp(currentToken.lx, "&") || !strcmp(currentToken.lx, "|"))
    {
        currentToken = GetNextToken();
        pi = relationalExpression();
        if (pi.er != none) return pi;
    }
    return pi;
}

ParserInfo relationalExpression()
{
    ParserInfo pi;
    pi.er = none;
    pi = arithmeticExpression();
    if (pi.er != none) return pi;
    while (!strcmp(currentToken.lx, "=") || !strcmp(currentToken.lx, "<") || !strcmp(currentToken.lx, ">"))
    {
        currentToken = GetNextToken();
        pi = arithmeticExpression();
        if (pi.er != none) return pi;
    }
    return pi;
}

ParserInfo arithmeticExpression()
{
    ParserInfo pi;
    pi.er = none;
    pi = term();
    if (pi.er != none) return pi;
    while (!strcmp(currentToken.lx, "+") || !strcmp(currentToken.lx, "-"))
    {
        currentToken = GetNextToken();
        pi = term();
        if (pi.er != none) return pi;
    }
    return pi;
}

ParserInfo term()
{
    ParserInfo pi;
    pi.er = none;
    pi = factor();
    if (pi.er != none) return pi;
    while (!strcmp(currentToken.lx, "*") || !strcmp(currentToken.lx, "/"))
    {
        currentToken = GetNextToken();
        pi = factor();
        if (pi.er != none) return pi;
    }
    return pi;
}

ParserInfo factor()
{
    ParserInfo pi;
    pi.er = none;
    //currentToken = GetNextToken();
    if (!strcmp(currentToken.lx, "-") || !strcmp(currentToken.lx, "~"))
        currentToken = GetNextToken();
    pi = operand();
    return pi;
}

ParserInfo ifStmt() {
    ParserInfo pi;
    pi.er = none;
    currentToken = GetNextToken();
    if (!strcmp(currentToken.lx, "(")) {
        currentToken = GetNextToken();
        pi = expression();
        if (pi.er != none) return pi;
        if (!strcmp(currentToken.lx, ")")) {
            currentToken = GetNextToken();
            if (!strcmp(currentToken.lx, "{")) {
                currentToken = GetNextToken();
                while (currentToken.tp == RESWORD) {
                    pi = statement();
                    if (pi.er != none) return pi;
                }
                if (!strcmp(currentToken.lx, "}")) {
                    currentToken = GetNextToken();
                    if (!strcmp(currentToken.lx, "else")) {
                        currentToken = GetNextToken();
                        if (!strcmp(currentToken.lx, "{")) {
                            currentToken = GetNextToken();
                            while (currentToken.tp == RESWORD) {
                                pi = statement();
                                if (pi.er != none) return pi;
                            }
                            if (!strcmp(currentToken.lx, "}")) {
                                currentToken = GetNextToken();
                            } else {
                                pi.er = closeBraceExpected;
                                pi.tk = currentToken;
                                error("505. close brace expected", pi.er);
                            }
                        } else {
                            pi.er = openBraceExpected;
                            pi.tk = currentToken;
                            error("510. open brace expected", pi.er);
                        }
                    }
                } else {
                    pi.er = closeBraceExpected;
                    pi.tk = currentToken;
                    error("516. close brace expected", pi.er);
                }
            } else {
                pi.er = openBraceExpected;
                pi.tk = currentToken;
                error("521. open brace expected", pi.er);
            }
        } else {
            pi.er = closeParenExpected;
            pi.tk = currentToken;
            error("526. close paren expected", pi.er);
        }
    } else {
        pi.er = openParenExpected;
        pi.tk = currentToken;
        error("531. open paren expected", pi.er);
    }
    return pi;
}

ParserInfo whileStmt() {
    ParserInfo pi;
    pi.er = none;
    currentToken = GetNextToken();
    if (!strcmp(currentToken.lx, "(")) {
        currentToken = GetNextToken();
        pi = expression();
        if (pi.er != none) return pi;
        if (!strcmp(currentToken.lx, ")")) {
            currentToken = GetNextToken();
            if (!strcmp(currentToken.lx, "{")) {
                currentToken = GetNextToken();
                while (currentToken.tp == RESWORD) {
                    pi = statement();
                    if (pi.er != none) return pi;
                    currentToken = GetNextToken();
                }
                if (!strcmp(currentToken.lx, "}")) {
                    currentToken = GetNextToken();
                } else {
                    pi.er = closeBraceExpected;
                    pi.tk = currentToken;
                    error("557. close brace expected", pi.er);
                }
            } else {
                pi.er = openBraceExpected;
                pi.tk = currentToken;
                error("562. open brace expected", pi.er);
            }
        } else {
            pi.er = closeParenExpected;
            pi.tk = currentToken;
            error("567. close paren expected", pi.er);
        }
    } else {
        pi.er = openParenExpected;
        pi.tk = currentToken;
        error("572. open paren expected", pi.er);
    }
    return pi;
}

ParserInfo doStmt() {
    ParserInfo pi;
    pi.er = none;
    currentToken = GetNextToken();
    pi = subroutineCall();
    if (pi.er != none) return pi;
    if (!strcmp(currentToken.lx, ";")) {
        currentToken = GetNextToken();
    } else {
        pi.er = semicolonExpected;
        pi.tk = currentToken;
        error("588. semicolon expected", pi.er);
    }
    return pi;
}

ParserInfo subroutineCall() {
    ParserInfo pi;
    pi.er = none;
    if (currentToken.tp == ID) {
        currentToken = GetNextToken();
        if (!strcmp(currentToken.lx, ".")) {
            currentToken = GetNextToken();
            if (currentToken.tp == ID) {
                currentToken = GetNextToken();
                if (!strcmp(currentToken.lx, "(")) {
                    currentToken = GetNextToken();
                    pi = expressionList();
                    if (pi.er != none) return pi;
                    if (!strcmp(currentToken.lx, ")")) {
                        currentToken = GetNextToken();
                    } else {
                        pi.er = closeParenExpected;
                        pi.tk = currentToken;
                        error("611. close paren expected", pi.er);
                    }
                } else {
                    pi.er = openParenExpected;
                    pi.tk = currentToken;
                    error("616. open paren expected", pi.er);
                }
            } else {
                pi.er = idExpected;
                pi.tk = currentToken;
                error("621. id expected", pi.er);
            }
        } else if (!strcmp(currentToken.lx, "(")) {
            currentToken = GetNextToken();
            pi = expressionList();
            if (pi.er != none) return pi;
            if (!strcmp(currentToken.lx, ")")) {
                currentToken = GetNextToken();
            } else {
                pi.er = closeParenExpected;
                pi.tk = currentToken;
                error("632. close paren expected", pi.er);
            }
        } else {
            pi.er = syntaxError;
            pi.tk = currentToken;
            error("637. syntax error", pi.er);
        }
    } else {
        pi.er = idExpected;
        pi.tk = currentToken;
        error("642. id expected", pi.er);
    }
    return pi;
}

ParserInfo expressionList() {
    ParserInfo pi;
    pi.er = none;
    if (currentToken.tp != SYMBOL || !strcmp(currentToken.lx, ")")) {
        return pi;
    }
    pi = expression();
    if (pi.er != none) return pi;
    while (!strcmp(currentToken.lx, ",")) {
        currentToken = GetNextToken();
        pi = expression();
        if (pi.er != none) return pi;
    }
    return pi;
}

ParserInfo returnStmt() {
    ParserInfo pi;
    pi.er = none;
    currentToken = GetNextToken();
    if (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, ";")) {
        currentToken = GetNextToken();
    } else {
        pi = expression();
        if (pi.er != none) return pi;
        if (!strcmp(currentToken.lx, ";")) {
            currentToken = GetNextToken();
        } else {
            pi.er = semicolonExpected;
            pi.tk = currentToken;
            error("677. semicolon expected", pi.er);
        }
    }
    return pi;
}

ParserInfo operand() {
    ParserInfo pi;
    pi.er = none;
    if (!strcmp(currentToken.lx, "."))
        currentToken = GetNextToken();
    if (currentToken.tp == INT) {
        currentToken = GetNextToken();
    } else if (currentToken.tp == STRING) {
        currentToken = GetNextToken();
    } else if (!strcmp(currentToken.lx, "true")) {
        currentToken = GetNextToken();
    } else if (!strcmp(currentToken.lx, "false")) {
        currentToken = GetNextToken();
    } else if (!strcmp(currentToken.lx, "null")) {
        currentToken = GetNextToken();
    } else if (!strcmp(currentToken.lx, "this")) {
        currentToken = GetNextToken();
    } else if (currentToken.tp == ID) {
        currentToken = GetNextToken();
        if (!strcmp(currentToken.lx, "[")) {
            currentToken = GetNextToken();
            pi = expression();
            if (pi.er != none) return pi;
            if (!strcmp(currentToken.lx, "]")) {
                currentToken = GetNextToken();
            } else {
                pi.er = closeBracketExpected;
                pi.tk = currentToken;
                error("709. close bracket expected", pi.er);
            }
        } else if (!strcmp(currentToken.lx, ".")) {
            currentToken = GetNextToken();
            if (currentToken.tp == ID) {
                currentToken = GetNextToken();
                if (!strcmp(currentToken.lx, "(")) {
                    currentToken = GetNextToken();
                    pi = expressionList();
                    if (pi.er != none) return pi;
                    currentToken = GetNextToken();
                    if (!strcmp(currentToken.lx, ")")) {
                        currentToken = GetNextToken();
                    } else {
                        pi.er = closeParenExpected;
                        pi.tk = currentToken;
                        error("724. close paren expected", pi.er);
                    }
                } else {
                    pi.er = openParenExpected;
                    pi.tk = currentToken;
                    error("729. open paren expected", pi.er);
                }
            } else {
                pi.er = idExpected;
                pi.tk = currentToken;
                error("734. id expected", pi.er);
            }
        }
    } else if (!strcmp(currentToken.lx, "(")) {
        currentToken = GetNextToken();
        pi = expression();
        if (pi.er != none) return pi;
        if (!strcmp(currentToken.lx, ")")) {
            currentToken = GetNextToken();
        } else {
            pi.er = closeParenExpected;
            pi.tk = currentToken;
            error("746. close paren expected", pi.er);
        }
    } else {
        pi.er = syntaxError;
        pi.tk = currentToken;
        error("751. syntax error", pi.er);
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

//#ifndef TEST_PARSER
int main() {
    InitParser("Main.jack");
    ParserInfo result = Parse();
    if (result.er != none) {
        printf("Parsing failed\n");
    } else {
        printf("Parsing successful\n");
    }
    StopParser();
    return 0;
}
//#endif
