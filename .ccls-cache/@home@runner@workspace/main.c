#include "ArquivosH/globals.h"
#include "ArquivosH/pilha.h"
#include "ArquivosH/tabelaSimbolos.h"
#include "ArquivosH/verificaSintaxe.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Funções de gerenciamento de memória
void *verificaSeTemMemoriaDisponivelEFazOMalloc(size_t size) {
  if (total_memory_used + size > memory_limit) {
    printf("-={********************************************************}=-\n");
    fprintf(stderr, "ERRO: Memória Insuficiente\n");
    printf("-={********************************************************}=-\n");
    exit(EXIT_FAILURE);
  }
  void *ptr = malloc(size);
  if (!ptr) {
    printf("-={********************************************************}=-\n");
    fprintf(stderr, "ERRO: Falha na alocação\n");
    printf("-={********************************************************}=-\n");
    exit(EXIT_FAILURE);
  }
  total_memory_used += size;
  if (total_memory_used > max_memory_used)
    max_memory_used = total_memory_used;
  return ptr;
}

void LiberaMallocELiberaMemoria(void *ptr, size_t size) {
  if (ptr) {
    free(ptr);
    if (total_memory_used >= size)
      total_memory_used -= size;
  }
  if (ptr == NULL) {
    if (total_memory_used >= size)
      total_memory_used -= size;
  }
}

// Funções utilitárias
char *safe_strdup(const char *s) {
  size_t size = strlen(s) + 1;
  char *dup = verificaSeTemMemoriaDisponivelEFazOMalloc(size);
  strcpy(dup, s);
  return dup;
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
      // printf("Achei um operador logico %s ",expr2->prox->token);
      TokenNode *res = verificaParDeExpressao(expr2->prox->prox);
      return res;
    } else {
      // retorna o token depois do )
      return expr2->prox->prox;
    }
  }
  printf("erro ao validar expressão do se Linha : %d\n", tk->linha);
  exit(1);
}
// 6) verifica se é bloco de várias linhas (abre com '{')
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

  // 4) agora validar o bloco verdadeiro
  TokenNode *blocoV = verificaParDeExpressao(expr);
  if (!blocoV) {
    printf("ERRO SINTÁTICO: esperado bloco após condição do 'se' - LINHA %d\n",
           expr->linha);
    exit(1);
  }

  // se for várias linhas → precisa abrir com {
  if (ehBlocoMultilinha(blocoV)) {
    if (strcmp(blocoV->token, "{") != 0) {
      printf("ERRO SINTÁTICO: bloco múltiplo sem '{' no 'se' - LINHA %d\n",
             blocoV->linha);
      exit(1);
    }
    // percorre até achar }
    TokenNode *cur = blocoV->prox;
    int encontrouFecha = 0;
    while (cur && strcmp(cur->token, "}") != 0) {
      if (ehTipoDeDado(cur->token)) {
        printf("ERRO SEMÂNTICO: declaração de variável não permitida dentro do "
               "'se' - LINHA %d\n",
               cur->linha);
        exit(1);
      }
      // pode chamar verificaSe recursivamente (se aninhado)
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
    encontrouFecha = 1;
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
    // mesmo processo de bloco do senao
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

// Função que retorna o nome base da variável (antes do '[')
char *getNomeBase(const char *token) {
  int len = 0;

  // Conta até o '[' ou fim da string
  while (token[len] != '\0' && token[len] != '[') {
    len++;
  }

  // Aloca memória para a string base (+1 para '\0')
  char *base = (char *)malloc(len + 1);
  if (!base) {
    perror("malloc falhou");
    exit(1);
  }

  // Copia os caracteres
  strncpy(base, token, len);
  base[len] = '\0'; // fecha a string

  return base; // quem chamar deve liberar a memória depois
}

// Simbolo *buscaFuncaoNaTabelaDeSimbolos(Simbolo *tabela, const char *nome) {}

// Lista encadeada de tokens
void addTokenToList(const char *token, int linha) {
  if (strlen(token) == 0)
    return;
  TokenNode *novo =
      (TokenNode *)verificaSeTemMemoriaDisponivelEFazOMalloc(sizeof(TokenNode));
  novo->token = safe_strdup(token);
  novo->linha = linha;
  novo->prox = NULL;

  if (!token_list)
    token_list = token_list_tail = novo;
  else {
    token_list_tail->prox = novo;
    token_list_tail = novo;
  }
  tokenCount++;
}

// Funções de tokenização
void tokenizeLine(const char *line, int num_linha) {
  char token[MAX_TOKEN_SIZE];
  int i = 0, j = 0, len = strlen(line), dentroDeString = 0;

  while (i < len) {
    char c = line[i];
    if (c == '"')
      dentroDeString = !dentroDeString;
    if (!dentroDeString && !verificaAsciiValido(c)) {
      printf("[LINHA %d] ERRO: %c\n", num_linha, c);
      i++;
      break;
    }

    switch (c) {
    case ' ':
    case '\t':
    case '\n':
    case '\r':
      token[j] = '\0';
      addTokenToList(token, num_linha);
      j = 0;
      i++;
      break;
    case '"':
      j = 0;
      token[j++] = line[i++];
      while (i < len && line[i] != '"')
        token[j++] = line[i++];
      if (i < len && line[i] == '"')
        token[j++] = line[i++];
      token[j] = '\0';
      addTokenToList(token, num_linha);
      j = 0;
      break;
    case '!':
      token[j] = '\0';
      addTokenToList(token, num_linha);
      j = 0;
      token[j++] = c;
      i++;
      while (i < len && isalnum(line[i]))
        token[j++] = line[i++];
      token[j] = '\0';
      addTokenToList(token, num_linha);
      j = 0;
      break;
    case '=':
    case '<':
    case '>':
    case '&':
    case '|':
      token[j] = '\0';
      addTokenToList(token, num_linha);
      j = 0;
      if ((c == '=' && line[i + 1] == '=') ||
          (c == '<' && (line[i + 1] == '=' || line[i + 1] == '>')) ||
          (c == '>' && line[i + 1] == '=') ||
          (c == '&' && line[i + 1] == '&') ||
          (c == '|' && line[i + 1] == '|')) {
        char op[3] = {c, line[i + 1], '\0'};
        addTokenToList(op, num_linha);
        i += 2;
      } else {
        char op[2] = {c, '\0'};
        addTokenToList(op, num_linha);
        i++;
      }
      break;
    case '+':
    case '-':
    case '*':
    case '/':
    case '^':
    case '(':
    case ')':
    case '{':
    case '}':
    case '[':
    case ']':
    case ';':
    case ',':
      token[j] = '\0';
      addTokenToList(token, num_linha);
      j = 0;
      char s[2] = {c, '\0'};
      addTokenToList(s, num_linha);
      i++;
      break;
    default:
      token[j++] = c;
      i++;
      break;
    }
  }
  token[j] = '\0';
  addTokenToList(token, num_linha);
}

TokenNode *tokenizeFile(const char *filename, int *num_tokens_ret) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    perror("Erro ao abrir arquivo");
    return NULL;
  }
  char line[1024];
  int linha = 1;
  while (fgets(line, sizeof(line), file))
    tokenizeLine(line, linha++);
  fclose(file);
  *num_tokens_ret = tokenCount;
  return token_list;
}

