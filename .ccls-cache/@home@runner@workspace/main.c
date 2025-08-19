// Alunos: YASMIN TEIXEIRA DE JESUS E DANIEL ANTÔNIO MIRANDA

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TOKEN_SIZE 256
#define MAX_LINE 1024
#define KB 1024
#define DEFAULT_MEMORY_LIMIT (2048 * KB)

// Prototipação
// Transforma Arquivo txt em Tokens
char **tokenizeFile(const char *filename, int *num_tokens_ret);
void tokenizeLine(const char *line, int num_linha);
int verificaAsciiValido(char c);
void addToken(const char *token);

void *verificaSeTemMemoriaDisponivelEFazOMalloc(size_t size);
void LiberaMallocELiberaMemoria(void *ptr, size_t size);

int VerificaSintaxeEhValida(char **tokens, int numTokens);
int ehPalavraReservada(const char *token);
int ehTipoDeDado(const char *token);
int ehFuncao(const char *token);
int ehVariavel(const char *token);
int VerificaSeNomeDeVarOuFuncEhValido(const char *token);
int ehNumero(const char *token);
int ehOperador(const char *token);
int ehMarcador(const char *token);
int ehString(const char *token);

char *safe_strdup(const char *s);

// Variáveis globais de memória
size_t total_memory_used = 0;
size_t max_memory_used = 0;
size_t memory_limit = DEFAULT_MEMORY_LIMIT;
char escopo_atual[50] = "global";
// Tokens fixos (constantes literais — não precisam de malloc)
const char *palavrasReservadas[] = {"principal", "funcao", "retorno", "leia",
                                    "escreva",   "se",     "senao",   "para"};
const char *tiposDeDados[] = {"inteiro", "texto", "decimal"};
const char *operadores[] = {"==", "<>", "<=", ">=", "&&", "||", "+",
                            "-",  "*",  "/",  "^",  "<",  ">",  "="};
const char *marcadores[] = {"(", ")", "{", "}", "[", "]", ";", ",", " "};
// Estrutura da tabela de símbolos
typedef struct Simbolo {
  char tipo[20];
  char nome[50];
  char valor[100];
  char escopo[50];
  struct Simbolo *prox;
} Simbolo;
Simbolo *tabela_simbolos = NULL;
char tipo_atual[20];
char nome_atual[50];
char valor_atual[100];

// Função para buscar símbolo
Simbolo *buscarSimbolo(const char *nome, const char *escopo) {
  Simbolo *atual = tabela_simbolos;
  while (atual != NULL) {
    if (strcmp(atual->nome, nome) == 0 && strcmp(atual->escopo, escopo) == 0) {
      return atual;
    }
    atual = atual->prox;
  }
  return NULL;
}
Simbolo *adicionarSimbolo(Simbolo *inicio, const char *tipo, const char *nome,
                          const char *valor, const char *funcao) {
  Simbolo *novo = (Simbolo *)malloc(sizeof(Simbolo));
  if (!novo) {
    printf("Memória insuficiente!\n");
    exit(1);
  }
  strcpy(novo->tipo, tipo);
  strcpy(novo->nome, nome);
  strcpy(novo->valor, valor);
  strcpy(novo->escopo, funcao);
  novo->prox = NULL;

  if (!inicio) {
    return novo;
  }

  Simbolo *temp = inicio;
  while (temp->prox)
    temp = temp->prox;
  temp->prox = novo;
  return inicio;
}
void imprimirTabela(Simbolo *inicio) {
  printf("\n%-10s %-15s %-15s %-20s\n", "Tipo", "Nome", "Valor", "Função");
  printf("-------------------------------------------------------------\n");
  while (inicio) {
    printf("%-10s %-15s %-15s %-20s\n", inicio->tipo, inicio->nome,
           inicio->valor, inicio->escopo);
    inicio = inicio->prox;
  }
}

// Vetor dinâmico de tokens
char **tokens = NULL;
int tokenCount = 0;

//******-------------------------------------------------**********************----------------------------------------------------
//           Transforma Arquivo TXT em tokens
//******-------------------------------------------------**********************----------------------------------------------------

