#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TOKEN_SIZE 256
#define MAX_LINE 1024
#define KB 1024
#define DEFAULT_MEMORY_LIMIT (2048 * KB)

// Variáveis globais de memória
size_t total_memory_used = 0;
size_t max_memory_used = 0;
size_t memory_limit = DEFAULT_MEMORY_LIMIT;

// Alocação de memória com controle
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

void safe_free(void *ptr, size_t size) {
    if (ptr) {
        free(ptr);
        if (total_memory_used >= size) {
            total_memory_used -= size;
        }
    }
}

char *safe_strdup(const char *s) {
    size_t size = strlen(s) + 1;
    char *dup = safe_malloc(size);
    strcpy(dup, s);
    return dup;
}

// Tokens fixos
const char *keywords[] = {"principal", "funcao", "retorno", "leia",
                          "escreva",   "se",     "senao",   "para"};
const char *tipos[] = {"inteiro", "texto", "decimal"};
const char *operadores[] = {"==", "<>", "<=", ">=", "&&", "||", "+",
                            "-",  "*",  "/",  "^",  "<",  ">",  "="};
const char *marcadores[] = {"(", ")", "{", "}", "[", "]", ";", ",", " "};

// Verificações
int isKeyword(const char *token) {
    for (int i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
        if (strcmp(token, keywords[i]) == 0) return 1;
    }
    return 0;
}

int isTipo(const char *token) {
    for (int i = 0; i < sizeof(tipos) / sizeof(tipos[0]); i++) {
        if (strcmp(token, tipos[i]) == 0) return 1;
    }
    return 0;
}

int isOperador(const char *token) {
    for (int i = 0; i < sizeof(operadores) / sizeof(operadores[0]); i++) {
        if (strcmp(token, operadores[i]) == 0) return 1;
    }
    return 0;
}

int isMarcador(const char *token) {
    for (int i = 0; i < sizeof(marcadores) / sizeof(marcadores[0]); i++) {
        if (strcmp(token, marcadores[i]) == 0) return 1;
    }
    return 0;
}

int isVariavel(const char *token) {
    return token[0] == '!' && islower(token[1]);
}

int isFuncao(const char *token) {
    return strncmp(token, "__", 2) == 0 && (isalpha(token[2]) || isdigit(token[2]));
}

int isNumero(const char *token) {
    safe_malloc(sizeof(int));
    int ponto = 0;
    for (int i = 0; token[i]; i++) {
        if (token[i] == '.') ponto++;
        else if (!isdigit(token[i])) return 0;
    }
    return ponto <= 1;
}

int verificaAsciiValido(char c) {
    return (c >= 0 && c <= 127);
}

int nomeDaVariavelEhValida(const char *token) {
    for (int i = 2; token[i] != '\0'; i++) {
        if (!((token[i] >= 'a' && token[i] <= 'z') ||
              (token[i] >= 'A' && token[i] <= 'Z') ||
              (token[i] >= '0' && token[i] <= '9'))) {
            
            return 0;
        }
    }
    return 1;
}

int eh_letra_ou_numero(const char *token) {
    for (int i = 0; token[i]; i++) {
        if (!((token[i] >= 'A' && token[i] <= 'Z') ||
              (token[i] >= 'a' && token[i] <= 'z') ||
              (token[i] >= '0' && token[i] <= '9'))) {
            return 0;
        }
    }
    return 1;
}

// Vetor dinâmico de tokens
char **tokens = NULL;
int tokenCount = 0;

void addToken(const char *token) {
    if (strlen(token) == 0) return;
    safe_malloc(sizeof(char *)); //Realoca o vetor de tokens para comportar mais um ponteiro (mais um token).
    tokens = realloc(tokens, (tokenCount + 1) * sizeof(char *)); //realoca o vetor para acomodar o novo token
    tokens[tokenCount] = safe_strdup(token);  //Duplica a string `token` usando `safe_strdup` (que aloca memória e copia a string original).
    tokenCount++;
}

