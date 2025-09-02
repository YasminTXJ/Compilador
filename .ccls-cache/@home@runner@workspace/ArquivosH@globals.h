#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdbool.h> // para usar bool
#include <stddef.h>

// Constantes
#define MAX_TOKEN_SIZE 256
#define MAX_LINE 1024
#define KB 1024
#define DEFAULT_MEMORY_LIMIT (2048 * KB)

#define QTD_PALAVRAS_RESERVADAS 8
#define QTD_TIPOS_DE_DADOS 3
#define QTD_OPERADORES 15
#define QTD_MARCADORES 9

// Estrutura da tabela de símbolos

typedef struct TokenNode {
  char *token;
  int linha;
  struct TokenNode *prox;
} TokenNode;

// Variáveis globais (apenas declaração)
extern size_t total_memory_used;
extern size_t max_memory_used;
extern size_t memory_limit;

extern char escopo_atual[50];
extern char funcao_anterior[50];
extern char tipo_atual[20];
extern char nome_atual[50];
extern char valor_atual[100];

extern TokenNode *token_list;
extern TokenNode *token_list_tail;
extern int tokenCount;
extern int flag_escopo_palavra_reservada;

// Variável global (apenas declaração)
extern bool principalExiste;

#endif