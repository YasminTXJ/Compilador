#ifndef GLOBALS_H
#define GLOBALS_H

typedef struct TokenNode {
  char *token;
  int linha;
  struct TokenNode *prox;
} TokenNode;

#endif