char **tokenizeFile(const char *filename, int *num_tokens_ret) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    perror("Erro ao abrir arquivo");
    return NULL;
  }

  char line[1024];
  verificaSeTemMemoriaDisponivelEFazOMalloc(sizeof(line));
  int linha = 1;
  verificaSeTemMemoriaDisponivelEFazOMalloc(sizeof(linha));
  while (fgets(line, sizeof(line), file)) {
    tokenizeLine(line, linha++);
  }

  fclose(file);
  *num_tokens_ret = tokenCount;
  return tokens;
}

void tokenizeLine(const char *line, int num_linha) {
  char token[MAX_TOKEN_SIZE]; // Buffer  para montar tokens à medida que os
                              // caracteres são lidos.

  int i = 0, j = 0;       // i percorre a linha do arquivo, j percorre o token
  int len = strlen(line); // tamanho da linha
  int dentroDeString = 0;
  verificaSeTemMemoriaDisponivelEFazOMalloc(sizeof(i));
  verificaSeTemMemoriaDisponivelEFazOMalloc(sizeof(j));
  verificaSeTemMemoriaDisponivelEFazOMalloc(sizeof(len));
  verificaSeTemMemoriaDisponivelEFazOMalloc(sizeof(dentroDeString));

  while (i < len) {
    char c = line[i];
    verificaSeTemMemoriaDisponivelEFazOMalloc(sizeof(c));

    if (c == '"')
      dentroDeString = !dentroDeString; // verifica se está entre aspas
    // se estiver entre aspas não verifia se são validos na ASCII

    if (!dentroDeString && !verificaAsciiValido(c)) {
      printf("[LINHA %d] ERRO: Caractere inválido: %c \n", num_linha, c);
      i++;
      break;
    }

    switch (c) {
    case ' ':
    case '\t':
    case '\n':
    case '\r':
      token[j] = '\0';
      addToken(
          token); // se for espaço, tab ou quebra de linha, adiciona o token
      j = 0;
      i++;
      break;

    case '"':
      j = 0;
      token[j++] = line[i++]; // Adiciona o caractere de aspas ao token
      // Adiciona os caracteres até encontrar a próxima aspas
      while (i < len && line[i] != '"')
        token[j++] = line[i++];
      if (i < len && line[i] == '"')
        token[j++] = line[i++];
      token[j] = '\0';
      addToken(token); // Adiciona o token completo
      j = 0;
      break;

    case '!':
      token[j] = '\0';
      addToken(token); // Finaliza qualquer token anterior
      j = 0;
      token[j++] = c; // Inicia um novo token com o caractere '!'
      i++;
      // Adiciona os caracteres até encontrar um caractere que não seja letra ou
      // número
      while (i < len && isalnum(line[i]))
        token[j++] = line[i++];
      token[j] = '\0';
      addToken(token); // adiciona o token completo
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
        addToken(op_duplo); // adiciona o operador duplo
        i += 2;
      } else {
        char op[2] = {c, '\0'};
        addToken(op); // adiciona o operador simples
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
      addToken(simbolo); // adiciona o simbolo
      i++;
      break;

    default:
      token[j++] = c; // continua montando o token
      i++;
      break;
    }
  }

  LiberaMallocELiberaMemoria(NULL, sizeof(int));
  LiberaMallocELiberaMemoria(NULL, sizeof(i));
  LiberaMallocELiberaMemoria(NULL, sizeof(j));
  LiberaMallocELiberaMemoria(NULL, sizeof(len));
  LiberaMallocELiberaMemoria(NULL, sizeof(dentroDeString));
  token[j] = '\0'; // Finaliza o token atual
  addToken(token);
}

int verificaAsciiValido(char c) { return (c >= 0 && c <= 127); }

