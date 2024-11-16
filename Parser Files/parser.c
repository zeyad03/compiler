#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "lexer.h"
#include "parser.h"

// Global variables.
Token t;
ParserInfo pi;

// Function Prototypes.
void error(char *msg, SyntaxErrors e);
void classDeclar();
void memberDeclar();
void classVarDeclar();
void type();
void subroutineDeclar();
void paramList();
void subroutineBody();
void statement();
void varDeclarStatement();
void letStatement();
void ifStatement();
void whileStatement();
void doStatement();
void subroutineCall();
void expressionList();
void returnStatement();
void expression();
void relationalExpression();
void arithmeticExpression();
void term();
void factor();
void operand();


int InitParser (char* file_name)
{
	return InitLexer(file_name);
}

ParserInfo Parse ()
{
	pi.er = none;
	classDeclar();
	return pi;
}

void classDeclar()
{
	// Consume "class"
	t = GetNextToken();
	if (strcmp(t.lx, "class") || t.tp != RESWORD)
    {
		error("keyword \"class\" expected", classExpected);
		return;
	}

    // Consume identifier
	t = GetNextToken();
	if (t.tp != ID)
        error("identifier expected", idExpected);
	if (pi.er) return;
    
    // Consume "{"
	t = GetNextToken();
	if (strcmp(t.lx, "{"))
		error("{ expected", openBraceExpected);
	if (pi.er) return;
    
	// Check for other member declarations
	t = PeekNextToken();
	while (!strcmp(t.lx, "static") || !strcmp(t.lx, "field") ||
		!strcmp(t.lx, "constructor") || !strcmp(t.lx, "function") ||
		!strcmp(t.lx, "method"))
    {
		memberDeclar();
		if (pi.er) return;

		t = PeekNextToken();
	}

	// Consume "}"
	t = GetNextToken();
	if (strcmp(t.lx, "}"))
		error("} expected", closeBraceExpected);
	if (pi.er) return;
}

void memberDeclar()
{
	t = PeekNextToken();
	if (!strcmp(t.lx, "static") || !strcmp(t.lx, "field")) {
		classVarDeclar();
    } else if (!strcmp(t.lx, "constructor") || !strcmp(t.lx, "function") || !strcmp(t.lx, "method")) {
		subroutineDeclar();
    } else {
        t = PeekNextToken();
        if (t.tp == ID) {
            error("class varibale declaration error", classVarErr);
        } else if (t.tp == RESWORD) {
            error("subroutine declaration error", subroutineDeclarErr);
        } else {
            error("class member declaration error", memberDeclarErr);
        }
		return;
	}
}

void classVarDeclar()
{
    // Consume "static" or "field"
	t = GetNextToken();
    // Consume type
	type();	
	if (pi.er) return;
    
    // Consume identifier
	t = GetNextToken();
	if (t.tp != ID)
        error("identifier expected", idExpected);
    
	// Check for more identifiers
	t = PeekNextToken();
	while (!strcmp(t.lx, ",")) {
        // Consume the token
		GetNextToken();
        // Consume identifier
		t = GetNextToken();
        if (t.tp != ID)
            error("identifier expected", idExpected);
		if (pi.er) return;
		t = PeekNextToken();
	}

	// Consume semi-colon
	t = GetNextToken();
	if (strcmp(t.lx, ";"))
		error("; expected", semicolonExpected);
	if (pi.er) return;
}

void type()
{
	t = GetNextToken();
	if ((strcmp(t.lx, "int") && strcmp(t.lx, "char") &&
		strcmp(t.lx, "boolean") && t.tp != ID))
		error("illegal type", illegalType);
}

void subroutineDeclar()
{
    // Consume reserved word and identifier
	t = GetNextToken();
	t = GetNextToken();
	// void or type
	if (strcmp(t.lx, "int") && strcmp(t.lx, "char") && 
		strcmp(t.lx, "boolean") && strcmp(t.lx, "void") && t.tp != ID)
	{
		error("expected void or type", syntaxError);
		return;
	}

	t = GetNextToken();
	if (t.tp != ID)
        error("identifier expected", idExpected);
	if (pi.er) return;
    
	t = GetNextToken();
	if (strcmp(t.lx, "("))
		error("( expected", openParenExpected);
	if (pi.er) return;
    
	paramList();
	if (pi.er) return;
    
	t = GetNextToken();
	if (strcmp(t.lx, ")"))
		error(") expected", closeParenExpected);
	if (pi.er) return;
    
	subroutineBody();
	if (pi.er) return;
}

