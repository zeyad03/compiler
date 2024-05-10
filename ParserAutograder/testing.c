#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "lexer.h"
#include "parser.h"

Token currentToken;

// Function prototypes
int error(char *msg, SyntaxErrors errorCode);
ParserInfo classDecl();
ParserInfo memberDecl();
ParserInfo classVarDecl();
ParserInfo type();
ParserInfo subroutineDecl();
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
    pi = classDecl();
    return pi;
}

int StopParser() {
    StopLexer();
    return 1;
}

ParserInfo classDecl() {
    ParserInfo pi;
    if (currentToken.tp == RESWORD && !strcmp(currentToken.lx, "class")) {
        currentToken = GetNextToken(); // Consume "class"
        pi = type(); // Parse class name (identifier)
        if (pi.er == none) {
            currentToken = GetNextToken(); // Consume class name
            if (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, "{")) {
                currentToken = GetNextToken(); // Consume "{"
                while (currentToken.tp == RESWORD && (!strcmp(currentToken.lx, "static") || !strcmp(currentToken.lx, "field") || !strcmp(currentToken.lx, "constructor") || !strcmp(currentToken.lx, "function") || !strcmp(currentToken.lx, "method"))) {
                    pi = memberDecl();
                    if (pi.er != none) {
                        return pi;
                    }
                }
                if (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, "}")) {
                    currentToken = GetNextToken(); // Consume "}"
                } else {
					pi.er = openBraceExpected;
					pi.tk = currentToken;
					if (strstr(pi.tk.lx, "lexer error") != NULL) {
						error("lexer error", pi.er);
					} else {
						error("{ expected", pi.er);
					}
                }
            }
        } else if (strstr(pi.tk.lx, "lexer error") != NULL) {
            error(pi.tk.lx, pi.er);
        }
    } else {
        pi.er = classExpected;
        pi.tk = currentToken;
        error("keyword class expected", pi.er);
    }
    return pi;
}

ParserInfo memberDecl() {
    ParserInfo pi;
    if (currentToken.tp == RESWORD && (!strcmp(currentToken.lx, "static") || !strcmp(currentToken.lx, "field"))) {
        pi = classVarDecl();
    } else if (currentToken.tp == RESWORD && (!strcmp(currentToken.lx, "constructor") || !strcmp(currentToken.lx, "function") || !strcmp(currentToken.lx, "method"))) {
        pi = subroutineDecl();
    } else {
        pi.er = memberDeclarErr;
        pi.tk = currentToken;
        error("Invalid member declaration", pi.er);
    }
    return pi;
}

ParserInfo classVarDecl() {
    ParserInfo pi;
    pi.er = none;
    if (currentToken.tp == RESWORD && (!strcmp(currentToken.lx, "static") || !strcmp(currentToken.lx, "field"))) {
        currentToken = GetNextToken(); // Consume "static" or "field"
        pi = type();
        if (pi.er == none) {
            pi = type(); // Parse variable type
            if (pi.er == none) {
                currentToken = GetNextToken(); // Consume variable type
                if (currentToken.tp == ID) {
                    currentToken = GetNextToken(); // Consume variable name
                    while (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, ",")) {
                        currentToken = GetNextToken(); // Consume ","
                        if (currentToken.tp != ID) {
                            pi.er = idExpected;
                            pi.tk = currentToken;
                            return pi;
                        }
                        currentToken = GetNextToken(); // Consume variable name
                    }
                    if (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, ";")) {
                        currentToken = GetNextToken(); // Consume ";"
                    } else {
                        pi.er = semicolonExpected;
                        pi.tk = currentToken;
                    }
                } else {
                    pi.er = idExpected;
                    pi.tk = currentToken;
                }
            }
        }
    } else {
        pi.er = classVarErr;
        pi.tk = currentToken;
    }
    return pi;
}

ParserInfo type() {
    ParserInfo pi;
    pi.er = none;
    if (currentToken.tp == RESWORD && (!strcmp(currentToken.lx, "int") || !strcmp(currentToken.lx, "char") || !strcmp(currentToken.lx, "boolean"))) {
        currentToken = GetNextToken(); // Consume type keyword
    } else if (currentToken.tp == ID) {
        currentToken = GetNextToken(); // Consume identifier
    } else {
        pi.er = illegalType;
        pi.tk = currentToken;
    }
    return pi;
}

