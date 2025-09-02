#include "../ArquivosH/verificaSintaxe.h"
#include "../ArquivosH/globals.h"
#include "../ArquivosH/mensagens.h"
#include "../ArquivosH/tabelaSimbolos.h"
#include <stdio.h>

const char *palavrasReservadas[] = {"principal", "funcao", "retorno", "leia",
                                    "escreva",   "se",     "senao",   "para"};

const char *tiposDeDados[] = {"inteiro", "texto", "decimal"};

const char *operadores[] = {"==", "<>", "<=", ">=", "&&", "||", "+",
                            "-",  "*",  "/",  "^",  "<",  ">",  "="};

const char *marcadores[] = {"(", ")", "{", "}", "[", "]", ";", ",", " "};

// Verificação de sintaxe usando lista encadeada

int VerificaSintaxeEhValida(TokenNode *token); // protótipo da função

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

TokenNode *verificaParDeExpressao(TokenNode *tk) {
  TokenNode *expr = tk;
  if (!expr) {
    printf("ERRO SINTÁTICO: condição vazia no 'se' - LINHA %d\n", tk->linha);
    exit(1);
  }
  // operando 1
  if (!(ehVariavel(expr->token) || ehNumero(expr->token) ||
        ehString(expr->token))) {
    printf("ERRO SINTÁTICO: condição inválida ('%s') - LINHA %d\n", expr->token,
           expr->linha);
    exit(1);
  }
  // operador
  TokenNode *op = expr->prox;
  if (!op || !ehOperador(op)) {
    printf("ERRO SINTÁTICO: esperado operador após '%s' - LINHA %d\n",
           expr->token, expr->linha);
    exit(1);
  }
  // checagem semântica de operadores
  if (strcmp(op->token, "=<") == 0 || strcmp(op->token, "=>") == 0 ||
      strcmp(op->token, "><") == 0 || strcmp(op->token, "!=") == 0 ||
      strcmp(op->token, "<<") == 0 || strcmp(op->token, ">>") == 0) {
    printf("ERRO SEMÂNTICO: operador inválido '%s' - LINHA %d\n", op->token,
           op->linha);
    exit(1);
  }
  if (ehOperador(op) && ehOperador(op->prox)) {
    printf("ERRO SEMÂNTICO: operadores duplicados na condição do 'se' - LINHA "
           "%d\n",
           op->linha);
    exit(1);
  }
  // operando 2
  TokenNode *expr2 = op->prox;
  if (!expr2 || !(ehVariavel(expr2->token) || ehNumero(expr2->token) ||
                  ehString(expr2->token))) {
    printf("ERRO SINTÁTICO: esperado operando após operador '%s' - LINHA %d\n",
           op->token, op->linha);
    exit(1);
  }
  // restrição: texto só pode usar == ou <>
  if ((ehString(expr->token) || ehString(expr2->token)) &&
      (strcmp(op->token, "<") == 0 || strcmp(op->token, "<=") == 0 ||
       strcmp(op->token, ">") == 0 || strcmp(op->token, ">=") == 0)) {
    printf("ERRO SEMÂNTICO: operador '%s' inválido para texto - LINHA %d\n",
           op->token, op->linha);
    exit(1);
  }
  if ((!expr2->prox || strcmp(expr2->prox->token, ")") != 0) &&
      (!strcmp(expr2->prox->token, "&&") &&
       !strcmp(expr2->prox->token, "||"))) {
    printf("ERRO SINTÁTICO: esperado ')' após expressão - LINHA %d\n",
           expr2->linha);
    exit(1);
  } else {
    if (strcmp(expr2->prox->token, "&&") == 0 ||
        strcmp(expr2->prox->token, "||") == 0) {
      return verificaParDeExpressao(expr2->prox->prox);
    } else {
      return expr2->prox->prox; // retorna o token depois do ')'
    }
  }
  printf("erro ao validar expressão do se Linha : %d\n", tk->linha);
  exit(1);
}

bool ehBlocoMultilinha(const TokenNode *tk) {
  if (!tk)
    return false;
  return strcmp(tk->token, "{") == 0;
}