void paramList()
{
	t = PeekNextToken();
	if (strcmp(t.lx, "int") && strcmp(t.lx, "char") && 
		strcmp(t.lx, "boolean") && t.tp != ID )
		return;
    
	type();
	if (pi.er) return;
    
	t = GetNextToken();
	if (t.tp != ID)
        error("identifier expected", idExpected);
	if (pi.er) return;

	t = PeekNextToken();	
	// Until we hit a close parenthesis
	while (!strcmp(t.lx, ","))
    {
		GetNextToken();

		type();
		if (pi.er) return;

		t = GetNextToken();
	    if (t.tp != ID)
            error("identifier expected", idExpected);
		if (pi.er) return;

		t = PeekNextToken();
	}
}

void subroutineBody()
{
	t = GetNextToken();
	if (strcmp(t.lx, "{"))
		error("{ expected", openBraceExpected);
	if (pi.er) return;

	// Statements
	t = PeekNextToken();
	while (!strcmp(t.lx, "var") || !strcmp(t.lx, "let") || !strcmp(t.lx, "if") ||
		!strcmp(t.lx, "while") || !strcmp(t.lx, "do") || !strcmp(t.lx, "return"))
    {
        statement();
		if (pi.er) return;
		t = PeekNextToken();
	}

	t = GetNextToken();
	if (strcmp(t.lx, "}"))
		error("} expected", closeBraceExpected);
	if (pi.er) return;
}

void statement()
{
	t = PeekNextToken();
	if (!strcmp(t.lx, "var")) {
        t = GetNextToken();
		varDeclarStatement();
	} else if (!strcmp(t.lx, "let")) {
        t = GetNextToken();
		letStatement();
	} else if (!strcmp(t.lx, "if")) {
        t = GetNextToken();
		ifStatement();
	} else if (!strcmp(t.lx, "while")) {
        t = GetNextToken();
		whileStatement();
	} else if (!strcmp(t.lx, "do")) {
        t = GetNextToken();
		doStatement();
	} else if (!strcmp(t.lx, "return")) {
        t = GetNextToken();
		returnStatement();
	} else {
		error( "Expected statement", syntaxError );
		return;
	}
}

void varDeclarStatement()
{
	type();
	if (pi.er) return;

	t = GetNextToken();
	if (t.tp != ID)
        error("identifier expected", idExpected);
	if (pi.er) return;

	// None or more other identfiers exist
	t = PeekNextToken();
	while (!strcmp(t.lx, ","))
    {
		GetNextToken();

		t = GetNextToken();
        if (t.tp != ID)
            error("identifier expected", idExpected);
		if (pi.er) return;

		t = PeekNextToken();
	}

	t = GetNextToken();
	if (strcmp(t.lx, ";"))
		error("; expected", semicolonExpected);
	if (pi.er) return;
}

void letStatement()
{
	t = GetNextToken();
	if (t.tp != ID)
        error("identifier expected", idExpected);
	if (pi.er) return;

	// None or one more expression exist
	t = PeekNextToken();
	if (!strcmp(t.lx, "[")) {
		GetNextToken();

		expression();
		if (pi.er) return;

		t = GetNextToken();
		if (strcmp(t.lx, "]")) {
			error("] expected", closeBracketExpected);
			return;
		}
	}
    
	t = GetNextToken();
	if (strcmp(t.lx, "=")) {
		error("= expected", equalExpected);
		return;
	}

	expression();
	if (pi.er) return;
    
	t = GetNextToken();
	if (strcmp(t.lx, ";"))
		error("; expected", semicolonExpected);
	if (pi.er) return;
}

