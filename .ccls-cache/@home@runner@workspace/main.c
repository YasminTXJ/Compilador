#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// 0 falso
#define MAX_TOKEN_SIZE 256
#define MAX_LINE 1024
#define KB 1024
#define DEFAULT_MEMORY_LIMIT (2048 * KB)

// Estrutura para lista encadeada de tokens
typedef struct TokenNode {
  char *token;
  int linha;
  struct TokenNode *prox;
} TokenNode;

// Estrutura da tabela de símbolos
typedef struct Simbolo {
  char tipo[20];
  char nome[50];
  char valor[100];
  char escopo[50];
  struct Simbolo *prox;
} Simbolo;
// pilha para verificar balanceamento de delimitadores
typedef struct Pilha {
  char simbolo; // delimitador
  int linha;    // número da linha
  struct Pilha *prox;
} Pilha;

// Tokens fixos
const char *palavrasReservadas[] = {"principal", "funcao", "retorno", "leia",
                                    "escreva",   "se",     "senao",   "para"};
const char *tiposDeDados[] = {"inteiro", "texto", "decimal"};
const char *operadores[] = {"==", "<>", "<=", ">=", "&&", "||", "+",
                            "-",  "*",  "/",  "^",  "<",  ">",  "="};
const char *marcadores[] = {"(", ")", "{", "}", "[", "]", ";", ",", " "};

// Variáveis globais
size_t total_memory_used = 0;
size_t max_memory_used = 0;
size_t memory_limit = DEFAULT_MEMORY_LIMIT;
char escopo_atual[50] = "global";
char funcao_anterior[50] = "global";
char tipo_atual[20];
char nome_atual[50];
char valor_atual[100];

TokenNode *token_list = NULL;
TokenNode *token_list_tail = NULL;
int tokenCount = 0;
int flag_escopo_palavra_reservada = 0;
Simbolo *tabela_simbolos = NULL;

// Empilhar
Pilha *push(Pilha *topo, char simbolo, int linha) {
  Pilha *novo = (Pilha *)malloc(sizeof(Pilha));
  novo->simbolo = simbolo;
  novo->linha = linha;
  novo->prox = topo;
  return novo;
}

// Desempilhar
Pilha *pop(Pilha *topo, char *simbolo, int *linha) {
  if (topo == NULL)
    return NULL;
  *simbolo = topo->simbolo;
  *linha = topo->linha;
  Pilha *tmp = topo;
  topo = topo->prox;
  free(tmp);
  return topo;
}

int isEmpty(Pilha *topo) { return topo == NULL; }
bool pricipalExiste = false;
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