// Tokenização
void tokenizeLine(const char *line, int num_linha) {
    char token[MAX_TOKEN_SIZE];// Buffer  para montar tokens à medida que os caracteres são lidos.
    
    int i = 0, j = 0; // i percorre a linha do arquivo, j percorre o token
    int len = strlen(line);//tamanho da linha
    int dentroDeString = 0;
    safe_malloc(sizeof(i));
    safe_malloc(sizeof(len));
    safe_malloc(sizeof(dentroDeString));

    while (i < len) {
        char c = line[i];
        safe_malloc(sizeof(c));

        if (c == '"') dentroDeString = !dentroDeString;//verifica se está entre aspas
        // se estiver entre aspas não verifia se são validos na ASCII
        if (!dentroDeString && !verificaAsciiValido(c)) {
            printf("[LINHA %d] ERRO: Caractere inválido: %c \n", num_linha, c);
            i++;
             safe_free(i, sizeof(int));
            break;
        }

        switch (c) {
            case ' ':
            case '\t':
            case '\n':
                token[j] = '\0';
                addToken(token); // se for espaço, tab ou quebra de linha, adiciona o token
                j = 0;
                i++;
                break;

            case '"':
                j = 0;
                token[j++] = line[i++];//Adiciona o caractere de aspas ao token
                //Adiciona os caracteres até encontrar a próxima aspas
                while (i < len && line[i] != '"') token[j++] = line[i++]; 
                if (i < len && line[i] == '"') token[j++] = line[i++];
                token[j] = '\0';
                addToken(token);//Adiciona o token completo
                j = 0;
                break;

            case '!':
                token[j] = '\0';
                addToken(token); // Finaliza qualquer token anterior
                j = 0;
                token[j++] = c; //Inicia um novo token com o caractere '!'
                i++;
                //Adiciona os caracteres até encontrar um caractere que não seja letra ou número
                while (i < len && isalnum(line[i])) token[j++] = line[i++]; 
                token[j] = '\0';
                addToken(token); //adiciona o token completo
                j = 0;
                break;

            case '=': case '<': case '>': case '&': case '|':
                token[j] = '\0';
                addToken(token);
                j = 0;
                if ((c == '=' && line[i + 1] == '=') ||
                    (c == '<' && (line[i + 1] == '=' || line[i + 1] == '>')) ||
                    (c == '>' && line[i + 1] == '=') ||
                    (c == '&' && line[i + 1] == '&') ||
                    (c == '|' && line[i + 1] == '|')) {
                    char op_duplo[3] = {c, line[i + 1], '\0'};
                    addToken(op_duplo);// adiciona o operador duplo
                    i += 2;
                } else {
                    char op[2] = {c, '\0'};
                    addToken(op);// adiciona o operador simples
                    i++;
                }
                break;

            case '+': case '-': case '*': case '/': case '^':
            case '(': case ')': case '{': case '}':
            case '[': case ']': case ';': case ',':
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
   
    token[j] = '\0';// Finaliza o token atual
    addToken(token);
}

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

void classificaToken(char **tokens, int numTokens) {
    int linhaAtual = 1;
    for (int i = 0; i < numTokens; i++) {
        const char *token = tokens[i];
        if (!token || strlen(token) == 0) continue;

        if (strcmp(token, ";") == 0) linhaAtual++;

        if (isKeyword(token)) {
            // printf("LINHA %d: %s - Palavra reservada\n", linhaAtual, token);
        } else if (isTipo(token)) {
        } else if (isFuncao(token)) {
        } else if (isVariavel(token)) {
            if (!nomeDaVariavelEhValida(token)) {
                printf("LINHA %d: %s - Nome de variável inválido!\n", linhaAtual, token);
                break;
            }
        } else if (isNumero(token) || (token[0] == '"' && token[strlen(token) - 1] == '"')) {
        } else if (isOperador(token)) {
        } else if (isMarcador(token)) {
        } else if (eh_letra_ou_numero(token)) {
        } else {
            printf("LINHA %d: %s - ERRO: Token inválido\n", linhaAtual, token);
            break;
        }
    }
}

int main() {
  safe_malloc(sizeof(operadores));
  safe_malloc(sizeof(keywords));
  safe_malloc(sizeof(marcadores));
  safe_malloc(sizeof(tipos));
  
    int numTokens;
    safe_malloc(sizeof(numTokens));
    char **resultado = tokenizeFile("Codigo1.txt", &numTokens);
    

    classificaToken(resultado, numTokens);

    printf("Tokens encontrados: %d\n", numTokens);
    for (int i = 0; i < numTokens; i++) {
        printf("Token[%d]: %s\n", i, resultado[i]);
        safe_free(resultado[i], strlen(resultado[i]) + 1);
    }
    safe_free(resultado, tokenCount * sizeof(char *));

    printf("\n[MEMÓRIA]\n");
    printf("Máximo de memória usada: %.2f KB\n", (float)max_memory_used / KB);
    printf("Memória total usada: %.2f KB\n", (float)total_memory_used / KB);

    return 0;
}