void ifStatement()
{
	t = GetNextToken();
	if (strcmp(t.lx, "("))
		error("( expected", openParenExpected);
	if (pi.er) return;
    
	expression();
	if (pi.er) return;

	t = GetNextToken();
	if (strcmp(t.lx, ")"))
		error(") expected", closeParenExpected);
	if (pi.er) return;
    
	t = GetNextToken();
	if (strcmp(t.lx, "{"))
		error("{ expected", openBraceExpected);
	if (pi.er) return;

	// None or more statements exist
	t = PeekNextToken();
	while (!strcmp(t.lx, "var") || !strcmp(t.lx, "let") || !strcmp(t.lx, "if") ||
		!strcmp(t.lx, "while") || !strcmp(t.lx, "do") || !strcmp(t.lx, "return"))
    {
		statement();
		if (pi.er) return;
		t = PeekNextToken();
	}
    
	t = GetNextToken();
	if (strcmp(t.lx, "}"))
		error("} expected", closeBraceExpected);
	if (pi.er) return;

	t = PeekNextToken();
	if (!strcmp(t.lx, "else")) {
		GetNextToken();

		t = GetNextToken();
        if (strcmp(t.lx, "{"))
            error("{ expected", openBraceExpected);
		if (pi.er) return;
        
		t = PeekNextToken();
		while (!strcmp(t.lx, "var") || !strcmp(t.lx, "let") || !strcmp(t.lx, "if") ||
			!strcmp(t.lx, "while") ||  !strcmp(t.lx, "do") || !strcmp(t.lx, "return"))
        {
			statement();
			if (pi.er) return;

			t = PeekNextToken();
		}

		t = GetNextToken();
        if (strcmp(t.lx, "}"))
            error("} expected", closeBraceExpected);
		if (pi.er)
			return;
	}
}

void whileStatement()
{
	t = GetNextToken();
	if (strcmp(t.lx, "("))
		error("( expected", openParenExpected);
	if (pi.er) return;

	expression();
	if (pi.er) return;

	t = GetNextToken();
	if (strcmp(t.lx, ")"))
		error(") expected", closeParenExpected);
	if (pi.er) return;

	t = GetNextToken();
	if (strcmp(t.lx, "{"))
		error("{ expected", openBraceExpected);
	if (pi.er) return;

	t = PeekNextToken();
	while (!strcmp(t.lx, "var") || !strcmp(t.lx, "let") || !strcmp(t.lx, "if") ||
		!strcmp(t.lx, "while") || !strcmp(t.lx, "do") || !strcmp(t.lx, "return"))
    {
		statement();
		if (pi.er) return;

		t = PeekNextToken();
	}

	t = GetNextToken();
	if (strcmp(t.lx, "}"))
		error("} expected", closeBraceExpected);
	if (pi.er) return;
}

void doStatement()
{
	subroutineCall();
	if (pi.er) return;

    t = GetNextToken();
	if (strcmp(t.lx, ";"))
		error("; expected", semicolonExpected);
	if (pi.er) return;
}

void returnStatement()
{
	if (strcmp(t.lx, "return")) {
		error( "'return' keyword expected", syntaxError );
		return;
	}

	// 0 or 1 expressions
	t = PeekNextToken();
	if (!strcmp(t.lx, "-") || !strcmp(t.lx, "~") || 
		!strcmp(t.lx, "(") || !strcmp(t.lx, "true") ||
		!strcmp(t.lx, "false") || !strcmp(t.lx, "null") ||
		!strcmp(t.lx, "this") || t.tp == INT || t.tp == ID || t.tp == STRING)
    {
		expression();
		if (pi.er) return;
	}

    t = GetNextToken();
	if (strcmp(t.lx, ";"))
		error("; expected", semicolonExpected);
	if (pi.er) return;
}

void subroutineCall()
{
	t = GetNextToken();
	if (t.tp != ID)
        error("identifier expected", idExpected);
	if (pi.er) return;

	t = PeekNextToken();
	if (!strcmp(t.lx, "."))
    {
        // Consume "." and get next token
		GetNextToken();
		t = GetNextToken();
        if (t.tp != ID)
            error("identifier expected", idExpected);
		if (pi.er) return;
	}

	t = GetNextToken();
	if (strcmp(t.lx, "("))
		error("( expected", openParenExpected);
	if (pi.er) return;

	expressionList();
	if (pi.er) return;

	t = GetNextToken();
	if (strcmp(t.lx, ")"))
		error(") expected", closeParenExpected);
	if (pi.er) return;
}

