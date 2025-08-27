#ifndef TABELA_SIMBOLOS_H
#define TABELA_SIMBOLOS_H

#include "globals.h" // para usar Simbolo e tabela_simbolos
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Declaração de funções da tabela de símbolos
Simbolo *buscarSimbolo(Simbolo *tabela, const char *nome, const char *funcao);
Simbolo *adicionarSimbolo(Simbolo *inicio, const char *tipo, const char *nome,
                          const char *valor, const char *funcao);
Simbolo *atribuirValor(Simbolo *inicio, const char *nome, const char *valor,
                       const char *funcao);
void imprimirTabela(Simbolo *inicio);

#endif