int verificaSe(const TokenNode *tk) {
  // 1) confere se é "se"
  if (!tk || strcmp(tk->token, "se") != 0) {
    printf("ERRO SINTÁTICO: esperado 'se' - LINHA %d\n", tk->linha);
    exit(1);
  }
  // 2) precisa ter "(" logo após
  if (!tk->prox || strcmp(tk->prox->token, "(") != 0) {
    printf("ERRO SINTÁTICO: esperado '(' após 'se' - LINHA %d\n", tk->linha);
    exit(1);
  }
  // 3) expressão entre ()
  TokenNode *expr = tk->prox->prox;
  // 4) validar a expressão
  TokenNode *blocoV = verificaParDeExpressao(expr);
  if (!blocoV) {
    printf("ERRO SINTÁTICO: esperado bloco após condição do 'se' - LINHA %d\n",
           expr->linha);
    exit(1);
  }
  // se for bloco múltiplo
  if (ehBlocoMultilinha(blocoV)) {
    if (strcmp(blocoV->token, "{") != 0) {
      printf("ERRO SINTÁTICO: bloco múltiplo sem '{' no 'se' - LINHA %d\n",
             blocoV->linha);
      exit(1);
    }
    TokenNode *cur = blocoV->prox;
    while (cur && strcmp(cur->token, "}") != 0) {
      if (ehTipoDeDado(cur->token)) {
        printf("ERRO SEMÂNTICO: declaração de variável não permitida dentro do "
               "'se' - LINHA %d\n",
               cur->linha);
        exit(1);
      }
      if (strcmp(cur->token, "se") == 0) {
        verificaSe(cur);
      }
      cur = cur->prox;
    }
    if (!cur) {
      printf("ERRO SINTÁTICO: bloco do 'se' não fechado com '}' - LINHA %d\n",
             blocoV->linha);
      exit(1);
    }
  } else {
    if (ehTipoDeDado(blocoV->token)) {
      printf("ERRO SEMÂNTICO: declaração de variável não permitida dentro do "
             "'se' - LINHA %d\n",
             blocoV->linha);
      exit(1);
    }
  }
  // 5) checar senao (opcional)
  TokenNode *senao = blocoV->prox;
  while (senao && strcmp(senao->token, ";") == 0) {
    senao = senao->prox; // pula ;
  }
  if (senao && strcmp(senao->token, "senao") == 0) {
    TokenNode *blocoF = senao->prox;
    if (!blocoF) {
      printf("ERRO SINTÁTICO: esperado bloco após 'senao' - LINHA %d\n",
             senao->linha);
      exit(1);
    }
    if (ehBlocoMultilinha(blocoF)) {
      if (strcmp(blocoF->token, "{") != 0) {
        printf("ERRO SINTÁTICO: bloco múltiplo sem '{' no 'senao' - LINHA %d\n",
               blocoF->linha);
        exit(1);
      }
      TokenNode *cur = blocoF->prox;
      while (cur && strcmp(cur->token, "}") != 0) {
        if (ehTipoDeDado(cur->token)) {
          printf("ERRO SEMÂNTICO: declaração de variável não permitida dentro "
                 "do 'senao' - LINHA %d\n",
                 cur->linha);
          exit(1);
        }
        if (strcmp(cur->token, "se") == 0) {
          verificaSe(cur);
        }
        cur = cur->prox;
      }
      if (!cur) {
        printf(
            "ERRO SINTÁTICO: bloco do 'senao' não fechado com '}' - LINHA %d\n",
            blocoF->linha);
        exit(1);
      }
    } else {
      if (!blocoF->prox || strcmp(blocoF->prox->token, ";") != 0) {
        printf("ERRO SINTÁTICO: comando único do 'senao' deve terminar com ';' "
               "- LINHA %d\n",
               blocoF->linha);
        exit(1);
      }
      if (ehTipoDeDado(blocoF->token)) {
        printf("ERRO SEMÂNTICO: declaração de variável não permitida dentro do "
               "'senao' - LINHA %d\n",
               blocoF->linha);
        exit(1);
      }
    }
  }
  return 1; // passou por tudo
}