TokenNode* verificaParDeExpressao(TokenNode *tk){
  TokenNode *expr = tk;
  if (!expr) {
      printf("ERRO SINTÁTICO: condição vazia no 'se' - LINHA %d\n", tk->linha);
      exit(1);
  }
  // operando 1
  if (!(ehVariavel(expr->token) || ehNumero(expr->token) || ehString(expr->token))) {
      printf("ERRO SINTÁTICO: condição inválida ('%s') - LINHA %d\n", expr->token, expr->linha);
      exit(1);
  }
  // operador
  TokenNode *op = expr->prox;
  if (!op || !ehOperador(op)) {
      printf("ERRO SINTÁTICO: esperado operador após '%s' - LINHA %d\n", expr->token, expr->linha);
      exit(1);
  }
  // checagem semântica de operadores (6.5 + 3.2.3 + 3.2.4)
  if (strcmp(op->token, "=<") == 0 || strcmp(op->token, "=>") == 0 ||
      strcmp(op->token, "><") == 0 || strcmp(op->token, "!=") == 0 ||
      strcmp(op->token, "<<") == 0 || strcmp(op->token, ">>") == 0) {
      printf("ERRO SEMÂNTICO: operador inválido '%s' - LINHA %d\n", op->token, op->linha);
      exit(1);
  }
  if (ehOperador(op) && ehOperador(op->prox)) {
      printf("ERRO SEMÂNTICO: operadores duplicados na condição do 'se' - LINHA %d\n", op->linha);
      exit(1);
  }
  // operando 2
  TokenNode *expr2 = op->prox;
  if (!expr2 || !(ehVariavel(expr2->token) || ehNumero(expr2->token) || ehString(expr2->token))) {
      printf("ERRO SINTÁTICO: esperado operando após operador '%s' - LINHA %d\n", op->token, op->linha);
      exit(1);
  }
  // restrição: texto só pode usar == ou <>
  if ((ehString(expr->token) || ehString(expr2->token)) &&
      (strcmp(op->token, "<") == 0 || strcmp(op->token, "<=") == 0 ||
       strcmp(op->token, ">") == 0 || strcmp(op->token, ">=") == 0)) {
      printf("ERRO SEMÂNTICO: operador '%s' inválido para texto - LINHA %d\n", op->token, op->linha);
      exit(1);
  }
  if((!expr2->prox || strcmp(expr2->prox->token, ")") != 0 ) && ( !strcmp(expr2->prox->token, "&&") && !strcmp(expr2->prox->token, "||") )){
      printf("ERRO SINTÁTICO: esperado ')' após expressão - LINHA %d\n", expr2->linha);
      exit(1);
  }else{
    if( strcmp(expr2->prox->token, "&&")==0 || strcmp(expr2->prox->token, "||") ==0){
      //printf("Achei um operador logico %s ",expr2->prox->token);
       TokenNode *res = verificaParDeExpressao(expr2->prox->prox);
      return res;
    }else{
      //retorna o token depois do )
        return expr2->prox->prox;
    }
  }
  printf("erro ao validar expressão do se Linha : %d\n", tk->linha);
  exit(1);
}
// 6) verifica se é bloco de várias linhas (abre com '{')
bool ehBlocoMultilinha(const TokenNode *tk) {
    if (!tk) return false;
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
        printf("ERRO SINTÁTICO: esperado bloco após condição do 'se' - LINHA %d\n", expr->linha);
        exit(1);
    }

    // se for várias linhas → precisa abrir com {
    if (ehBlocoMultilinha(blocoV)) {
        if (strcmp(blocoV->token, "{") != 0) {
            printf("ERRO SINTÁTICO: bloco múltiplo sem '{' no 'se' - LINHA %d\n", blocoV->linha);
            exit(1);
        }
        // percorre até achar }
        TokenNode *cur = blocoV->prox;
        int encontrouFecha = 0;
        while (cur && strcmp(cur->token, "}") != 0) {
            if (ehTipoDeDado(cur->token)) {
                printf("ERRO SEMÂNTICO: declaração de variável não permitida dentro do 'se' - LINHA %d\n", cur->linha);
                exit(1);
            }
            // pode chamar verificaSe recursivamente (se aninhado)
            if (strcmp(cur->token, "se") == 0) {
                verificaSe(cur);
            }
            cur = cur->prox;
        }
        if (!cur) {
            printf("ERRO SINTÁTICO: bloco do 'se' não fechado com '}' - LINHA %d\n", blocoV->linha);
            exit(1);
        }
        encontrouFecha = 1;
    } else {
        if (ehTipoDeDado(blocoV->token)) {
            printf("ERRO SEMÂNTICO: declaração de variável não permitida dentro do 'se' - LINHA %d\n", blocoV->linha);
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
            printf("ERRO SINTÁTICO: esperado bloco após 'senao' - LINHA %d\n", senao->linha);
            exit(1);
        }
        if (ehBlocoMultilinha(blocoF)) {
            if (strcmp(blocoF->token, "{") != 0) {
                printf("ERRO SINTÁTICO: bloco múltiplo sem '{' no 'senao' - LINHA %d\n", blocoF->linha);
                exit(1);
            }
            TokenNode *cur = blocoF->prox;
            while (cur && strcmp(cur->token, "}") != 0) {
                if (ehTipoDeDado(cur->token)) {
                    printf("ERRO SEMÂNTICO: declaração de variável não permitida dentro do 'senao' - LINHA %d\n", cur->linha);
                    exit(1);
                }
                if (strcmp(cur->token, "se") == 0) {
                    verificaSe(cur);
                }
                cur = cur->prox;
            }
            if (!cur) {
                printf("ERRO SINTÁTICO: bloco do 'senao' não fechado com '}' - LINHA %d\n", blocoF->linha);
                exit(1);
            }
        } else {
            if (!blocoF->prox || strcmp(blocoF->prox->token, ";") != 0) {
                printf("ERRO SINTÁTICO: comando único do 'senao' deve terminar com ';' - LINHA %d\n", blocoF->linha);
                exit(1);
            }
            if (ehTipoDeDado(blocoF->token)) {
                printf("ERRO SEMÂNTICO: declaração de variável não permitida dentro do 'senao' - LINHA %d\n", blocoF->linha);
                exit(1);
            }
        }
    }

    return 1; // passou por tudo
}

