#ifndef GLOBALS_H
#define GLOBALS_H

#define QTD_PALAVRAS_RESERVADAS 8
#define QTD_TIPOS_DE_DADOS 3
#define QTD_OPERADORES 15
#define QTD_MARCADORES 9

typedef struct TokenNode {
  char *token;
  int linha;
  struct TokenNode *prox;
} TokenNode;

// Declarações dos arrays globais (definidos no .c)
extern const char *palavrasReservadas[];
extern const char *tiposDeDados[];
extern const char *operadores[];
extern const char *marcadores[];

#endif