bool ehDecimal(const char *str) {
  if (str[0] == '\0')
    return false; // string vazia não é número

  int ponto = 0;

  for (int i = 0; str[i] != '\0'; i++) {
    if (str[i] == '.') {
      ponto++;
      if (ponto > 1)
        return false; // mais de um ponto, não é decimal válido
    } else if (!isdigit(str[i])) {
      return false; // caractere inválido
    }
  }

  // Para ser decimal, deve ter **exatamente um ponto** e pelo menos um dígito
  return (ponto == 1);
}

bool ehInteiro(const char *str) {
  if (str[0] == '\0')
    return false; // string vazia não é número

  for (int i = 0; str[i] != '\0'; i++) {
    if (!isdigit(str[i]))
      return false; // se tiver caractere que não é dígito
  }

  return true;
}

void verificaLeia(TokenNode *token) {
  TokenNode *prox = token->prox->prox;
  if (prox->token == NULL) {
    printf("-={********************************************************}="
           "-\n");
    printf("ERRO Sintático: Comando 'leia' sem variável - LINHA %d\n",
           prox->linha);
    printf("-={********************************************************}="
           "-\n");
    exit(1);
  }
  if (prox->token != NULL && !ehVariavel(prox->token)) {
    printf("-={********************************************************}="
           "-\n");
    printf("ERRO Semântico: Comando 'leia' só pode ler variavéis - LINHA %d\n",
           prox->linha);
    printf("-={********************************************************}="
           "-\n");
    exit(1);
  }
  // pode ler mais de uma variavel, separadas por virgula e declaradas
  // anteriormente
  if (prox->token != NULL && ehVariavel(prox->token) &&
      ehVariavel(prox->prox->token)) {
    printf("-={********************************************************}="
           "-\n");
    printf("ERRO Sintático: As variavéis do comando 'leia' não estão separadas "
           "por ','- LINHA %d\n",
           prox->linha);
    printf("-={********************************************************}="
           "-\n");
    exit(1);
  }
  while (prox->token != NULL && ehVariavel(prox->token) &&
         strcmp(prox->prox->token, ",") == 0) {
    prox = prox->prox->prox;
  }

  if (prox->token != NULL && ehVariavel(prox->token) &&
      strcmp(prox->prox->token, ")") == 0 &&
      strcmp(prox->prox->prox->token, ";") == 0) {
    return;
  } else {
    printf("-={********************************************************}="
           "-\n");
    printf("ERRO Sintático: Comando 'leia' escrito incorretamente- LINHA %d\n",
           prox->linha);
    printf("-={********************************************************}="
           "-\n");
    exit(1);
  }
}

void verificaEscreva(TokenNode *token) {
  TokenNode *prox = token->prox->prox;

  // escreva(" A é maior", !a);
  if (prox->token == NULL) {
    imprimeMensagemErroSemToken(
        "ERRO Sintático: Comando 'escreva' sem variável ou texto", prox->linha);
    exit(1);
  }
  if (prox->token != NULL && !ehVariavel(prox->token) &&
      !ehString(prox->token)) {
    printf("-={********************************************************}="
           "-\n");
    printf("ERRO Semântico: Comando 'escreva' só pode escrever variavéis ou "
           "texto - LINHA %d\n",
           prox->linha);
    printf("-={********************************************************}="
           "-\n");
    exit(1);
  }
  // pode escrever mais de uma variavel ou texto, separadas por virgula e
  // declaradas anteriormente
  // escreva("Escreva um número", !a , !x ,);
  while (prox->token != NULL &&
         (ehVariavel(prox->token) == 0 || ehString(prox->token) == 0) &&
         strcmp(prox->prox->token, ",") == 0) {

    prox = prox->prox->prox; // pula a virgula
  }
  // se tiver uma virgula sobrando, ele vai sair do while no )
  if (strcmp(prox->token, ")") == 0) {
    printf("-={********************************************************}="
           "-\n");
    printf(
        "ERRO Sintático: Comando 'escreva' escrito incorretamente- LINHA %d\n",
        prox->linha);
    printf("-={********************************************************}="
           "-\n");
    exit(1);
  }
  // se tiver certo ele sai do while com uma variavel ou texto
  if (prox->token != NULL &&
      (ehVariavel(prox->token) == 0 || ehString(prox->token) == 0) &&
      strcmp(prox->prox->token, ")") == 0) {
    if (strcmp(prox->prox->prox->token, ";") == 0) {
      return;
    } else {
      printf("-={********************************************************}="
             "-\n");
      printf("ERRO Sintático: Comando 'escreva' escrito incorretamente- LINHA "
             "%d\n",
             prox->linha);
      printf("-={********************************************************}="
             "-\n");
      exit(1);
    }
  }
  if (prox->token != NULL &&
      (ehVariavel(prox->token) == 0 || ehString(prox->token) == 0) &&
      ehTipoDeDado(prox->token) != 0) {
    printf("-={********************************************************}="
           "-\n");
    printf("ERRO Sintático: Comando 'escreva' escrito incorretamente ,não "
           "podem conter declarações - LINHA %d\n",
           prox->linha);
    printf("-={********************************************************}="
           "-\n");
    exit(1);
  }
}