ParserInfo subroutineDecl() {
    ParserInfo pi;
    pi.er = none;
    if (currentToken.tp == RESWORD && (!strcmp(currentToken.lx, "constructor") || !strcmp(currentToken.lx, "function") || !strcmp(currentToken.lx, "method"))) {
        currentToken = GetNextToken(); // Consume "constructor", "function", or "method"
        pi = type(); // Parse return type
        if (pi.er == none) {
            currentToken = GetNextToken(); // Consume return type
            if (currentToken.tp == ID) {
                currentToken = GetNextToken(); // Consume subroutine name
                if (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, "(")) {
                    currentToken = GetNextToken(); // Consume "("
                    pi = paramList();
                    if (pi.er == none) {
                        if (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, ")")) {
                            currentToken = GetNextToken(); // Consume ")"
                            pi = subroutineBody();
                        } else {
                            pi.er = closeParenExpected;
                            pi.tk = currentToken;
                            error(") expected", pi.er);
                        }
                    }
                } else {
                    pi.er = openParenExpected;
                    pi.tk = currentToken;
                    error("( expected", pi.er);
                }
            } else {
                pi.er = idExpected;
                pi.tk = currentToken;
                error("identifier expected", pi.er);
            }
        }
    } else {
        pi.er = subroutineDeclarErr;
        pi.tk = currentToken;
        error("Invalid subroutine declaration", pi.er);
    }
    return pi;
}

ParserInfo paramList() {
    ParserInfo pi;
    pi.er = none;
    if (currentToken.tp != SYMBOL || strcmp(currentToken.lx, ")")) {
        pi = type(); // Parse parameter type
        if (pi.er == none) {
            currentToken = GetNextToken(); // Consume parameter type
            if (currentToken.tp != ID) {
                pi.er = idExpected;
                pi.tk = currentToken;
                return pi;
            }
            currentToken = GetNextToken(); // Consume parameter name
            while (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, ",")) {
                currentToken = GetNextToken(); // Consume ","
                pi = type(); // Parse next parameter type
                if (pi.er != none) {
                    return pi;
                }
                currentToken = GetNextToken(); // Consume parameter type
                if (currentToken.tp != ID) {
                    pi.er = idExpected;
                    pi.tk = currentToken;
                    return pi;
                }
                currentToken = GetNextToken(); // Consume parameter name
            }
        }
    }
    return pi;
}

ParserInfo subroutineBody() {
    ParserInfo pi;
    pi.er = none;
    if (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, "{")) {
        currentToken = GetNextToken(); // Consume "{"
        while (currentToken.tp == RESWORD && (!strcmp(currentToken.lx, "var") || !strcmp(currentToken.lx, "let") || !strcmp(currentToken.lx, "if") || !strcmp(currentToken.lx, "while") || !strcmp(currentToken.lx, "do") || !strcmp(currentToken.lx, "return"))) {
            pi = statement();
            if (pi.er != none) {
                return pi;
            }
        }
        if (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, "}")) {
            currentToken = GetNextToken(); // Consume "}"
        } else {
            pi.er = closeBraceExpected;
            pi.tk = currentToken;
        }
    } else {
        pi.er = openBraceExpected;
        pi.tk = currentToken;
    }
    return pi;
}

ParserInfo statement() {
    ParserInfo pi;
    pi.er = none;
    if (currentToken.tp == RESWORD && !strcmp(currentToken.lx, "var")) {
        pi = varDeclStmt();
    } else if (currentToken.tp == RESWORD && !strcmp(currentToken.lx, "let")) {
        pi = letStmt();
    } else if (currentToken.tp == RESWORD && !strcmp(currentToken.lx, "if")) {
        pi = ifStmt();
    } else if (currentToken.tp == RESWORD && !strcmp(currentToken.lx, "while")) {
        pi = whileStmt();
    } else if (currentToken.tp == RESWORD && !strcmp(currentToken.lx, "do")) {
        pi = doStmt();
    } else if (currentToken.tp == RESWORD && !strcmp(currentToken.lx, "return")) {
        pi = returnStmt();
    } else {
        pi.er = syntaxError;
        pi.tk = currentToken;
    }
    return pi;
}

