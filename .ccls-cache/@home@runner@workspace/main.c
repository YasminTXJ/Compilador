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

int ehOperadorLogico(const char *token) {
  return (
      strcmp(token, "&&") == 0 ||
      strcmp(token, "||") == 0
  );
}

void verificaExpressoesLogicas(TokenNode *lista) {
  TokenNode *atual = lista;

  int achouRelacional = 0;

  while (atual != NULL) {
      // Se encontrar operador relacional
      if (ehOperadorRelacional(atual->token)) {
          achouRelacional = 1;
      }

      // Se encontrar operador lógico
      else if (ehOperadorLogico(atual->token)) {
          if (!achouRelacional) {
              printf("-={********************************************************}=-\n");
              printf("Alerta semântico (linha %d): Operador lógico '%s' usado sem expressão relacional à esquerda.\n",
                     atual->linha, atual->token);
              printf("-={********************************************************}=-\n");
          }

          // Agora percorre lado direito até marcador
          int ladoDireitoTemRelacional = 0;
          TokenNode *p = atual->prox;

          while (p != NULL && !ehMarcador(p->token)) {
              if (ehOperadorRelacional(p->token)) {
                  ladoDireitoTemRelacional = 1;
              }
              p = p->prox;
          }

          if (!ladoDireitoTemRelacional) {
              printf("-={********************************************************}=-\n");
              printf("Alerta semântico (linha %d): Operador lógico '%s' usado sem expressão relacional à direita.\n",
                     atual->linha, atual->token);
              printf("-={********************************************************}=-\n");
          }

          // Depois do marcador, zera flag e recomeça
          achouRelacional = 0;
      }

      // Se encontrar marcador, zera flag
      else if (ehMarcador(atual->token)) {
          achouRelacional = 0;
      }

      atual = atual->prox;
  }
}
// Main
int main() {
  int numTokens;
  TokenNode *resultado = tokenizeFile("Arquivos/Codigo1.txt", &numTokens);
  if (VerificaSeTemPontoVirgulaNoFimdaLinha(resultado) == 0) {
    if (verificarBalanceamento(resultado)) {
      verificaExpressoesLogicas(resultado);
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