TokenNode *verificaFuncao(TokenNode *current) {

  strcpy(valor_atual, "");
  strcpy(tipo_atual, "funcao");
  TokenNode *nomeFuncao = current->prox;
  if (ehFuncao(nomeFuncao->token)) {
    strcpy(nome_atual, nomeFuncao->token);
  };
  TokenNode *openP = nomeFuncao->prox;
  TokenNode *proximo = openP->prox;
  if (strcmp(openP->token, "(") == 0) {
    while (strcmp(proximo->token, ")") != 0) {
      if (ehTipoDeDado(proximo->token) == 1) {
        strcat(valor_atual, proximo->token);
        proximo = proximo->prox;
        if (ehVariavel(proximo->token) == 1) {
          strcat(valor_atual, " ");
          strcat(valor_atual, proximo->token);
          proximo = proximo->prox;
          if (strcmp(proximo->token, ",") == 0) {
            strcat(valor_atual, proximo->token);
            proximo = proximo->prox;
            if (ehTipoDeDado(proximo->token) == 1) {
              continue;
            } else {
              imprimeMensagemErroSemToken(
                  "ERRO Sintático: Função com tipo de dado invalido ",
                  proximo->linha);
            }
          } else {
            continue;
          }

        } else {
          imprimeMensagemErroSemToken("ERRO Sintático: Função com parametro "
                                      "com nome de variavel invalido",
                                      proximo->linha);
          exit(1);
        }
      } else {
        imprimeMensagemErroSemToken(
            "ERRO Sintático: Função com tipo de dado invalido ",
            proximo->linha);
        exit(1);
      }
      strcat(valor_atual, proximo->token);
      proximo = proximo->prox;
    }
  } else {
    imprimeMensagemErroSemToken("ERRO Sintático: Função sem '('", openP->linha);
    exit(1);
  }

  adicionarSimbolo(tabela_simbolos, tipo_atual, nome_atual, valor_atual,
                   escopo_atual);
  return proximo;
}

TokenNode *verificaParDeExpressaoPara(TokenNode *tk) {
  TokenNode *expr = tk;
  if (!expr) {
    printf("ERRO SINTÁTICO: condição vazia no 'para' - LINHA %d\n", tk->linha);
    exit(1);
  }
  // operando 1
  // printf("Operando 1 do para: %s - Linha: %d\n", expr->token, expr->linha);
  if (!(ehVariavel(expr->token) || ehNumero(expr->token) ||
        ehString(expr->token))) {
    printf("ERRO SINTÁTICO: condição inválida ('%s') - LINHA %d\n", expr->token,
           expr->linha);
    exit(1);
  }
  // operador
  TokenNode *op = expr->prox;
  // printf("Operando 1: %s  \n", op->token);
  if (!op || !ehOperador(op)) {
    printf("ERRO SINTÁTICO: esperado operador após '%s' - LINHA %d\n",
           expr->token, expr->linha);
    exit(1);
  }
  // checagem semântica de operadores (6.5 + 3.2.3 + 3.2.4)
  if (strcmp(op->token, "=<") == 0 || strcmp(op->token, "=>") == 0 ||
      strcmp(op->token, "><") == 0 || strcmp(op->token, "!=") == 0 ||
      strcmp(op->token, "<<") == 0 || strcmp(op->token, ">>") == 0) {
    printf("ERRO SEMÂNTICO: operador inválido '%s' - LINHA %d\n", op->token,
           op->linha);
    exit(1);
  }
  if (ehOperador(op) && ehOperador(op->prox)) {
    printf("ERRO SEMÂNTICO: operadores duplicados na condição do 'se' - LINHA "
           "%d\n",
           op->linha);
    exit(1);
  }
  // operando 2
  TokenNode *expr2 = op->prox;
  // printf("Exp2 2: %s  \n", expr2->token);
  if (!expr2 || !(ehVariavel(expr2->token) || ehNumero(expr2->token) ||
                  ehString(expr2->token))) {
    printf("ERRO SINTÁTICO: esperado operando após operador '%s' - LINHA %d\n",
           op->token, op->linha);
    exit(1);
  }
  // restrição: texto só pode usar == ou <>
  if ((ehString(expr->token) || ehString(expr2->token)) &&
      (strcmp(op->token, "<") == 0 || strcmp(op->token, "<=") == 0 ||
       strcmp(op->token, ">") == 0 || strcmp(op->token, ">=") == 0)) {
    printf("ERRO SEMÂNTICO: operador '%s' inválido para texto - LINHA %d\n",
           op->token, op->linha);
    exit(1);
  } else {
    return (expr2);
  }

  printf("erro ao validar expressão do se Linha : %d\n", tk->linha);
  exit(1);
}

