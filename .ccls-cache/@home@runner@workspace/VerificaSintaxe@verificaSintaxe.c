#include "verificaSintaxe.h"

const char *palavrasReservadas[] = {"principal", "funcao", "retorno", "leia",
                                    "escreva",   "se",     "senao",   "para"};
const char *tiposDeDados[] = {"inteiro", "texto", "decimal"};
const char *operadores[] = {"==", "<>", "<=", ">=", "&&", "||", "+",
                            "-",  "*",  "/",  "^",  "<",  ">",  "="};
const char *marcadores[] = {"(", ")", "{", "}", "[", "]", ";", ",", " "};

int verificaAsciiValido(char c) { return (c >= 0 && c <= 127); }

int ehPalavraReservada(const char *token) {
  for (int i = 0;
       i < sizeof(palavrasReservadas) / sizeof(palavrasReservadas[0]); i++)
    if (strcmp(token, palavrasReservadas[i]) == 0)
      return 1;
  return 0;
}

int ehTipoDeDado(const char *token) {
  for (int i = 0; i < sizeof(tiposDeDados) / sizeof(tiposDeDados[0]); i++)
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
  for (int i = 0; i < sizeof(operadores) / sizeof(operadores[0]); i++)
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