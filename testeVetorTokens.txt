#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TOKEN_SIZE 256

// Verifica se caractere ASCII é válido (pode personalizar)
int verificaAsciiValido(char c) { return (c >= 0 && c <= 127); }

// Vetor dinâmico de strings
char **tokens = NULL;
int tokenCount = 0;

// Adiciona token ao vetor
void addToken(const char *token) {
  if (strlen(token) == 0)
    return; // Ignora tokens vazios

  tokens = realloc(tokens, (tokenCount + 1) * sizeof(char *));
  tokens[tokenCount] = strdup(token);
  tokenCount++;
}

// Tokeniza uma única linha
void tokenizeLine(const char *line, int num_linha) {
  char token[MAX_TOKEN_SIZE];
  int i = 0, j = 0;
  int len = strlen(line);
  int dentroDeString = 0;

  while (i < len) {
    char c = line[i];
    //printf("caracter : %s \n ", token);
    if (c == '"')
      dentroDeString = !dentroDeString;

    if (!dentroDeString && !verificaAsciiValido(c)) {
      printf("[LINHA %d] ERRO: Caractere inválido: %c (%d)\n", num_linha, c, i);
      i++;
      break;
    }

    switch (c) {
    case ' ':
    case '\t':
    case '\n':
      token[j] = '\0';
      addToken(token);
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
      addToken(token);
      j = 0;
      break;

    case '!':
      token[j] = '\0';
      addToken(token);
      j = 0;
      token[j++] = c;
      i++;
      while (i < len && isalnum(line[i]))
        token[j++] = line[i++];
      token[j] = '\0';
      addToken(token);
      j = 0;
      break;

    case '=':
    case '<':
    case '>':
    case '&':
    case '|':
      token[j] = '\0';
      addToken(token);
      j = 0;
      if ((c == '=' && line[i + 1] == '=') ||
          (c == '<' && (line[i + 1] == '=' || line[i + 1] == '>')) ||
          (c == '>' && line[i + 1] == '=') ||
          (c == '&' && line[i + 1] == '&') ||
          (c == '|' && line[i + 1] == '|')) {
        char op_duplo[3] = {c, line[i + 1], '\0'};
        addToken(op_duplo);
        i += 2;
      } else {
        char op[2] = {c, '\0'};
        addToken(op);
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
      addToken(token);
      j = 0;
      char simbolo[2] = {c, '\0'};
      addToken(simbolo);
      i++;
      break;

    default:
      token[j++] = c;
      i++;
      break;
    }
  }

  token[j] = '\0';
  addToken(token);
}

// Lê arquivo, processa e retorna vetor de tokens
char **tokenizeFile(const char *filename, int *num_tokens_ret) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    perror("Erro ao abrir arquivo");
    return NULL;
  }

  char line[1024];
  int linha = 1;
  while (fgets(line, sizeof(line), file)) {
    tokenizeLine(line, linha++);
  }

  fclose(file);
  *num_tokens_ret = tokenCount;
  return tokens;
}

// Exemplo de uso
int main() {
  int numTokens;
  char **resultado = tokenizeFile("Codigo2.txt", &numTokens);

  printf("Tokens encontrados: %d\n", numTokens);
  for (int i = 0; i < numTokens; i++) {
    printf("Token[%d]: %s\n", i, resultado[i]);
    free(resultado[i]); // Liberar token individual
  }
  free(resultado); // Liberar vetor de ponteiros

  return 0;
}