void addToken(const char *token) {
  // printf("token = %s\n", token);
  if (strlen(token) == 0)
    return;
  verificaSeTemMemoriaDisponivelEFazOMalloc(
      sizeof(char *)); // Realoca o vetor de tokens para comportar mais
                       // um ponteiro (mais um token).
  tokens = realloc(
      tokens, (tokenCount + 1) *
                  sizeof(char *)); // realoca o vetor para acomodar o novo token
  tokens[tokenCount] =
      safe_strdup(token); // Duplica a string `token` usando `safe_strdup` (que
                          // aloca memória e copia a string original).
  tokenCount++;
}

char *safe_strdup(const char *s) {
  size_t size = strlen(s) + 1;
  char *dup = verificaSeTemMemoriaDisponivelEFazOMalloc(size);
  strcpy(dup, s);
  return dup;
}

//******-------------------------------------------------**********************----------------------------------------------------
//           Funcoes que fazem a verificacao de sintaxe
//******-------------------------------------------------**********************----------------------------------------------------
int ehPalavraReservada(const char *token) {
  for (int i = 0;
       i < sizeof(palavrasReservadas) / sizeof(palavrasReservadas[0]); i++) {
    if (strcmp(token, palavrasReservadas[i]) == 0)
      return 1;
  }
  return 0;
}

int ehTipoDeDado(const char *token) {
  for (int i = 0; i < sizeof(tiposDeDados) / sizeof(tiposDeDados[0]); i++) {
    if (strcmp(token, tiposDeDados[i]) == 0)
      return 1;
  };
  return 0;
}
int ehString(const char *token) {
  if (token[0] == '"' && token[strlen(token) - 1] == '"') {
    return 1;
  } else {
    return 0;
  }
}

int ehFuncao(const char *token) {
  if (strncmp(token, "__", 2) == 0 &&
      VerificaSeNomeDeVarOuFuncEhValido(token + 2)) {
    return 1;
  } else {
    return 0;
  }
}

int ehVariavel(const char *token) {
  if (token[0] == '!' && islower(token[1])) {
    if (strlen(token) > 2) {
      if (VerificaSeNomeDeVarOuFuncEhValido(token + 2)) {
        return 1;
      } else {
        return 0;
      }
    } else {
      return 1;
    }
  } else {
    return 0;
  }
}

int VerificaSeNomeDeVarOuFuncEhValido(const char *token) {
  for (int i = 0; token[i] != '\0'; i++) {
    if (!isalpha(token[i]) && !isdigit(token[i])) {
      return 0; // found something that is not a letter or digit
    }
  }
  return 1;
}

int ehNumero(const char *token) {
  verificaSeTemMemoriaDisponivelEFazOMalloc(sizeof(int));
  int ponto = 0;
  for (int i = 0; token[i]; i++) {
    if (token[i] == '.')
      ponto++;
    else if (!isdigit(token[i]))
      return 0;
  }
  LiberaMallocELiberaMemoria(NULL, sizeof(int));
  return ponto <= 1;
}

int ehOperador(const char *token) {
  for (int i = 0; i < sizeof(operadores) / sizeof(operadores[0]); i++) {
    if (strcmp(token, operadores[i]) == 0) {
      return 1;
    };
  }
  return 0;
}

