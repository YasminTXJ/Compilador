#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 1024
#define KB 1024
#define DEFAULT_MEMORY_LIMIT (2048 * KB)

// Variáveis globais para armazenar o último tipo e variável encontrados
char ultimo_tipo[20] = "";
char ultima_variavel[50] = "";
// Variáveis globais para armazenar o escopo atual e o escopo anterior
char escopo_atual[50] = "global";
char escopo_anterior[50] = "global";
int dentro_funcao = 0;

// Variáveis globais para controle de memória
size_t total_memory_used = 0;
size_t max_memory_used = 0;
size_t memory_limit = DEFAULT_MEMORY_LIMIT;

// Função para alocação de memória
void *safe_malloc(size_t size) {
  if (total_memory_used + size > memory_limit) {
    fprintf(stderr, "ERRO: Memória Insuficiente\n");
    exit(EXIT_FAILURE);
  }

  void *ptr = malloc(size);
  if (!ptr) {
    fprintf(stderr, "ERRO: Falha na alocação\n");
    exit(EXIT_FAILURE);
  }

  total_memory_used += size;
  if (total_memory_used > max_memory_used) {
    max_memory_used = total_memory_used;
  }

  return ptr;
}
// Função para realocação de memória
void *safe_realloc(void *ptr, size_t old_size, size_t new_size) {
  if (new_size > old_size &&
      total_memory_used + (new_size - old_size) > memory_limit) {
    fprintf(stderr, "ERRO: Memória Insuficiente\n");
    exit(EXIT_FAILURE);
  }

  void *new_ptr = realloc(ptr, new_size);
  if (!new_ptr) {
    fprintf(stderr, "ERRO: Falha na realocação\n");
    exit(EXIT_FAILURE);
  }

  total_memory_used += (new_size - old_size);
  if (total_memory_used > max_memory_used) {
    max_memory_used = total_memory_used;
  }

  return new_ptr;
}
// Função para liberação de memória
void safe_free(void *ptr, size_t size) {
  if (ptr) {
    free(ptr);
    if (total_memory_used >= size) {
      total_memory_used -= size;
    }
  }
}


// Estrutura para a tabela de símbolos
typedef struct Simbolo {
    char tipo[20];
    char nome[50];
    char valor[100];
    char escopo[50]; // Função / módulo
    struct Simbolo *prox;
} Simbolo;
Simbolo *tabela_simbolos = NULL;

// Função para criar um novo símbolo
Simbolo* criarSimbolo(const char *tipo, const char *nome, const char *valor, const char *escopo) {
    Simbolo *novo = (Simbolo *)safe_malloc(sizeof(Simbolo));
    strcpy(novo->tipo, tipo);
    strcpy(novo->nome, nome);
    strcpy(novo->valor, valor);
    strcpy(novo->escopo, escopo);
    novo->prox = NULL;
    return novo;
}
// Função para adicionar um símbolo à tabela
void adicionaSimbolo(const char *tipo, const char *nome, const char *valor, const char *escopo) {
    Simbolo *novo = criarSimbolo(tipo, nome, valor, escopo);

    // Inserção no início
    novo->prox = tabela_simbolos;
    tabela_simbolos = novo;
}

// Lista das palavras-chave, tipos e operadores
const char *keywords[] = {"principal", "funcao", "retorno", "leia",
                          "escreva",   "se",     "senao",   "para"};
const char *tipos[] = {"inteiro", "texto", "decimal"};
const char *operadores[] = {"==", "<>", "<=", ">=", "&&", "||", "+",
                            "-",  "*",  "/",  "^",  "<",  ">",  "="};

// Funções para verificar se um token é uma palavra-chave, tipo, operador, variável, função ou número
int isKeyword(const char *token) {
  for (int i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
    if (strcmp(token, keywords[i]) == 0)
      return 1;
  }
  return 0;
}

int isTipo(const char *token) {
  for (int i = 0; i < sizeof(tipos) / sizeof(tipos[0]); i++) {
    if (strcmp(token, tipos[i]) == 0)
      return 1;
  }
  return 0;
}

int isOperador(const char *token) {
  for (int i = 0; i < sizeof(operadores) / sizeof(operadores[0]); i++) {
    if (strcmp(token, operadores[i]) == 0)
      return 1;
  }
  return 0;
}

int isVariavel(const char *token) {
  return token[0] == '!' && islower(token[1]);
}

int isFuncao(const char *token) {
  return strncmp(token, "__", 2) == 0 &&
         (isalpha(token[2]) || isdigit(token[2]));
}

int isNumero(const char *token) {
  int ponto = 0;
  for (int i = 0; token[i]; i++) {
    if (token[i] == '.')
      ponto++;
    else if (!isdigit(token[i]))
      return 0;
  }
  return ponto <= 1;
}

// Função para verificar se um caractere é válido na tabela ASCII
int verificaAsciiValido(char c) { return (c >= 0 && c <= 127); }