int ehMarcador(const char *token) {
  for (int i = 0; i < sizeof(marcadores) / sizeof(marcadores[0]); i++)
    if (strcmp(token, marcadores[i]) == 0)
      return 1;
  return 0;
}

// Função que retorna o nome base da variável (antes do '[')
char* getNomeBase(const char *token) {
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

Simbolo *buscarSimbolo(Simbolo *tabela, const char *nome, const char *funcao) {
  Simbolo *atual = tabela;
  char nomeBase [50] ;
  while (atual != NULL) {
    //pega só o nome da várivel sem o tamanho do array para decimal
    if(strcmp(atual->tipo, "decimal") == 0 ){
      strcpy(nomeBase, getNomeBase(atual->nome));
    }
    //pego o nome normal
    if(strcmp(atual->tipo, "decimal") != 0 ){
      strcpy(nomeBase, getNomeBase(atual->nome));
    }
    if (strcmp(nomeBase, nome) == 0 && strcmp(atual->escopo, funcao) == 0) {
      return atual; // Encontrou
    }
    atual = atual->prox;
  }
  return NULL; // Não existe
}
// Funções da tabela de símbolos
Simbolo *adicionarSimbolo(Simbolo *inicio, const char *tipo, const char *nome,
                          const char *valor, const char *funcao) {
  // Verifica se já existe
  // printf("Adicionando '%s' '%s' '%s' '%s'\n", tipo, nome, valor, funcao);
  Simbolo *existe = buscarSimbolo(inicio, nome, funcao);

  if (existe != NULL) {
    if (strlen(tipo) > 0) {
      // Tentando redeclarar -> ERRO
      printf(
          "-={********************************************************}=-\n");
      printf("ERRO: Variável '%s' já declarada no escopo '%s'.\n", nome,
             funcao);
      printf(
          "-={********************************************************}=-\n");
    }
    return existe;
  }

  Simbolo *novo = (Simbolo *)malloc(sizeof(Simbolo));
  if (!novo) {
    printf("-={********************************************************}=-\n");
    printf("Memória insuficiente!\n");
    printf("-={********************************************************}=-\n");
    exit(1);
  }
  strcpy(novo->tipo, tipo);
  strcpy(novo->nome, nome);
  strcpy(novo->valor, valor);
  strcpy(novo->escopo, funcao);
  novo->prox = NULL;

  if (!inicio)
    return novo;

  Simbolo *temp = inicio;
  while (temp->prox)
    temp = temp->prox;
  temp->prox = novo;
  return inicio;
}
// Atribuir valor a variavel
Simbolo *atribuirValor(Simbolo *inicio, const char *nome, const char *valor,
                       const char *funcao) {
  // Verifica se já existe
  Simbolo *existe = buscarSimbolo(inicio, nome, funcao);

  if (existe != NULL) {
    // Verificar se o tipo do valor é compativel com o tipo da variavel

    // Apenas atribuindo valor -> atualiza
    strcpy(existe->valor, valor);
    // printf("Variável '%s' atualizada para valor '%s'.\n", nome, valor);

    return existe;
  } else {
    printf("-={********************************************************}=-\n");
    printf("Variável '%s' - Impossivél atribuir valor '%s' a uma variavél  não "
           "declarada.\n",
           nome, valor);
    printf("-={********************************************************}=-\n");
    exit(1);
  }
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
int processarDeclaracao(TokenNode *token, char *tipo_atual, char *escopo_atual) {
      if (!token || !token->prox)
        return 0;

      // Pega o nome da variável (token seguinte ao tipo)
      if (ehVariavel(token->prox->token)) {
        strcpy(nome_atual, token->prox->token);
        // printf("nome atual = %s\n", nome_atual);
      } else {
        return 0;
      }

      TokenNode *atual = token->prox->prox;

      // Verifica se é um array (ex: car[2.5])
      if (atual && ehMarcador(atual->token) && strcmp(atual->token, "[") == 0) {
        strcat(nome_atual, "[");
        atual = atual->prox;

        // pega o que está entre os colchetes
        if (ehNumero(atual->token)) {
          strcat(nome_atual, atual->token);
          atual = atual->prox;
          if (atual && ehMarcador(atual->token) && strcmp(atual->token, "]") == 0) {
            strcat(nome_atual, "]");
          }
          atual = atual->prox;
        } else {
          printf(
              "-={********************************************************}=-\n");
          printf("ERRO: Array '%s' não é um número - LINHA %d\n", atual->token,
                 atual->linha);
          exit(1);
        }

        /*
        while (atual && strcmp(atual->token, "]") != 0) {
          strcat(nome_atual, atual->token);
          atual = atual->prox;
        }

        if (atual && strcmp(atual->token, "]") == 0 && atual) {
          strcat(nome_atual, "]");
          atual = atual->prox; // Avança além do colchete
        }*/
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

        // rintf("valoooor atual = %s\n", valor_atual);
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
          //printf("token virgula = %s\n", atual->prox->token);
          if(ehTipoDeDado(atual->prox->token)!=0){//se o proximo token for um tipo de dado, é um erro
            printf("-={********************************************************}="
       "-\n");
            printf("ERRO Sintático: Não se pode declarar dois tipos de dados na mesma linha - '%s' não pode ser declarada - LINHA %d\n", atual->prox->token,
                 atual->linha);
            printf("-={********************************************************}="
               "-\n");
            exit(1);
          }
          // printf("tem virgula\n");
          processarDeclaracao(atual, tipo_atual, escopo_atual);
        }
        return 1;
      }

      return 0;
    }

void verificaLeia(TokenNode *token){
  TokenNode *prox = token->prox->prox;
  if(prox->token == NULL){
    printf("-={********************************************************}="
       "-\n");
    printf("ERRO Sintático: Comando 'leia' sem variável - LINHA %d\n", prox->linha);
    printf("-={********************************************************}="
       "-\n");
    exit(1);
  }
  if(prox->token != NULL && !ehVariavel(prox->token)){
    printf("-={********************************************************}="
       "-\n");
    printf("ERRO Semântico: Comando 'leia' só pode ler variavéis - LINHA %d\n", prox->linha);
    printf("-={********************************************************}="
       "-\n");
    exit(1);
  }
  //pode ler mais de uma variavel, separadas por virgula e declaradas anteriormente
 if(prox->token != NULL && ehVariavel(prox->token) && ehVariavel(prox->prox->token)){
   printf("-={********************************************************}="
      "-\n");
   printf("ERRO Sintático: As variavéis do comando 'leia' não estão separadas por ','- LINHA %d\n", prox->linha);
   printf("-={********************************************************}="
      "-\n");
   exit(1);
 }
  while(prox->token != NULL && ehVariavel(prox->token) && strcmp(prox->prox->token,",") == 0){
      prox = prox->prox->prox;
  } 

  if(prox->token != NULL && ehVariavel(prox->token) && strcmp(prox->prox->token,")") ==0 && strcmp(prox->prox->prox->token,";")==0) {
    return;
  }else{
    printf("-={********************************************************}="
       "-\n");
    printf("ERRO Sintático: Comando 'leia' escrito incorretamente- LINHA %d\n", prox->linha);
    printf("-={********************************************************}="
       "-\n");
    exit(1);
  }
}

void verificaEscreva(TokenNode *token){
  TokenNode *prox = token->prox->prox;

  //escreva(" A é maior", !a);
  if(prox->token == NULL){
    printf("-={********************************************************}="
       "-\n");
    printf("ERRO Sintático: Comando 'excreva' sem variável ou texto - LINHA %d\n", prox->linha);
    printf("-={********************************************************}="
       "-\n");
    exit(1);
  }
  if(prox->token != NULL && !ehVariavel(prox->token) && !ehString(prox->token)){
    printf("-={********************************************************}="
       "-\n");
    printf("ERRO Semântico: Comando 'escreva' só pode escrever variavéis ou texto - LINHA %d\n", prox->linha);
    printf("-={********************************************************}="
       "-\n");
    exit(1);
  }
  //pode escrever mais de uma variavel ou texto, separadas por virgula e declaradas anteriormente
 // escreva("Escreva um número", !a , !x ,);
  while(prox->token != NULL && (ehVariavel(prox->token)==0 || ehString(prox->token)==0) && strcmp(prox->prox->token,",") == 0) {

    prox = prox->prox->prox;  //pula a virgula

  }
  //se tiver uma virgula sobrando, ele vai sair do while no )
  if(strcmp(prox->token,")") ==0){
    printf("-={********************************************************}="
       "-\n");
    printf("ERRO Sintático: Comando 'escreva' escrito incorretamente- LINHA %d\n", prox->linha);
    printf("-={********************************************************}="
       "-\n");
    exit(1);
  }
  //se tiver certo ele sai do while com uma variavel ou texto 
  if(prox->token != NULL && (ehVariavel(prox->token)==0 || ehString(prox->token)==0) && strcmp(prox->prox->token,")") ==0 ){
    if(strcmp(prox->prox->prox->token,";")==0){
      return;
    }else{
      printf("-={********************************************************}="
       "-\n");
      printf("ERRO Sintático: Comando 'escreva' escrito incorretamente- LINHA %d\n", prox->linha);
      printf("-={********************************************************}="
       "-\n");
       exit(1);
    }
  }
  if(prox->token != NULL && (ehVariavel(prox->token)==0 || ehString(prox->token)==0) && ehTipoDeDado(prox->token)!= 0 ){
    printf("-={********************************************************}="
       "-\n");
    printf("ERRO Sintático: Comando 'escreva' escrito incorretamente ,não podem conter declarações - LINHA %d\n", prox->linha);
    printf("-={********************************************************}="
       "-\n");
    exit(1);
  }
}
// Verificação de sintaxe usando lista encadeada
int VerificaSintaxeEhValida(TokenNode *head) {

  TokenNode *current = head;
  while (current) {
    const char *token = current->token;
    //printf("Token: %s - Linha: %d\n", token, current->linha);
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
        if (pricipalExiste) {
          printf("-={********************************************************}="
                 "-\n");
          printf("ERRO: Função 'principal' já declarada - LINHA %d\n",
                 current->linha);
          printf("-={********************************************************}="
                 "-\n");
          return 0;
        } else {
          pricipalExiste = true;
          strcpy(escopo_atual, "principal");
        }
      } else if (strcmp(token, "se") == 0 || strcmp(token, "senao") == 0 ||
                 strcmp(token, "para") == 0) {
                int num_linha = current->linha;
                TokenNode *currentAux = current->prox;

              // percorrer todos os tokens da linha
              while (currentAux && currentAux->linha == num_linha && currentAux != NULL) {
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

      }else  if (strcmp(token, "leia") == 0){
      verificaLeia(current);
      }else  if (strcmp(token, "escreva") == 0){
              verificaEscreva(current);
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

        atribuirValor(tabela_simbolos, nome_atual, valor_atual, escopo_atual);
      }
      //verifica se a variavel existe na tabela de simbolos
      if (buscarSimbolo(tabela_simbolos, token, escopo_atual) == NULL){
         printf("-={********************************************************}="
                 "-\n");
          printf("ERRO Semântico: Variável '%s' não declarada - LINHA %d\n", token,
                 current->linha);
        printf("-={********************************************************}="
                 "-\n");
        exit(1);
      }

    } else if (ehMarcador(token) && strcmp(token, "}") == 0) {
      if (flag_escopo_palavra_reservada < 0) {
        strcpy(escopo_atual, "global");
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

// Main
int main() {
  int numTokens;
  TokenNode *resultado = tokenizeFile("Codigo1.txt", &numTokens);

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

  printf("\n[Informacoes]\nTokens encontrados: %d\n", numTokens);

  freeTokenList(resultado);

  printf("\n[MEMORIA]\nMaximo de memoria usada: %.2f KB\n",
         (float)max_memory_used / KB);

  return 0;
}