int ehMarcador(const char *token) {
  for (int i = 0; i < sizeof(marcadores) / sizeof(marcadores[0]); i++) {
    if (strcmp(token, marcadores[i]) == 0)
      return 1;
  }
  return 0;
}
int processarDeclaracao(char **tokens, int i, char *tipo_atual,
                        char *escopo_atual) {

  // Pega o nome da variável
  if (tokens[i + 1] != NULL && ehVariavel(tokens[i + 1])) {
    strcpy(nome_atual, tokens[i + 1]);
    printf("nome atual = %s\n", nome_atual);
  }

  // Verifica se é decimal (ex: car[2.5])
  if (tokens[i + 2] != NULL && ehMarcador(tokens[i + 2]) &&
      strcmp(tokens[i + 2], "[") == 0) {
    strcat(nome_atual, tokens[i + 2]); // Adiciona o '['
    int j = i + 3;
    while (tokens[j] != NULL && strcmp(tokens[j], "]") != 0) {
      strcat(nome_atual, tokens[j]); // Adiciona conteúdo entre os colchetes
      j++;
    }
    if (tokens[j] != NULL && strcmp(tokens[j], "]") == 0) {
      strcat(nome_atual, "]"); // Fecha o colchete
      i = j - 1;               // Avança o índice para depois do colchete
    }
  }

  // Verifica se há um operador "=" após o nome
  if (tokens[i + 2] != NULL && ehOperador(tokens[i + 2]) &&
      strcmp(tokens[i + 2], "=") == 0) {
    printf("eh operador = %s\n", tokens[i + 2]);
    strcpy(valor_atual, tokens[i + 3]);

    int j = i + 4;
    while (tokens[j] != NULL && !ehMarcador(tokens[j])) {
      strcat(valor_atual, tokens[j]); // Concatena valor
      j++;
    }

    printf("valoooor atual = %s\n", valor_atual);
    tabela_simbolos = adicionarSimbolo(tabela_simbolos, tipo_atual, nome_atual,
                                       valor_atual, escopo_atual);
    printf("adicionado na tabela de simbolos\n");
    return j - 1;

  } else if (tokens[i + 2] != NULL && ehMarcador(tokens[i + 2])) {
    printf("token , = %s\n", tokens[i + 2]);
    tabela_simbolos = adicionarSimbolo(tabela_simbolos, tipo_atual, nome_atual,
                                       "", escopo_atual);
    printf("adicionado na tabela de simbolos sem valor\n");

    // verifica se tem virgula e continua a declaracao de mais variaveis
    if (tokens[i + 2] != NULL && strcmp(tokens[i + 2], ",") == 0) {
      printf("tem virgula\n ");
      int valor = processarDeclaracao(tokens, i + 2, tipo_atual, escopo_atual);
      return valor;
    }
    return i + 2;
  }

  return i;
}