// Função para classificar o token
void classificaToken(const char *token, int linha) {
    if (strlen(token) == 0)
        return;

    if (isKeyword(token)) {
      if (strcmp(token, "principal") == 0) {
          strcpy(escopo_atual, token);// atualiza o escopo atual para o nome da função principal
      }
       // printf("[LINHA %d] Palavra-chave: %s\n", linha, token);
    } else if (isTipo(token)) {
        //printf("[LINHA %d] Tipo de dado: %s\n", linha, token);
        strcpy(ultimo_tipo, token);
    } else if (isFuncao(token)) {
        //printf("[LINHA %d] Função: %s\n", linha, token);
        strcpy(escopo_anterior, escopo_atual);
        strcpy(escopo_atual, token);
        dentro_funcao = 1;
    } else if (isVariavel(token)) {
        //printf("[LINHA %d] Variável: %s\n", linha, token);
        strcpy(ultima_variavel, token);
    } else if (isNumero(token) || (token[0] == '"' && token[strlen(token) - 1] == '"')) {
        //printf("[LINHA %d] Valor: %s\n", linha, token);
        if (strlen(ultimo_tipo) > 0 && strlen(ultima_variavel) > 0) {
            adicionaSimbolo(ultimo_tipo, ultima_variavel, token, escopo_atual);
            ultimo_tipo[0] = '\0';
            ultima_variavel[0] = '\0';
        }
    } else if (strcmp(token, "{") == 0) {
        if (dentro_funcao) {
            // já está no escopo da função, não faz nada
        }
    } else if (strcmp(token, "}") == 0) {
        if (dentro_funcao) {
            strcpy(escopo_atual, "global");
            dentro_funcao = 0;
        }
    } else if (isOperador(token)) {
        //printf("[LINHA %d] Operador: %s\n", linha, token);
    } else {
        //printf("[LINHA %d] Token desconhecido: %s\n", linha, token);
    }
}




// Função para tokenizar uma linha
void tokenizeLine(const char *line, int num_linha) {
  printf("------------ ANALISANDO [LINHA %d] ------------\n", num_linha);
  char token[256];
  int i = 0, j = 0;
  int len = strlen(line);
  int dentroDeString = 0;

  while (i < len) {
    char c = line[i];
    // Verifica se é início de string
    if (c == '"') {
      dentroDeString = !dentroDeString; 
    }

    if (!dentroDeString && !verificaAsciiValido(c)) {
      printf("[LINHA %d] ERRO: Caractere inválido na tabela ASCII: %c (%d)\n",
             num_linha, c, c);
      i++;
      break;
    }

    switch (c) {
    case ' ':
    case '\t':
    case '\n':
      token[j] = '\0';
      classificaToken(token, num_linha);
      j = 0;
      i++;
      break;

    case '"': {
      j = 0;                  // Reinicia o índice do token
      token[j++] = line[i++]; // Adiciona a primeira aspas (") ao token

      // Começa a ler todos os caracteres até encontrar a próxima aspas
      while (i < len && line[i] != '"') {
        token[j++] = line[i++];
      }

      // Verifica se encontrou a aspas final
      if (i < len && line[i] == '"') {
        token[j++] = line[i++]; // Adiciona a aspas de fechamento ao token
        token[j] = '\0';        // Fecha a string C com '\0'
        classificaToken(token, num_linha); // Classifica a string completa
      } else {
        printf("[LINHA %d] ERRO: String não fechada\n", num_linha);
        break;
      }

      j = 0; // Limpa o token para o próximo
      break;
    }
    case '!': {
      token[j] = '\0';
      classificaToken(token, num_linha);
      j = 0;

      token[j++] = c; // '!'
      i++;

      // Acumula nome da variável
      while (i < len && isalnum(line[i])) {
        token[j++] = line[i++];
      }

      token[j] = '\0';
      classificaToken(token, num_linha);
      j = 0;
      break;
    }

    case '=':
    case '<':
    case '>':
    case '&':
    case '|': {
      token[j] = '\0';
      classificaToken(token, num_linha);
      j = 0;

      if ((c == '=' && line[i + 1] == '=') ||
          (c == '<' && line[i + 1] == '=') ||
          (c == '>' && line[i + 1] == '=') ||
          (c == '<' && line[i + 1] == '>') ||
          (c == '&' && line[i + 1] == '&') ||
          (c == '|' && line[i + 1] == '|')) {
        char op_duplo[3] = {c, line[i + 1], '\0'};
        classificaToken(op_duplo, num_linha);
        i += 2;
      } else {
        char op[2] = {c, '\0'};
        classificaToken(op, num_linha);
        i++;
      }
      break;
    }

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
      classificaToken(token, num_linha);
      j = 0;

      char simbolo[2] = {c, '\0'};
      classificaToken(simbolo, num_linha);
      i++;
      break;

    default:
      token[j++] = c;
      i++;
      break;
    }
  }

  token[j] = '\0';
  classificaToken(token, num_linha);
}




int main() {
  FILE *fp = fopen("Codigo1.txt", "r"); // Altere o caminho se necessário
  if (!fp) {
    perror("Erro ao abrir o arquivo");
    return 1;
  }
  // Aloca memória para armazenar cada linha lida
  char *linha = (char *)safe_malloc(MAX_LINE);
  int num_linha = 1;

  // Lê cada linha do arquivo e processa
  while (fgets(linha, MAX_LINE, fp)) {
    tokenizeLine(linha, num_linha);
    num_linha++;
  }

  // Após o loop, libera a memória e exibe o relatório de memória
  safe_free(linha, MAX_LINE);

  // Relatório de uso de memória
  printf("\n[RELATÓRIO FINAL DE MEMÓRIA]\n");
  printf("Máximo de memória usada: %.2f KB\n", (float)max_memory_used / KB);
  printf("Memória total usada: %.2f KB\n", (float)total_memory_used / KB);

  printf("\n[TABELA DE SÍMBOLOS]\n");
  printf("%-15s %-15s %-15s %-15s\n", "Tipo", "Nome", "Valor", "Escopo");

  Simbolo *atual = tabela_simbolos;
  while (atual) {
      printf("%-15s %-15s %-15s %-15s\n", atual->tipo, atual->nome, atual->valor, atual->escopo);
      atual = atual->prox;
  }
  
  fclose(fp);
  return 0;
}