/************************************************************************
University of Leeds
School of Computing
COMP2932- Compiler Design and Construction
Lexer Module

I confirm that the following code has been developed and written by me and it is entirely the result of my own work.
I also confirm that I have not copied any parts of this program from another person or any other source or facilitated someone to copy this program from me.
I confirm that I will not publish the program online or share it with anyone without permission of the module leader.

Student Name: Zeyad Bassyouni
Student ID: 201649438
Email: sc22zb@leeds.ac.uk
Date Work Commenced: 25/03/2024
*************************************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

#define MAX_TOKEN_LENGTH 128
#define MAX_LINE_LENGTH 1024

// YOU CAN ADD YOUR OWN FUNCTIONS, DECLARATIONS AND VARIABLES HERE
char *filename;
FILE *file_stream;
int line_number = 1;

char *reserved_words[] = {
  "class", "constructor", "method", "function", "int", "boolean", "char", "void",
  "var", "static", "field", "let", "do", "if", "else", "while", "return", "true",
  "false", "null", "this", NULL
};

char *symbols = "()[]{},;=.+-*/&|~><";

void skip_whitespace()
{
  int c;
  while ((c = fgetc(file_stream)) != EOF)
  {
    if (isspace(c)) {
      if (c == '\n') line_number++;
    } else {
      ungetc(c, file_stream);
      break;
    }
  }
}

void handle_comment(Token *t, int c)
{
  if (c == '/') {
    // Single-line comment.
    c = fgetc(file_stream);
    while (c != '\n' && c != EOF) {
      c = fgetc(file_stream);
    }
    line_number += (c == '\n');
  } else if (c == '*') {
    // Multi-line comment.
    int prev_c = 0;
    while ((c = fgetc(file_stream)) != EOF) {
      if (c == '\n') {
        line_number++;
      } else if (prev_c == '*' && c == '/') {
        break;
      }
      prev_c = c;
    }
    if (c == EOF) {
      // Unterminated comment error.
      t->tp = ERR;
      t->ec = EofInCom;
      strcpy(t->lx, "Error: unexpected eof in comment");
      return;
    }
  }
}

// IMPLEMENT THE FOLLOWING functions
//***********************************

// Initialise the lexer to read from source file
// file_name is the name of the source file
// This requires opening the file and making any necessary initialisations of the lexer
// If an error occurs, the function should return 0
// if everything goes well the function should return 1
int InitLexer (char* file_name)
{
  filename = file_name;
  file_stream = fopen(file_name, "r");
  if (file_stream == NULL) {
    printf("Error: unable to open file '%s'\n", file_name);
    return 0;
  }
  return 1;
}