int VerificaSintaxeEhValida(char **tokens, int numTokens) {
  int linhaAtual = 1;

  for (int i = 0; i < numTokens; i++) {
    const char *token = tokens[i];
    printf("Analisando token : %s\n", token);
    if (strcmp(token, " ") == 0) {
      continue;
    }
    if (!token || strlen(token) == 0) {
      printf("continuei\n");
      continue;
    }

    if (strcmp(token, ";") == 0)
      linhaAtual++;

    if (ehPalavraReservada(token)) {
      if (strcmp(token, "principal") == 0) {
        strcpy(escopo_atual, "principal");
        printf("ecopo atual = %s\n", escopo_atual);
      }
      // printf("eh palavra reservada - token %s\n", token);
    } else if (ehTipoDeDado(token)) {
      strcpy(tipo_atual, token);
      printf("tipo atual = %s\n", tipo_atual);
      processarDeclaracao(tokens, i, tipo_atual, escopo_atual);
      // variavel declarada
      /*
      if (tokens[i + 1] != NULL && ehVariavel(tokens[i + 1])) {
        strcpy(nome_atual, tokens[i + 1]);

      }
      //se for decimal car[2.5]
      if(tokens[i + 2] != NULL && ehMarcador(tokens[i + 2]) &&
          strcmp(tokens[i + 2], "[") == 0){
          strcat(nome_atual, tokens[i+2]);
          while (tokens[i + 3] != NULL && strcmp(tokens[i + 3], "]") == 1){
            strcat(nome_atual, tokens[i+3]);  // pego o nome com o tamanho
            i++;
          }
      }
      if (tokens[i + 2] != NULL && ehOperador(tokens[i + 2]) &&
          strcmp(tokens[i + 2], "=") == 0) {
        printf("eh operador = %s\n", tokens[i + 2]);
        strcpy(valor_atual, tokens[i + 3]);
        while (tokens[i + 4] != NULL && !ehMarcador(tokens[i + 4])){
          printf("valoooor atual = %s\n", valor_atual);
           strcat(valor_atual, tokens[i+4]);  // Concatena o token atual
          i++;
        }
        // adiciono a variavel com valor na tabela de simbolos
        tabela_simbolos = adicionarSimbolo(
            tabela_simbolos, tipo_atual, nome_atual, valor_atual, escopo_atual);
        printf("adicionado na tabela de simbolos\n");
      }else{
        if (tokens[i + 2] != NULL && ehMarcador(tokens[i + 2]) ) {
          printf("token , = %s\n", tokens[i + 2]);
          // adiciono a variavel sem valor na tabela de simbolos
          tabela_simbolos = adicionarSimbolo(tabela_simbolos, tipo_atual,
                                             nome_atual, "", escopo_atual);
          printf("adicionado na tabela de simbolos sem valor\n");

        }

      }*/
      printf("token + 1  %s\n", tokens[i + 1]);
      printf("token + 2  %s\n", tokens[i + 2]);
      printf("token + 3  %s\n", tokens[i + 3]);

      // printf("eh tipo de dado - token %s\n", token);
    } else if (ehFuncao(token)) {
      strcpy(escopo_atual, token);
      // printf("eh funcao - token %s\n", token);
    } else if (ehVariavel(token)) { /*
       if (tokens[i + 1] != NULL && ehOperador(tokens[i + 1]) &&
         strcmp(tokens[i + 1], "=") == 0) {
         strcpy(nome_atual, tokens[i]);
         printf("nome atual = %s\n", nome_atual);
         strcpy(valor_atual, tokens[i + 2]);
         while (tokens[i + 3] != NULL && !ehMarcador(tokens[i + 3])){
           printf("valoooor atual = %s\n", valor_atual);
            strcat(valor_atual, tokens[i + 3]);  // Concatena o token atual
           i++;
         }
         tabela_simbolos = adicionarSimbolo(
             tabela_simbolos, tipo_atual, nome_atual, valor_atual,
       escopo_atual); printf("adicionado na tabela de simbolos\n");
       }*/
      // printf("eh variavel - token %s\n", token);
    } else if (ehNumero(token)) {
      // printf("eh numero - token %s\n", token);
    } else if (ehString(token)) {
      // printf("eh string - token %s\n", token);
    } else if (ehOperador(token)) {
      // printf("eh operador - token %s\n", token);
    } else if (ehMarcador(token)) {
      // printf("eh marcador - token %s\n", token);
    } else {
      printf("------------------------------------------------------\n");
      printf("\tERRO: Sintaxe invalida ('%s')- LINHA %d \n", token, linhaAtual);
      printf("------------------------------------------------------\n");
      return 0;
    }
  }
  return 1;
}

//******-------------------------------------------------**********************----------------------------------------------------
//           Funcoes que fazem o gerenciamento de memoria
//******-------------------------------------------------**********************----------------------------------------------------

void *verificaSeTemMemoriaDisponivelEFazOMalloc(size_t size) {
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

void LiberaMallocELiberaMemoria(void *ptr, size_t size) {
  if (ptr) {
    free(ptr);
    if (total_memory_used >= size) {
      total_memory_used -= size;
    }
  }
  if (ptr == NULL) {
    if (total_memory_used >= size) {
      total_memory_used -= size;
    }
  }
}

int main() {
  int numTokens;
  char **resultado = tokenizeFile("Codigo1.txt", &numTokens);

  if (VerificaSintaxeEhValida(resultado, numTokens)) {
    printf("-={********************************************************}=-\n");
    printf("\t\tCodigo Compilado com Sucesso\n");
    printf("-={********************************************************}=-\n");
    imprimirTabela(tabela_simbolos);
  }

  printf("\n[Informacoes]\nTokens encontrados: %d\n", numTokens);
  for (int i = 0; i < numTokens; i++) {
    // printf("Token[%d]: %s\n", i, resultado[i]);
    LiberaMallocELiberaMemoria(resultado[i], strlen(resultado[i]) + 1);
  }
  LiberaMallocELiberaMemoria(resultado, numTokens * sizeof(char *));

  printf("\n[MEMORIA]\n");
  printf("Maximo de memoria usada: %.2f KB\n", (float)max_memory_used / KB);

  return 0;
}