ParserInfo varDeclStmt() {
    ParserInfo pi;
    pi.er = none;
    if (currentToken.tp == RESWORD && !strcmp(currentToken.lx, "var")) {
        currentToken = GetNextToken(); // Consume "var"
        pi = type(); // Parse variable type
        if (pi.er == none) {
            currentToken = GetNextToken(); // Consume variable type
            if (currentToken.tp != ID) {
                pi.er = idExpected;
                pi.tk = currentToken;
                return pi;
            }
            currentToken = GetNextToken(); // Consume variable name
            while (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, ",")) {
                currentToken = GetNextToken(); // Consume ","
                if (currentToken.tp != ID) {
                    pi.er = idExpected;
                    pi.tk = currentToken;
                    return pi;
                }
                currentToken = GetNextToken(); // Consume variable name
            }
            if (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, ";")) {
                currentToken = GetNextToken(); // Consume ";"
            } else {
                pi.er = semicolonExpected;
                pi.tk = currentToken;
            }
        }
    } else {
        pi.er = syntaxError;
        pi.tk = currentToken;
    }
    return pi;
}

ParserInfo letStmt() {
    ParserInfo pi;
    pi.er = none;
    if (currentToken.tp == RESWORD && !strcmp(currentToken.lx, "let")) {
        currentToken = GetNextToken(); // Consume "let"
        if (currentToken.tp != ID) {
            pi.er = idExpected;
            pi.tk = currentToken;
            error("identifier expected", pi.er);
            return pi;
        }
        currentToken = GetNextToken(); // Consume variable name
        if (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, "[")) {
            currentToken = GetNextToken(); // Consume "["
            pi = expression();
            if (pi.er != none) {
                return pi;
            }
            if (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, "]")) {
                currentToken = GetNextToken(); // Consume "]"
            } else {
                pi.er = closeBracketExpected;
                pi.tk = currentToken;
                error("] expected", pi.er);
                return pi;
            }
        }
        if (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, "=")) {
            currentToken = GetNextToken(); // Consume "="
        } else {
            pi.er = equalExpected;
            pi.tk = currentToken;
            error("= expected", pi.er);
            return pi;
        }
        pi = expression();
        if (pi.er == none) {
            if (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, ";")) {
                currentToken = GetNextToken(); // Consume ";"
            } else {
                pi.er = semicolonExpected;
                pi.tk = currentToken;
                error("; expected", pi.er);
            }
        }
    } else {
        pi.er = syntaxError;
        pi.tk = currentToken;
        error("Invalid let statement", pi.er);
    }
    return pi;
}

ParserInfo ifStmt() {
    ParserInfo pi;
    pi.er = none;
    if (currentToken.tp == RESWORD && !strcmp(currentToken.lx, "if")) {
        currentToken = GetNextToken(); // Consume "if"
        if (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, "(")) {
            currentToken = GetNextToken(); // Consume "("
            pi = expression();
            if (pi.er == none) {
                if (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, ")")) {
                    currentToken = GetNextToken(); // Consume ")"
                    if (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, "{")) {
                        currentToken = GetNextToken(); // Consume "{"
                        pi = subroutineBody();
                        if (pi.er == none) {
                            if (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, "}")) {
                                currentToken = GetNextToken(); // Consume "}"
                                if (currentToken.tp == RESWORD && !strcmp(currentToken.lx, "else")) {
                                    currentToken = GetNextToken(); // Consume "else"
                                    if (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, "{")) {
                                        currentToken = GetNextToken(); // Consume "{"
                                        pi = subroutineBody();
                                        if (pi.er == none) {
                                            if (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, "}")) {
                                                currentToken = GetNextToken(); // Consume "}"
                                            } else {
                                                pi.er = closeBraceExpected;
                                                pi.tk = currentToken;
                                            }
                                        }
                                    } else {
                                        pi.er = openBraceExpected;
                                        pi.tk = currentToken;
                                    }
                                }
                            } else {
                                pi.er = closeBraceExpected;
								pi.tk = currentToken;
                            }
                        }
                    } else {
                        pi.er = openBraceExpected;
                        pi.tk = currentToken;
                    }
                } else {
                    pi.er = closeParenExpected;
                    pi.tk = currentToken;
                }
            }
        } else {
            pi.er = openParenExpected;
            pi.tk = currentToken;
        }
    } else {
        pi.er = syntaxError;
        pi.tk = currentToken;
    }
    return pi;
}

