

#ifndef VERIFICASINTAXE_H
#define VERIFICASINTAXE_H

#include "globals.h"
#include <ctype.h>
#include <string.h>

// Declarações dos arrays globais (definidos no .c)
extern const char *palavrasReservadas[];
extern const char *tiposDeDados[];
extern const char *operadores[];
extern const char *marcadores[];

TokenNode *verificaParDeExpressao(TokenNode *tk);
bool ehBlocoMultilinha(const TokenNode *tk);
int verificaSe(const TokenNode *tk);

// Protótipos das funções
int verificaAsciiValido(char c);
int ehPalavraReservada(const char *token);
int ehTipoDeDado(const char *token);
int ehString(const char *token);
int ehFuncao(const char *token);
int VerificaSeNomeDeVarOuFuncEhValido(const char *token);
int ehVariavel(const char *token);
int ehNumero(const char *token);
int ehOperador(const TokenNode *token);
int ehMarcador(const char *token);

int processarDeclaracao(TokenNode *token, char *tipo_atual, char *escopo_atual);

int VerificaSintaxeEhValida(TokenNode *head);

#endif // VERIFICASINTAXE_H
