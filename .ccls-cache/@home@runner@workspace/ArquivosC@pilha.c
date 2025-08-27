#include "../ArquivosH/pilha.h"

// Empilhar
Pilha *push(Pilha *topo, char simbolo, int linha) {
  Pilha *novo = (Pilha *)malloc(sizeof(Pilha));
  novo->simbolo = simbolo;
  novo->linha = linha;
  novo->prox = topo;
  return novo;
}

// Desempilhar
Pilha *pop(Pilha *topo, char *simbolo, int *linha) {
  if (topo == NULL)
    return NULL;
  *simbolo = topo->simbolo;
  *linha = topo->linha;
  Pilha *tmp = topo;
  topo = topo->prox;
  free(tmp);
  return topo;
}

// Verificar se a pilha está vazia
int isEmpty(Pilha *topo) { return topo == NULL; }