ParserInfo whileStmt() {
    ParserInfo pi;
    pi.er = none;
    if (currentToken.tp == RESWORD && !strcmp(currentToken.lx, "while")) {
        currentToken = GetNextToken(); // Consume "while"
        if (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, "(")) {
            currentToken = GetNextToken(); // Consume "("
            pi = expression();
            if (pi.er == none) {
                if (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, ")")) {
                    currentToken = GetNextToken(); // Consume ")"
                    if (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, "{")) {
                        currentToken = GetNextToken(); // Consume "{"
                        pi = subroutineBody();
                        if (pi.er == none) {
                            if (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, "}")) {
                                currentToken = GetNextToken(); // Consume "}"
                            } else {
                                pi.er = closeBraceExpected;
                                pi.tk = currentToken;
                            }
                        }
                    } else {
                        pi.er = openBraceExpected;
                        pi.tk = currentToken;
                    }
                } else {
                    pi.er = closeParenExpected;
                    pi.tk = currentToken;
                }
            }
        } else {
            pi.er = openParenExpected;
            pi.tk = currentToken;
        }
    } else {
        pi.er = syntaxError;
        pi.tk = currentToken;
    }
    return pi;
}

ParserInfo doStmt() {
    ParserInfo pi;
    pi.er = none;
    if (currentToken.tp == RESWORD && !strcmp(currentToken.lx, "do")) {
        currentToken = GetNextToken(); // Consume "do"
        pi = subroutineCall();
        if (pi.er == none) {
            if (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, ";")) {
                currentToken = GetNextToken(); // Consume ";"
            } else {
                pi.er = semicolonExpected;
                pi.tk = currentToken;
            }
        }
    } else {
        pi.er = syntaxError;
        pi.tk = currentToken;
    }
    return pi;
}

ParserInfo subroutineCall() {
    ParserInfo pi;
    pi.er = none;
    if (currentToken.tp == ID) {
        currentToken = GetNextToken(); // Consume subroutine name
        if (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, ".")) {
            currentToken = GetNextToken(); // Consume "."
            if (currentToken.tp != ID) {
                pi.er = idExpected;
                pi.tk = currentToken;
                return pi;
            }
            currentToken = GetNextToken(); // Consume object name
        }
        if (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, "(")) {
            currentToken = GetNextToken(); // Consume "("
            pi = expressionList();
            if (pi.er == none) {
                if (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, ")")) {
                    currentToken = GetNextToken(); // Consume ")"
                } else {
                    pi.er = closeParenExpected;
                    pi.tk = currentToken;
                }
            }
        } else {
            pi.er = openParenExpected;
            pi.tk = currentToken;
        }
    } else {
        pi.er = idExpected;
        pi.tk = currentToken;
    }
    return pi;
}

ParserInfo expressionList() {
    ParserInfo pi;
    pi.er = none;
    if (currentToken.tp != SYMBOL || strcmp(currentToken.lx, ")")) {
        pi = expression();
        if (pi.er != none) {
            return pi;
        }
        while (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, ",")) {
            currentToken = GetNextToken(); // Consume ","
            pi = expression();
            if (pi.er != none) {
                return pi;
            }
        }
    }
    return pi;
}

ParserInfo returnStmt() {
    ParserInfo pi;
    pi.er = none;
    if (currentToken.tp == RESWORD && !strcmp(currentToken.lx, "return")) {
        currentToken = GetNextToken(); // Consume "return"
        if (currentToken.tp != SYMBOL || strcmp(currentToken.lx, ";")) {
            pi = expression();
            if (pi.er != none) {
                return pi;
            }
        }
        if (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, ";")) {
            currentToken = GetNextToken(); // Consume ";"
        } else {
            pi.er = semicolonExpected;
            pi.tk = currentToken;
        }
    } else {
        pi.er = syntaxError;
        pi.tk = currentToken;
    }
    return pi;
}

ParserInfo expression() {
    ParserInfo pi;
    pi.er = none;
    pi = relationalExpression();
    if (pi.er != none) {
        return pi;
    }
    while (currentToken.tp == SYMBOL && (!strcmp(currentToken.lx, "&") || !strcmp(currentToken.lx, "|"))) {
        currentToken = GetNextToken(); // Consume "&" or "|"
        pi = relationalExpression();
        if (pi.er != none) {
            return pi;
        }
    }
    return pi;
}

ParserInfo relationalExpression() {
    ParserInfo pi;
    pi.er = none;
    pi = arithmeticExpression();
    if (pi.er != none) {
        return pi;
    }
    while (currentToken.tp == SYMBOL && (!strcmp(currentToken.lx, "=") || !strcmp(currentToken.lx, ">") || !strcmp(currentToken.lx, "<"))) {
        currentToken = GetNextToken(); // Consume "=", ">", or "<"
        pi = arithmeticExpression();
        if (pi.er != none) {
            return pi;
        }
    }
    return pi;
}