TokenNode *verificaPara(TokenNode *tk) {
  // 1) confere se é "para"// vai vir de fora
  // printf("Token: %s - Linha: %d\n", tk->token, tk->linha);
  if (!tk || strcmp(tk->token, "para") != 0) {
    printf("ERRO SINTÁTICO: esperado 'para' - LINHA %d\n", tk->linha);
    exit(1);
  }

  // 2) precisa ter "(" logo após
  // printf("Token PROX: %s - Linha: %d\n", tk->prox->token, tk->prox->linha);
  if (!tk->prox || strcmp(tk->prox->token, "(") != 0) {
    printf("ERRO SINTÁTICO: esperado '(' após 'para' - LINHA %d\n", tk->linha);
    exit(1);
  }

  TokenNode *expr = tk->prox->prox;
  // printf("Token prox prox : %s - Linha: %d\n", expr->token, expr->linha);
  // pego o primeiro token depois da virgula

  // -------- X1: inicialização --------
  // verifica se já foi declarado
  if (buscarSimbolo(tabela_simbolos, expr->token, escopo_atual) == NULL) {
    printf("ERRO SEMÂNTICO: variável '%s' não declarada no 'para' - LINHA %d\n",
           expr->token, expr->linha);
    exit(1);
  }

  // pode ser vazio (quando vem direto um ";")
  if (strcmp(expr->token, ";") != 0) {
    while (expr && strcmp(expr->token, ";") != 0) {
      if (!ehVariavel(expr->token) && strcmp(expr->token, "=") != 0 &&
          !ehNumero(expr->token) && strcmp(expr->token, ",") != 0) {
        printf("ERRO SINTÁTICO: inicialização inválida em 'para' - LINHA %d\n",
               expr->linha);
        exit(1);
      }
      expr = expr->prox;
    }
  }
  // printf("Token que deveria ser ; = %s - Linha: %d\n", expr->token,
  // expr->linha);
  if (!expr || strcmp(expr->token, ";") != 0) {
    printf("ERRO SINTÁTICO: esperado ';' após inicialização no 'para' - LINHA "
           "%d\n",
           expr ? expr->linha : tk->linha);
    exit(1);
  }
  expr = expr->prox;
  // -------- X2: condição --------
  // printf("Inicio da condição  = %s - Linha: %d\n", expr->token, expr->linha);
  // verifica se já foi declarado
  if (ehVariavel(expr->token)) {
    if (buscarSimbolo(tabela_simbolos, expr->token, escopo_atual) == NULL) {
      printf(
          "ERRO SEMÂNTICO: variável '%s' não declarada no 'para' - LINHA %d\n",
          expr->token, expr->linha);
      exit(1);
    }
  } else {
    printf("ERRO SINTÁTICO: condição vazia no 'para' - LINHA %d\n",
           expr->linha);
    exit(1);
  }

  if (strcmp(expr->token, ";") != 0) {
    expr = verificaParDeExpressaoPara(expr);
  } else {
    printf("ERRO SINTÁTICO: condição vazia no 'para' - LINHA %d\n",
           expr->linha);
    exit(1);
  }
  expr = expr->prox;
  if (!expr || strcmp(expr->token, ";") != 0) {
    printf("ERRO SINTÁTICO: esperado ';' após condição no 'para' - LINHA %d\n",
           expr ? expr->linha : tk->linha);
    exit(1);
  }
  expr = expr->prox;

  // -------- X3: incremento --------
  // printf("Inicio do incremento  = %s - Linha: %d\n", expr->token,
  // expr->linha);
  if (strcmp(expr->token, ")") != 0) {
    while (expr && strcmp(expr->token, ")") != 0) {
      // printf("EXPRESSAO: %s - LINHA: %d\n", expr->token, expr->linha);
      if (!(ehVariavel(expr->token) || ehNumero(expr->token) ||
            strcmp(expr->token, "+") == 0 || strcmp(expr->token, "-") == 0 ||
            ehOperador(expr))) {
        printf("ERRO SINTÁTICO: incremento inválido em 'para' - LINHA %d\n",
               expr->linha);
        exit(1);
      }
      if (ehVariavel(expr->token)) {
        if (buscarSimbolo(tabela_simbolos, expr->token, escopo_atual) == NULL) {
          printf("ERRO SEMÂNTICO: variável '%s' não declarada no 'para' - "
                 "LINHA %d\n",
                 expr->token, expr->linha);
          exit(1);
        }
      }
      expr = expr->prox;
    }
  } else {
    imprimeMensagemErroSemToken("ERRO SINTÁTICO: incremento vazio no 'para'",
                                expr->linha);
    exit(1);
  }
  if (!expr || strcmp(expr->token, ")") != 0) {
    imprimeMensagemErroSemToken(
        "ERRO SINTÁTICO: esperado ')' após incremento no 'para'",
        expr ? expr->linha : tk->linha);
    exit(1);
  }
  TokenNode *tokenAposPara = expr;
  expr = expr->prox;
  // printf("Saindo do para  = %s - Linha: %d\n", expr->token, expr->linha);
  // -------- Corpo do laço --------
  if (!expr || strcmp(expr->prox->token, "}") == 0) {
    imprimeMensagemErroSemToken("ERRO SINTÁTICO: esperado corpo após 'para'",
                                tk->linha);
    exit(1);
  }
  if (ehBlocoMultilinha(expr)) {
    TokenNode *cur = expr->prox;
    while (cur && strcmp(cur->token, "}") != 0) {
      if (ehTipoDeDado(cur->token)) {
        imprimeMensagemErroSemToken(
            "ERRO SEMÂNTICO: declaração não permitida dentro do 'para'",
            cur->linha);
        exit(1);
      }
      if (strcmp(cur->token, "para") == 0) {
        verificaPara(cur); // suporte a para aninhado
      }
      cur = cur->prox;
    }
    if (!cur) {
      printf("ERRO SINTÁTICO: bloco do 'para' não fechado com '}' - LINHA %d\n",
             expr->linha);
      exit(1);
    }
  }
  return tokenAposPara; // passou em tudo
}