void freeTokenList(TokenNode *head) {
  while (head) {
    TokenNode *tmp = head;
    head = head->prox;
    LiberaMallocELiberaMemoria(tmp->token, strlen(tmp->token) + 1);
    LiberaMallocELiberaMemoria(tmp, sizeof(TokenNode));
  }
}

// Função de processamento de declaração
// Função para verificar se a string é um número inteiro
bool ehInteiro(const char *str) {
  if (str[0] == '\0')
    return false; // string vazia não é número

  for (int i = 0; str[i] != '\0'; i++) {
    if (!isdigit(str[i]))
      return false; // se tiver caractere que não é dígito
  }

  return true;
}

// Função para verificar se a string é um número decimal (com ponto)
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
// Função de processamento de declaração
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
        printf(
            "-={********************************************************}=-\n");
        printf("Alerta Semântico: variavél sem tamanho correto declarado - "
               "LINHA %d\n",
               atual->linha);
        printf(
            "-={********************************************************}=-\n");
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
        printf(
            "-={********************************************************}=-\n");
        printf("Alerta Semântico: variavél declara sem tamanho -  '%s' não é "
               "um número - LINHA %d\n",
               atual->token, atual->linha);
        printf(
            "-={********************************************************}=-\n");
      }
    } else {
      printf(
          "-={********************************************************}=-\n");
      printf("Alerta Semântico: variavél sem tamanho correto declarado - LINHA "
             "%d\n",
             atual->linha);
      printf(
          "-={********************************************************}=-\n");
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
        printf("-={********************************************************}="
               "-\n");
        printf("ERRO Sintático: Não se pode declarar dois tipos de dados na "
               "mesma linha - '%s' não pode ser declarada - LINHA %d\n",
               atual->prox->token, atual->linha);
        printf("-={********************************************************}="
               "-\n");
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
    printf("-={********************************************************}="
           "-\n");
    printf(
        "ERRO Sintático: Comando 'escreva' sem variável ou texto - LINHA %d\n",
        prox->linha);
    printf("-={********************************************************}="
           "-\n");
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

TokenNode *verificaParDeExpressaoPara(TokenNode *tk) {
  TokenNode *expr = tk;
  if (!expr) {
    printf("ERRO SINTÁTICO: condição vazia no 'para' - LINHA %d\n", tk->linha);
    exit(1);
  }
  // operando 1
  //printf("Operando 1 do para: %s - Linha: %d\n", expr->token, expr->linha);
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
  }else{
     return(expr2);
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
    //printf("Token PROX: %s - Linha: %d\n", tk->prox->token, tk->prox->linha);
    if (!tk->prox || strcmp(tk->prox->token, "(") != 0) {
        printf("ERRO SINTÁTICO: esperado '(' após 'para' - LINHA %d\n", tk->linha);
        exit(1);
    }

    TokenNode *expr = tk->prox->prox;
   //printf("Token prox prox : %s - Linha: %d\n", expr->token, expr->linha);  
     //pego o primeiro token depois da virgula

    // -------- X1: inicialização --------
     //verifica se já foi declarado
     if(buscarSimbolo(tabela_simbolos, expr->token, escopo_atual) == NULL){
        printf("ERRO SEMÂNTICO: variável '%s' não declarada no 'para' - LINHA %d\n", expr->token, expr->linha);
        exit(1);
     }

    // pode ser vazio (quando vem direto um ";")
    if (strcmp(expr->token, ";") != 0) {
        while (expr && strcmp(expr->token, ";") != 0) {
            if (!ehVariavel(expr->token) && strcmp(expr->token, "=") != 0 &&
                !ehNumero(expr->token) && strcmp(expr->token, ",") != 0) {
                printf("ERRO SINTÁTICO: inicialização inválida em 'para' - LINHA %d\n", expr->linha);
                exit(1);
            }
            expr = expr->prox;
        }
    }
    //printf("Token que deveria ser ; = %s - Linha: %d\n", expr->token, expr->linha);
    if (!expr || strcmp(expr->token, ";") != 0) {
        printf("ERRO SINTÁTICO: esperado ';' após inicialização no 'para' - LINHA %d\n", expr ? expr->linha : tk->linha);
        exit(1);
    }
    expr = expr->prox;
  // -------- X2: condição --------
   // printf("Inicio da condição  = %s - Linha: %d\n", expr->token, expr->linha);
    //verifica se já foi declarado
     if(ehVariavel(expr->token)){
       if(buscarSimbolo(tabela_simbolos, expr->token, escopo_atual) == NULL){
           printf("ERRO SEMÂNTICO: variável '%s' não declarada no 'para' - LINHA %d\n", expr->token, expr->linha);
           exit(1);
        }
     }else{
       printf("ERRO SINTÁTICO: condição vazia no 'para' - LINHA %d\n", expr->linha);
       exit(1);
     }


    if (strcmp(expr->token, ";") != 0 ) {
        expr = verificaParDeExpressaoPara(expr);
    } else {
        printf("ERRO SINTÁTICO: condição vazia no 'para' - LINHA %d\n", expr->linha);
        exit(1);
    }
  expr = expr->prox;
    if (!expr || strcmp(expr->token, ";") != 0) {
        printf("ERRO SINTÁTICO: esperado ';' após condição no 'para' - LINHA %d\n", expr ? expr->linha : tk->linha);
        exit(1);
    }
    expr = expr->prox;

    // -------- X3: incremento --------
    //printf("Inicio do incremento  = %s - Linha: %d\n", expr->token, expr->linha);
    if (strcmp(expr->token, ")") != 0) {
        while (expr && strcmp(expr->token, ")") != 0) {
          //printf("EXPRESSAO: %s - LINHA: %d\n", expr->token, expr->linha);
            if (!(ehVariavel(expr->token) || ehNumero(expr->token) ||
                  strcmp(expr->token, "+") == 0 || strcmp(expr->token, "-") == 0 ||
                  ehOperador(expr))) {
                printf("ERRO SINTÁTICO: incremento inválido em 'para' - LINHA %d\n", expr->linha);
                exit(1);
            }
          if(ehVariavel(expr->token)){
                if(buscarSimbolo(tabela_simbolos, expr->token, escopo_atual) == NULL){
                    printf("ERRO SEMÂNTICO: variável '%s' não declarada no 'para' - LINHA %d\n", expr->token, expr->linha);
                    exit(1);
                }
          }
            expr = expr->prox;
        }
    }else{
        printf("ERRO SINTÁTICO: incremento vazio no 'para' - LINHA %d\n", expr->linha);
        exit(1);
    }
    if (!expr || strcmp(expr->token, ")") != 0) {
        printf("ERRO SINTÁTICO: esperado ')' após incremento no 'para' - LINHA %d\n", expr ? expr->linha : tk->linha);
        exit(1);
    }
    TokenNode *tokenAposPara = expr;
    expr = expr->prox;
 // printf("Saindo do para  = %s - Linha: %d\n", expr->token, expr->linha);
    // -------- Corpo do laço --------
    if (!expr || strcmp(expr->prox->token, "}")==0) {
        printf("ERRO SINTÁTICO: esperado corpo após 'para' - LINHA %d\n", tk->linha);
        exit(1);
    }
    if (ehBlocoMultilinha(expr)) {
        TokenNode *cur = expr->prox;
        while (cur && strcmp(cur->token, "}") != 0) {
            if (ehTipoDeDado(cur->token)) {
                printf("ERRO SEMÂNTICO: declaração não permitida dentro do 'para' - LINHA %d\n", cur->linha);
                exit(1);
            }
            if (strcmp(cur->token, "para") == 0) {
                verificaPara(cur); // suporte a para aninhado
            }
            cur = cur->prox;
        }
        if (!cur) {
            printf("ERRO SINTÁTICO: bloco do 'para' não fechado com '}' - LINHA %d\n", expr->linha);
            exit(1);
        }
    } 
    return tokenAposPara; // passou em tudo


}
// Verificação de sintaxe usando lista encadeada
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
          printf("-={********************************************************}="
                 "-\n");
          printf("ERRO: Função 'principal' já declarada - LINHA %d\n",
                 current->linha);
          printf("-={********************************************************}="
                 "-\n");
          return 0;
        } else {
          principalExiste = true;
          strcpy(escopo_atual, "principal");
          if(strcmp(current->prox->token, "(") != 0 || strcmp(current->prox->prox->token, ")") != 0){
            printf("-={********************************************************}="
                 "-\n");
            printf("Alerta Semântico: Função 'principal' não pode ter parametros - LINHA %d\n",
                 current->linha);
            printf("-={********************************************************}="
                 "-\n");
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
        if(strcmp(token, "para") == 0){
         // printf("Verificando para \n");
          current = verificaPara(current);
        }
      } else if (strcmp(token, "leia") == 0) {
        verificaLeia(current);
      } else if (strcmp(token, "escreva") == 0) {
        verificaEscreva(current);
      }
    } else if (ehTipoDeDado(token)) {
      strcpy(tipo_atual, token);
      processarDeclaracao(current, tipo_atual, escopo_atual);
    } else if (ehFuncao(token)) {
      strcpy(tipo_atual, "funcao");
      processarDeclaracao(current, tipo_atual, escopo_atual);
      adicionarSimbolo(tabela_simbolos, tipo_atual, nome_atual, valor_atual,
                       escopo_atual);
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

        atribuirValor(tabela_simbolos, nome_atual, valor_atual, escopo_atual);
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
        atribuirValor(tabela_simbolos, nome_atual, valor_atual, escopo_atual);
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
        printf(
            "-={********************************************************}=-\n");
        printf("ERRO: Sintaxe invalida ('%s') - LINHA %d\n", token,
               current->linha);
        printf(
            "-={********************************************************}=-\n");
        exit(1);
      }

    } else if (ehNumero(token) || ehString(token) || ehMarcador(token)) {
    } else {
      printf("Entrei aqui aaaaaaaaaa");
      printf(
          "-={********************************************************}=-\n");
      printf("ERRO: Sintaxe invalida ('%s') - LINHA %d\n", token,
             current->linha);
      printf(
          "-={********************************************************}=-\n");
      return 0;
    }
    current = current->prox;
  }
  return 1;
}

