
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 1024
#define KB 1024
#define DEFAULT_MEMORY_LIMIT (2048 * KB)
// Verificações de tokens
const char *keywords[] = {"principal", "funcao", "retorno", "leia",
                          "escreva",   "se",     "senao",   "para"};
const char *tipos[] = {"inteiro", "texto", "decimal"};
const char *operadores[] = {"==", "<>", "<=", ">=", "&&", "||", "+",
                            "-",  "*",  "/",  "^",  "<",  ">",  "="};
size_t total_memory_used = 0;
size_t max_memory_used = 0;
size_t memory_limit = DEFAULT_MEMORY_LIMIT;

// Alocação de memória
void *safe_malloc(size_t size) {
  if (total_memory_used + size > memory_limit) {
    fprintf(stderr, "ERRO: Memória Insuficiente\n");
    exit(EXIT_FAILURE);
  }

  void *ptr = malloc(size);
  if (!ptr) {
    fprintf(stderr, "ERRO: Falha na alocação\n");
    exit(EXIT_FAILURE);
  }

  total_memory_used += size;
  if (total_memory_used > max_memory_used) {
    max_memory_used = total_memory_used;
  }

  return ptr;
}

void safe_free(void *ptr, size_t size) {
  if (ptr) {
    free(ptr);
    if (total_memory_used >= size) {
      total_memory_used -= size;
    }
  }
}
int isKeyword(const char *token) {
  for (int i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
    if (strcmp(token, keywords[i]) == 0) return 1;
  }

  return 0;
}

int isTipo(const char *token) {
  for (int i = 0; i < sizeof(tipos) / sizeof(tipos[0]); i++) {
    if (strcmp(token, tipos[i]) == 0) return 1;
  }
  return 0;
}

int isOperador(const char *token) {
  for (int i = 0; i < sizeof(operadores) / sizeof(operadores[0]); i++) {
    if (strcmp(token, operadores[i]) == 0) return 1;
  }

  return 0;
}

int isVariavel(const char *token) {
  return token[0] == '!' && islower(token[1]);
}

int isFuncao(const char *token) {
  return strncmp(token, "__", 2) == 0 &&
         (isalpha(token[2]) || isdigit(token[2]));
}

int isNumero(const char *token) {
  int ponto = 0;
  for (int i = 0; token[i]; i++) {
    if (token[i] == '.') ponto++;
    else if (!isdigit(token[i])) return 0;
  }
  return ponto <= 1;
}

// Estrutura da tabela de símbolos
typedef struct Simbolo {
  char tipo[20];
  char nome[50];
  char valor[100];
  char escopo[50];
  struct Simbolo *prox;
} Simbolo;
Simbolo *tabela_simbolos = NULL;

// Função para buscar símbolo
Simbolo *buscarSimbolo(const char *nome, const char *escopo) {
  Simbolo *atual = tabela_simbolos;
  while (atual != NULL) {
    if (strcmp(atual->nome, nome) == 0 && strcmp(atual->escopo, escopo) == 0) {
      return atual;
    }
    atual = atual->prox;
  }
  return NULL;
}

// Criar símbolo
Simbolo *criarSimbolo(const char *tipo, const char *nome, const char *valor,
                      const char *escopo) {
  Simbolo *novo = (Simbolo *)safe_malloc(sizeof(Simbolo));
  strcpy(novo->tipo, tipo);
  strcpy(novo->nome, nome);
  strcpy(novo->valor, valor);
  strcpy(novo->escopo, escopo);
  novo->prox = NULL;
  return novo;
}

// Adicionar símbolo
void adicionaSimbolo(const char *tipo, const char *nome, const char *valor,
                     const char *escopo) {
  Simbolo *novo = criarSimbolo(tipo, nome, valor, escopo);
  novo->prox = tabela_simbolos;
  tabela_simbolos = novo;
}