ParserInfo arithmeticExpression() {
    ParserInfo pi;
    pi.er = none;
    pi = term();
    if (pi.er != none) {
        return pi;
    }
    while (currentToken.tp == SYMBOL && (!strcmp(currentToken.lx, "+") || !strcmp(currentToken.lx, "-"))) {
        currentToken = GetNextToken(); // Consume "+" or "-"
        pi = term();
        if (pi.er != none) {
            return pi;
        }
    }
    return pi;
}

ParserInfo term() {
    ParserInfo pi;
    pi.er = none;
    pi = factor();
    if (pi.er != none) {
        return pi;
    }
    while (currentToken.tp == SYMBOL && (!strcmp(currentToken.lx, "*") || !strcmp(currentToken.lx, "/"))) {
        currentToken = GetNextToken(); // Consume "*" or "/"
        pi = factor();
        if (pi.er != none) {
            return pi;
        }
    }
    return pi;
}

ParserInfo factor() {
    ParserInfo pi;
    pi.er = none;
    if (currentToken.tp == SYMBOL && (!strcmp(currentToken.lx, "-") || !strcmp(currentToken.lx, "~"))) {
        currentToken = GetNextToken(); // Consume "-" or "~"
        pi = operand();
    } else {
        pi = operand();
    }
    return pi;
}

ParserInfo operand() {
    ParserInfo pi;
    pi.er = none;
    if (currentToken.tp == INT) {
        currentToken = GetNextToken(); // Consume integer constant
    } else if (currentToken.tp == STRING) {
        currentToken = GetNextToken(); // Consume string constant
    } else if (currentToken.tp == RESWORD && (!strcmp(currentToken.lx, "true") || !strcmp(currentToken.lx, "false") || !strcmp(currentToken.lx, "null") || !strcmp(currentToken.lx, "this"))) {
        currentToken = GetNextToken(); // Consume keyword constant
    } else if (currentToken.tp == ID) {
        currentToken = GetNextToken(); // Consume identifier
        if (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, "[")) {
            currentToken = GetNextToken(); // Consume "["
            pi = expression();
            if (pi.er != none) {
                return pi;
            }
            if (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, "]")) {
                currentToken = GetNextToken(); // Consume "]"
            } else {
                pi.er = closeBracketExpected;
                pi.tk = currentToken;
                return pi;
            }
        } else if (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, "(")) {
            currentToken = GetNextToken(); // Consume "("
            pi = expressionList();
            if (pi.er != none) {
                return pi;
            }
            if (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, ")")) {
                currentToken = GetNextToken(); // Consume ")"
            } else {
                pi.er = closeParenExpected;
                pi.tk = currentToken;
                return pi;
            }
        } else if (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, ".")) {
            currentToken = GetNextToken(); // Consume "."
            if (currentToken.tp != ID) {
                pi.er = idExpected;
                pi.tk = currentToken;
                return pi;
            }
            currentToken = GetNextToken(); // Consume identifier
        } else {
            // No array access, subroutine call, or object access
        }
    } else if (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, "(")) {
        currentToken = GetNextToken(); // Consume "("
        pi = expression();
        if (pi.er != none) {
            return pi;
        }
        if (currentToken.tp == SYMBOL && !strcmp(currentToken.lx, ")")) {
            currentToken = GetNextToken(); // Consume ")"
        } else {
            pi.er = closeParenExpected;
            pi.tk = currentToken;
            return pi;
        }
    } else {
        pi.er = syntaxError;
        pi.tk = currentToken;
    }
    return pi;
}

int error(char *msg, SyntaxErrors errorCode) {
    if (strcmp(msg, "lexer error") == 0) {
        printf("Lexer Error: %s\n", msg);
        printf("Line: %d, Lexeme: %s\n", currentToken.ln, currentToken.lx);
    } else {
        printf("Parser Error: %s\n", msg);
        printf("Line: %d, Lexeme: %s\n", currentToken.ln, currentToken.lx);
    }
    return(errorCode);
}

#ifndef TEST_PARSER
int main() {
    InitParser("memberDeclarErr.jack");
    ParserInfo result = Parse();
    if (result.er != none) {
        error("Parsing failed", result.er);
    } else {
        printf("Parsing successful\n");
    }
    StopParser();
    return 0;
}
#endif