

#ifndef VERIFICASINTAXE_H
#define VERIFICASINTAXE_H

#include "globals.h"
#include <ctype.h>
#include <string.h>

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
#endif // VERIFICASINTAXE_H
