/************************************************************************
University of Leeds
School of Computing
COMP2932- Compiler Design and Construction
Lexer Module

I confirm that the following code has been developed and written by me and it is entirely the result of my own work.
I also confirm that I have not copied any parts of this program from another person or any other source or facilitated someone to copy this program from me.
I confirm that I will not publish the program online or share it with anyone without permission of the module leader.

Student Name:
Student ID:
Email:
Date Work Commenced:
*************************************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"


// YOU CAN ADD YOUR OWN FUNCTIONS, DECLARATIONS AND VARIABLES HERE
const char *reswords[21] = {
  "class", "constructor", "method", "function",
  "int", "boolean", "char", "void",
  "var", "static", "field",
  "let", "do", "if", "else", "while", "return",
  "ture", "false", "null",
  "this"
};

const char *sym[19] = {
  ')', '(', '[', ']', '{', '}',
  ',', ';', '=', '.',
  '+', '-', '*', '/', '&', '|',
  '~', '>', '<'
};

static unsigned char *buffer;

long getFilesize(FILE *fptr)
{
  long filesize;

  if (fseek(fptr, 0, SEEK_END) != 0) exit(0);

  filesize = ftell(fptr);
  rewind(fptr);

  return filesize;
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
  FILE *fp = fopen(file_name, "r");

  if (fp == NULL)
  {
    printf("Error: file does not exist\n");
    return 0;
  }

  fread(buffer, sizeof(unsigned char), 5000, fp);

  return 1;
}


// Get the next token from the source file
Token GetNextToken ()
{
	Token t;
  t.tp = ERR;
  return t;
}

// peek (look) at the next token in the source file without removing it from the stream
Token PeekNextToken ()
{
  Token t;
  t.tp = ERR;
  return t;
}

// clean out at end, e.g. close files, free memory, ... etc
int StopLexer ()
{
	return 0;
}

// do not remove the next line
#ifndef TEST
int main ()
{
	// implement your main function here
  // NOTE: the autograder will not use your main function

  
	return 0;
}
// do not remove the next line
#endif