int verificarBalanceamento(TokenNode *token_list) {
  Pilha *pilha = NULL;
  TokenNode *current = token_list;

  while (current) {
    char *tok = current->token;
    int linha = current->linha;

    // Abre delimitadores
    if (strcmp(tok, "(") == 0 || strcmp(tok, "{") == 0 ||
        strcmp(tok, "[") == 0) {
      pilha = push(pilha, tok[0], linha);
    }

    // Fecha delimitadores
    else if (strcmp(tok, ")") == 0 || strcmp(tok, "}") == 0 ||
             strcmp(tok, "]") == 0) {
      if (isEmpty(pilha)) {
        printf(
            "-={********************************************************}=-\n");
        printf(
            "\t\tERRO na linha %d: delimitador de fechamento '%s' sem par!\n",
            linha, tok);
        printf(
            "-={********************************************************}=-\n");
        return 0;
      }

      char aberto;
      int linha_aberto;
      pilha = pop(pilha, &aberto, &linha_aberto);

      if ((aberto == '(' && tok[0] != ')') ||
          (aberto == '{' && tok[0] != '}') ||
          (aberto == '[' && tok[0] != ']')) {
        printf(
            "-={********************************************************}=-\n");
        printf("\t\tERRO: delimitador '%c' aberto na linha %d foi fechado "
               "incorretamente com '%s' na linha %d\n",
               aberto, linha_aberto, tok, linha);
        printf(
            "-={********************************************************}=-\n");
        return 0;
      }
    }

    current = current->prox;
  }

  // Se sobrar algo na pilha → não foi fechado
  while (!isEmpty(pilha)) {
    char aberto;
    int linha_aberto;
    pilha = pop(pilha, &aberto, &linha_aberto);
    printf("-={********************************************************}=-\n");
    printf("ERRO: delimitador '%c' aberto na linha %d não foi fechado!\n",
           aberto, linha_aberto);
    printf("-={********************************************************}=-\n");
    return 0;
  }

  return 1; // OK se não houve erro
}

