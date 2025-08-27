#ifndef PILHA_H
#define PILHA_H

#include <stdlib.h>

// Estrutura da pilha
typedef struct Pilha {
  char simbolo; // delimitador
  int linha;    // número da linha
  struct Pilha *prox;
} Pilha;

// Funções da pilha
Pilha *push(Pilha *topo, char simbolo, int linha);
Pilha *pop(Pilha *topo, char *simbolo, int *linha);
int isEmpty(Pilha *topo);

#endif