void expressionList()
{
	// None or start of expression
	t = PeekNextToken();
	if (strcmp(t.lx, "-") && strcmp(t.lx, "~") &&
		strcmp(t.lx, "(") && strcmp(t.lx, "true") &&
		strcmp(t.lx, "false") && strcmp(t.lx, "null") &&
		strcmp(t.lx, "this")  && t.tp != INT && 
		t.tp != ID && t.tp != STRING )
		return;

	expression();
	if (pi.er) return;

	t = PeekNextToken();
	while (!strcmp(t.lx, ","))
    {
		GetNextToken();

		expression();
		if (pi.er) return;

		t = PeekNextToken();
	}
}

void expression()
{
	relationalExpression();
	if (pi.er) return;

	// None or more (& or |) exists followed by relational expression
	t = PeekNextToken();
	while (!strcmp(t.lx, "&") || !strcmp(t.lx, "|"))
    {
		GetNextToken();
		relationalExpression();
		if (pi.er) return;

		t = PeekNextToken();
	}
}

void relationalExpression()
{
	arithmeticExpression();
	if (pi.er) return;

	// None or more (= or < or >) followed by an arithmetic expression
	t = PeekNextToken();
	while (!strcmp(t.lx, "=") || !strcmp(t.lx, ">") || !strcmp(t.lx, "<"))
    {
		GetNextToken();
	
		arithmeticExpression();
		if (pi.er) return;

		t = PeekNextToken();
	}
}

void arithmeticExpression()
{
	term();
	if (pi.er) return;

	// None or more (+ or -) followed by an arithmetic expression
	t = PeekNextToken();
	while (!strcmp(t.lx, "-") || !strcmp(t.lx, "+"))
    {
		GetNextToken();
		term();
		if (pi.er) return;

		t = PeekNextToken();
	}
}

void term()
{
	factor();
	if (pi.er) return;

	// None or more (* or /) followed by a factor
	t = PeekNextToken();
	while ( !strcmp(t.lx, "*") || !strcmp(t.lx, "/") )
    {
		GetNextToken();
		factor();
		if (pi.er) return;

		t = PeekNextToken();
	}
}

void factor()
{
	// No expression or - or ~
	t = PeekNextToken();
	if (!strcmp(t.lx, "-") || !strcmp(t.lx, "~"))
		GetNextToken();

	operand();
	if (pi.er) return;
}

void operand()
{
	t = GetNextToken();
	if (t.tp == INT) return;

	// id [.id][ (expression) | (expressionList) ]
	if (t.tp == ID)
    {
		t = PeekNextToken();
		if (!strcmp(t.lx, "."))
        {
			GetNextToken();
			t = GetNextToken();
			if (t.tp != ID) {
				error("identifier expected", idExpected);
				return;
			}
		}

		t = PeekNextToken();
		if (!strcmp(t.lx, "["))
        {
			GetNextToken();
							
			expression();
			if (pi.er) return;
			
			t = GetNextToken();
			if (strcmp(t.lx, "]")) {
				error("] expected", closeBracketExpected);
				return;
			}
		} else if (!strcmp(t.lx, "(")) {
			GetNextToken();

			expressionList();
			if (pi.er) return;

			t = GetNextToken();
            if (strcmp(t.lx, ")"))
                error(") expected", closeParenExpected);
			if (pi.er) return;
		}
		return;
	}

	if (!strcmp(t.lx, "("))
    {
		expression();
		if (pi.er) return;

		t = GetNextToken();
        if (strcmp(t.lx, ")"))
            error(") expected", closeParenExpected);
		return;
	}

	if (t.tp == STRING) return;

	if (t.tp != RESWORD) {
		error("expected operand", syntaxError);
		return;
	}

	if (!strcmp(t.lx, "true") || !strcmp(t.lx, "false") ||
		!strcmp(t.lx, "null") || !strcmp(t.lx, "this"))
		return;
}

void error(char *msg, SyntaxErrors e)
{
	pi.tk = t;
	pi.er = e;
	if (t.tp == ERR) {
	 	printf("%s at line %i\n", t.lx, t.ln);
		pi.er = lexerErr;
	}
	else 
		printf("Error, line %i, close to \"%s\", %s.\n", t.ln, t.lx, msg);
}

int StopParser ()
{
	return StopLexer();
}

#ifndef TEST_PARSER
int main (void)
{
	InitParser("Main.jack");

    ParserInfo result = Parse();
    if (result.er != none) {
        printf("Parsing failed\n");
        return 1;
    }

    printf("Parsing successful\n");
    StopParser();
    return 0;
}
#endif