int VerificaSeTemPontoVirgulaNoFimdaLinha(TokenNode *token_list) {
  TokenNode *current = token_list;

  while (current != NULL) {
    TokenNode *linhaInicio = current;
    TokenNode *anterior = current;

    // anda até o fim da linha
    while (current->prox && current->prox->linha == linhaInicio->linha) {
      anterior = current;
      current = current->prox;
    }

    // agora "current" é o último token da linha
    // e "anterior" é o penúltimo
    if (strcmp(linhaInicio->token, "principal") != 0 &&
        strcmp(linhaInicio->token, "se") != 0 &&
        strcmp(linhaInicio->token, "senao") != 0 &&
        strcmp(linhaInicio->token, "para") != 0 &&
        strcmp(linhaInicio->token, "funcao") != 0 &&
        strcmp(linhaInicio->token, "}") != 0 &&
        strcmp(linhaInicio->token, "{") != 0) {

      if (strcmp(current->token, ";") != 0) {
        printf(
            "-={********************************************************}=-\n");
        printf("ERRO: Sintaxe invalida  - LINHA %d  (faltando ';')\n",
               current->linha);
        printf(
            "-={********************************************************}=-\n");
        exit(1);
      }
    }

    current = current->prox; // vai pra próxima linha
  }

  return 0;
}
// Main
int main() {
  int numTokens;
  TokenNode *resultado = tokenizeFile("Arquivos/Codigo1.txt", &numTokens);
  if (VerificaSeTemPontoVirgulaNoFimdaLinha(resultado) == 0) {
    if (verificarBalanceamento(resultado)) {
      if (VerificaSintaxeEhValida(resultado)) {
        printf(
            "-={********************************************************}=-\n");
        printf("\t\tCodigo Compilado com Sucesso\n");
        printf(
            "-={********************************************************}=-\n");
        imprimirTabela(tabela_simbolos);
      }
    }
  }

  printf("\n[Informacoes]\nTokens encontrados: %d\n", numTokens);

  freeTokenList(resultado);

  printf("\n[MEMORIA]\nMaximo de memoria usada: %.2f KB\n",
         (float)max_memory_used / KB);

  return 0;
}
