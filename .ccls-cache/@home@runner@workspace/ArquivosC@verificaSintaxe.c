#include "../ArquivosH/verificaSintaxe.h"
#include "../ArquivosH/globals.h"
#include <stdio.h>

int verificaAsciiValido(char c) { return (c >= 0 && c <= 127); }

int ehPalavraReservada(const char *token) {
  for (int i = 0; i < QTD_PALAVRAS_RESERVADAS; i++)
    if (strcmp(token, palavrasReservadas[i]) == 0)
      return 1;
  return 0;
}

int ehTipoDeDado(const char *token) {
  for (int i = 0; i < QTD_TIPOS_DE_DADOS; i++)
    if (strcmp(token, tiposDeDados[i]) == 0)
      return 1;
  return 0;
}

int ehString(const char *token) {
  return (token[0] == '"' && token[strlen(token) - 1] == '"');
}

int ehFuncao(const char *token) { return (strncmp(token, "__", 2) == 0); }

int VerificaSeNomeDeVarOuFuncEhValido(const char *token) {
  for (int i = 0; token[i] != '\0'; i++)
    if (!isalpha(token[i]) && !isdigit(token[i]))
      return 0;
  return 1;
}

int ehVariavel(const char *token) {
  if (token[0] == '!' && islower(token[1])) {
    if (strlen(token) > 2)
      return VerificaSeNomeDeVarOuFuncEhValido(token + 2);
    else
      return 1;
  }
  return 0;
}

int ehNumero(const char *token) {
  int ponto = 0;
  for (int i = 0; token[i]; i++) {
    if (token[i] == '.')
      ponto++;
    else if (!isdigit(token[i]))
      return 0;
  }
  return ponto <= 1;
}
int ehOperador(const TokenNode *token) {
  for (int i = 0; i < QTD_OPERADORES; i++)
    if (strcmp(token->token, operadores[i]) == 0) {
      // TokenNode *proximo = token->prox;
      // for (int j = 0; j < sizeof(operadores) / sizeof(operadores[0]); j++) {
      //   if (proximo->token == operadores[j])
      //     return 0;
      // }
      return 1;
    }

  return 0;
}

int ehMarcador(const char *token) {
  for (int i = 0; i < QTD_MARCADORES; i++) {
    if (strcmp(token, marcadores[i]) == 0) {
      return 1;
    }
  }
  return 0;
}