//     ---------------------------------
//     Função de processamento de declaração
//     ---------------------------------

int processarDeclaracao(TokenNode *token, char *tipo_atual,
                        char *escopo_atual) {
  // printf("nome processado = %s\n", token->token);
  if (!token || !token->prox)
    return 0;
  if (ehFuncao(token->token)) {
    strcpy(valor_atual, "");
    strcpy(nome_atual, token->token);
    return 1;
  }
  // Pega o nome da variável (token seguinte ao tipo)
  if (ehVariavel(token->prox->token)) {
    strcpy(nome_atual, token->prox->token);
    // printf("nome atual = %s\n", nome_atual);
  } else {
    return 0;
  }

  TokenNode *atual = token->prox->prox;

  if (strcmp(token->token, "decimal") == 0 ||
      strcmp(token->token, "texto") == 0) {
    // Verifica se é um array com tamanho(ex: car[2.5])
    // printf("Atual = %s\n", atual->token);
    if (atual && ehMarcador(atual->token) && strcmp(atual->token, "[") == 0) {
      strcat(nome_atual, "[");
      atual = atual->prox;
      if ((strcmp(token->token, "decimal") == 0 &&
           ehDecimal(atual->token) == false) ||
          (strcmp(token->token, "texto") == 0 &&
           ehInteiro(atual->token) == false)) {
        alertaSemantico("variavél sem tamanho correto declarado", atual->linha);
      }

      // pega o que está entre os colchetes
      if (ehNumero(atual->token)) {
        strcat(nome_atual, atual->token);
        atual = atual->prox;
        if (atual && ehMarcador(atual->token) &&
            strcmp(atual->token, "]") == 0) {
          strcat(nome_atual, "]");
        }
        atual = atual->prox;
      } else { // se não tem nada entre os colchetes, erro
        alertaSemanticoComToken("variavél declara sem tamanho", atual->token,
                                atual->linha);
      }
    } else {
      alertaSemantico("variavél sem tamanho correto declarado", atual->linha);
    }
  }

  // Verifica se há um operador "=" após o nome
  if (atual && ehOperador(atual) && strcmp(atual->token, "=") == 0) {
    // printf("eh operador = %s\n", atual->token);

    atual = atual->prox;
    if (atual)
      strcpy(valor_atual, atual->token);

    atual = atual->prox;
    while (atual && !ehMarcador(atual->token)) {
      strcat(valor_atual, atual->token);
      atual = atual->prox;
    }

    // printf("valoooor atual = %s\n", valor_atual);
    tabela_simbolos = adicionarSimbolo(tabela_simbolos, tipo_atual, nome_atual,
                                       valor_atual, escopo_atual);
    // printf("adicionado na tabela de simbolos\n");

    return 1; // consumiu até o valor
  }
  // Caso seja declaração sem inicialização
  else if (atual && ehMarcador(atual->token)) {
    // printf("token marcador = %s\n", atual->token);

    tabela_simbolos = adicionarSimbolo(tabela_simbolos, tipo_atual, nome_atual,
                                       "", escopo_atual);
    // printf("adicionado na tabela de simbolos sem valor\n");

    // verifica se tem vírgula e continua declarando
    if (strcmp(atual->token, ",") == 0) {
      // printf("token virgula = %s\n", atual->prox->token);
      if (ehTipoDeDado(atual->prox->token) !=
          0) { // se o proximo token for um tipo de dado, é um erro
        imprimeMensagemErro("ERRO Sintático: Não se pode declarar dois tipos "
                            "de dados na mesma linha",
                            atual->prox->token, atual->linha);
        exit(1);
      }
      // printf("tem virgula\n");
      // printf("Token apos , %s\n", atual->token);
      return processarDeclaracao(atual, tipo_atual, escopo_atual);
    }
    return 1;
  }

  return 0;
}

