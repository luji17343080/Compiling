#include <stdio.h>
#include <string.h>

/* 将Tiny+语句变为token */

/* Tiny+ token type*/
typedef enum {
  /* special symbols */
  EQ = 256,NE,ASSIGN,IDENTIFIER,INT_LITERAL,REAL_LITERAL,STRING_LITERAL,
  
  /* keywords */
  INT,REAL,STRING,MAIN,BEGIN,END,IF,ELSE,READ,WRITE,RETURN
} TokenType;

/* Define keyword groups */
char* keywords[] = {"INT", "REAL", "STRING", "MAIN",  "BEGIN", "END",
                    "IF",  "ELSE", "READ",   "WRITE", "RETURN"};

/* Match keywords */
TokenType matchKeyword(const char* c) {
  for (int i = 0; i < sizeof(keywords) / sizeof(keywords[0]); ++i) {
    if (strcmp(c, keywords[i]) == 0) {
      return INT + i;
    }
  }
  return IDENTIFIER;
}

char* tokens[] = {
  /* special symbol */
  "==", "!=", ":=", "IDENTIFIER", "INT_LITERAL", "REAL_LITERAL", "STRING_LITERAL",
  
  /* keywords */        
  "INT", "REAL", "STRING", "MAIN", "BEGIN", "END",
  "IF", "ELSE", "READ", "WRITE", "RETURN"};

/* Get token for lexical analysis */
char* getToken(TokenType t) {
  if (t >= 256) {
    return strdup(tokens[t - 256]);
  }
  char str[2] = {t, 0};
  return strdup(str);
}