// Get the next token from the source file
Token GetNextToken() {
  Token t;
  t.lx[0] = '\0';
  skip_whitespace();

  int c = fgetc(file_stream);
  int next_c = fgetc(file_stream);
  if (c == EOF) {
    t.tp = EOFile;
    strcpy(t.lx, "End of File");
    t.ln = line_number;
    strcpy(t.fl, filename);
    return t;
  } else if (c == '/' && (next_c == '/' || next_c == '*')) {
    handle_comment(&t, next_c);
    if (t.tp == ERR) {
      t.ln = line_number;
      strcpy(t.fl, filename);
      return t;
    }
    return GetNextToken();
  } else {
    ungetc(next_c, file_stream);
  }

  // Handle identifiers, keywords, and integers
  if (isalpha(c) || c == '_') {
    int i = 0;
    while (isalnum(c) || c == '_') {
      if (i < MAX_TOKEN_LENGTH - 1) {
        t.lx[i++] = c;
      } else {
        // Identifier too long error
        t.tp = ERR;
        t.ec = IllSym;
        t.ln = line_number;
        strcpy(t.fl, filename);
        strcpy(t.lx, "Error: Identifier too long");
        return t;
      }
      c = fgetc(file_stream);
    }
    ungetc(c, file_stream);
    t.lx[i] = '\0';

    // Check if it's a reserved word
    for (char **word = reserved_words; *word != NULL; word++) {
      if (strcmp(t.lx, *word) == 0) {
        t.tp = RESWORD;
        t.ln = line_number;
        strcpy(t.fl, filename);
        return t;
      }
    }
    t.tp = ID;
    t.ln = line_number;
    strcpy(t.fl, filename);
    return t;
  } else if (isdigit(c)) {
    int i = 0;
    while (isdigit(c)) {
      if (i < MAX_TOKEN_LENGTH - 1) {
        t.lx[i++] = c;
      } else {
        // Number too long error
        t.tp = ERR;
        t.ec = IllSym;
        strcpy(t.lx, "Error: Number too long");
        t.ln = line_number;
        strcpy(t.fl, filename);
        return t;
      }
      c = fgetc(file_stream);
    }
    ungetc(c, file_stream);
    t.lx[i] = '\0';
    t.tp = INT;
    t.ln = line_number;
    strcpy(t.fl, filename);
    return t;
  } else if (c == '"') {
    // Handle string literals
    int i = 0;
    c = fgetc(file_stream);
    if (c == '\n') {
      // New line in string error
      t.tp = ERR;
      t.ec = NewLnInStr;
      t.ln = line_number;
      strcpy(t.fl, filename);
      strcpy(t.lx, "Error: new line in string constant");
      return t;
    }
    while (c != '"' && c != EOF) {
      if (i < MAX_TOKEN_LENGTH - 1) {
        t.lx[i++] = c;
      } else {
        // String too long error
        t.tp = ERR;
        t.ec = IllSym;
        t.ln = line_number;
        strcpy(t.fl, filename);
        strcpy(t.lx, "Error: String too long");
        return t;
      }
      c = fgetc(file_stream);
      if (c == '\n') {
        // New line in string error
        t.tp = ERR;
        t.ec = NewLnInStr;
        t.ln = line_number;
        strcpy(t.fl, filename);
        strcpy(t.lx, "Error: new line in string constant");
        return t;
      }
    }
    if (c == EOF) {
      // Unterminated string error
      t.tp = ERR;
      t.ec = EofInStr;
      t.ln = line_number;
      strcpy(t.fl, filename);
      strcpy(t.lx, "Error: unexpected eof in string constant");
      return t;
    }
    t.lx[i] = '\0';
    t.tp = STRING;
    t.ln = line_number;
    strcpy(t.fl, filename);
    return t;
  } else if (strchr(symbols, c) != NULL) {
    // Handle symbols
    t.lx[0] = c;
    t.lx[1] = '\0';
    t.tp = SYMBOL;
    t.ln = line_number;
    strcpy(t.fl, filename);
    return t;
  } else {
    // Invalid symbol error
    t.tp = ERR;
    t.ec = IllSym;
    t.lx[0] = c;
    t.lx[1] = '\0';
    t.ln = line_number;
    strcpy(t.fl, filename);
    strcpy(t.lx, "Error: illegal symbol in source file");
    return t;
  }
}

// peek (look) at the next token in the source file without removing it from the stream
Token PeekNextToken ()
{
  Token t;
  t.tp = ERR;

  fpos_t pos;
  fgetpos(file_stream, &pos);
  int old_line_number = line_number;

  t = GetNextToken();

  fsetpos(file_stream, &pos);
  line_number = old_line_number;

  return t;
}

// clean out at end, e.g. close files, free memory, ... etc
int StopLexer ()
{
  fclose(file_stream);
  filename = NULL;
  line_number = 1;
	return 0;
}

// do not remove the next line
#ifndef TEST
int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
    return 1;
  }

  if (!InitLexer(argv[1])) {
    fprintf(stderr, "Failed to initialize lexer\n");
    return 1;
  }

  Token next_token;
  while ((next_token = GetNextToken()).tp != EOFile) {
    if (next_token.tp == ERR) {
      fprintf(stderr, "Error on line %d: %s\n", next_token.ln, next_token.lx);
      continue;
    }

    printf("<%s, %d, %s, %d>\n", next_token.fl, next_token.ln, next_token.lx, next_token.tp);
  }

  StopLexer();
  return 0;
}
// do not remove the next line
#endif