///////////////VerificaSintaxeEhValida///////////////

int VerificaSintaxeEhValida(TokenNode *head) {

  TokenNode *current = head;
  while (current) {
    const char *token = current->token;
    // printf("Token: %s - Linha: %d\n", token, current->linha);
    if (strcmp(token, " ") == 0) {
      current = current->prox;
      continue;
    }
    if (!token || strlen(token) == 0) {
      current = current->prox;
      continue;
    }

    if (ehPalavraReservada(token)) {
      if (strcmp(token, "principal") == 0) {
        if (principalExiste) {
          imprimeMensagemErro("ERRO: Função 'principal' já declarada",
                              current->token, current->linha);
          return 0;
        } else {
          principalExiste = true;
          strcpy(escopo_atual, "principal");
          if (strcmp(current->prox->token, "(") != 0 ||
              strcmp(current->prox->prox->token, ")") != 0) {
            alertaSemantico("Função 'principal' não pode ter parametros",
                            current->linha);
          }
        }
      } else if (strcmp(token, "se") == 0 || strcmp(token, "senao") == 0 ||
                 strcmp(token, "para") == 0) {
        int num_linha = current->linha;
        TokenNode *currentAux = current->prox;

        // percorrer todos os tokens da linha
        while (currentAux && currentAux->linha == num_linha &&
               currentAux != NULL) {
          // se encontro um { na mesma linha que a palavra reservada, entao
          // adiciono um na flag para indicar que o proximo } nao deve alterar o
          // escopo
          if (strcmp(currentAux->token, "{") == 0) {
            flag_escopo_palavra_reservada++;
            currentAux = NULL;
          }
          currentAux = currentAux->prox;
        }
        if (strcmp(token, "se") == 0) {
          verificaSe(current);
        }
        if (strcmp(token, "para") == 0) {
          // printf("Verificando para \n");
          current = verificaPara(current);
        }
      } else if (strcmp(token, "leia") == 0) {
        verificaLeia(current);
      } else if (strcmp(token, "escreva") == 0) {
        verificaEscreva(current);
      } else if (strcmp(token, "funcao") == 0) {
        current = verificaFuncao(current);
      }
    } else if (ehTipoDeDado(token)) {
      strcpy(tipo_atual, token);
      processarDeclaracao(current, tipo_atual, escopo_atual);
    } else if (ehFuncao(token)) {
      strcpy(escopo_atual, token);
    } else if (ehVariavel(token)) {
      // se o proximo token for um operador de atribuição , eu pego o valor e
      // atribuo a variavel
      if (current->prox && ehOperador(current->prox) &&
          strcmp(current->prox->token, "=") == 0) {
        strcpy(nome_atual, token); // nome da variável

        valor_atual[0] = '\0'; // limpa buffer
        TokenNode *tmp = current->prox->prox;

        // Concatena até encontrar marcador (ex: ; , ) { } etc.)
        while (tmp && !ehMarcador(tmp->token)) {
          strcat(valor_atual, tmp->token);
          strcat(valor_atual, " "); // espaço entre tokens
          tmp = tmp->prox;
        }

        atribuirValor(tabela_simbolos, nome_atual, valor_atual, escopo_atual,tmp->linha);
      } else if (current->prox && ehMarcador(current->prox->token) &&
                 strcmp(current->prox->token, "[") == 0 &&
                 strcmp(current->prox->prox->prox->prox->token, "=") == 0) {

        strcpy(nome_atual, token); // nome da variável
        TokenNode *tmpNome = current->prox;

        while (tmpNome && strcmp(tmpNome->token, "]") != 0) {
          tmpNome = tmpNome->prox;
        }
        // printf("TmpNome: %s - Linha: %d\n", tmpNome->token, tmpNome->linha);
        valor_atual[0] = '\0'; // limpa buffer
        TokenNode *tmpAtribuiValor = tmpNome->prox;
        // printf("tmpAtribuiValor: %s - Linha: %d\n",tmpAtribuiValor->token,
        // tmpAtribuiValor->linha);
        if (tmpAtribuiValor && ehOperador(tmpAtribuiValor) &&
            strcmp(tmpAtribuiValor->token, "=") == 0) {
          strcpy(valor_atual, tmpAtribuiValor->prox->token);
        }

        // imprimirTabela(tabela_simbolos);
        atribuirValor(tabela_simbolos, nome_atual, valor_atual, escopo_atual,tmpAtribuiValor->linha);
      } /*
       //verifica se a variavel existe na tabela de simbolos
       if (buscarSimbolo(tabela_simbolos, token, escopo_atual) == NULL){
          printf("-={********************************************************}="
                  "-\n");
           printf("ERRO Semântico: Variável '%s' não declarada - LINHA %d\n",
       token, current->linha);
         printf("-={********************************************************}="
                  "-\n");
         exit(1);
       }*/

    } else if (ehMarcador(token) && strcmp(token, "}") == 0) {

      if (flag_escopo_palavra_reservada < 0) {
        strcpy(escopo_atual, "global");
        // adicionar funcao na tabela de simboloss
      } else {

        flag_escopo_palavra_reservada--;
      }
    } else if (ehOperador(current)) {
      if (ehOperador(current->prox)) {
        imprimeMensagemErro("ERRO: Sintaxe invalida", current->token,
                            current->linha);
        exit(1);
      }

    } else if (ehNumero(token) || ehString(token) || ehMarcador(token)) {
    } else {
      imprimeMensagemErro("ERRO: Sintaxe invalida", current->token,
                          current->linha);
      return 0;
    }
    current = current->prox;
  }
  return 1;
}