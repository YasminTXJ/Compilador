    #include <ctype.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>

    #define MAX_LINE 1024
    #define KB 1024
    #define DEFAULT_MEMORY_LIMIT (2048 * KB)

    size_t total_memory_used = 0;
    size_t max_memory_used = 0;
    size_t memory_limit = DEFAULT_MEMORY_LIMIT;

    void* safe_malloc(size_t size) {
      if (total_memory_used + size > memory_limit) {
          fprintf(stderr, "ERRO: Memória Insuficiente\n");
          exit(EXIT_FAILURE);
      }

      void* ptr = malloc(size);
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

    void* safe_realloc(void* ptr, size_t old_size, size_t new_size) {
      if (new_size > old_size &&
          total_memory_used + (new_size - old_size) > memory_limit) {
          fprintf(stderr, "ERRO: Memória Insuficiente\n");
          exit(EXIT_FAILURE);
      }

      void* new_ptr = realloc(ptr, new_size);
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

    void safe_free(void* ptr, size_t size) {
      if (ptr) {
          free(ptr);
          if (total_memory_used >= size) {
              total_memory_used -= size;
          }
      }
    }



    const char *keywords[] = {"principal", "funcao", "retorno", "leia",
                              "escreva",   "se",     "senao",   "para"};
    const char *tipos[] = {"inteiro", "texto", "decimal"};
    const char *operadores[] = {"==", "<>", "<=", ">=", "&&", "||", "+",
                                "-",  "*",  "/",  "^",  "<",  ">",  "="};

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

    int verificaAsciiValido(char c) { return (c >= 0 && c <= 127); }

    void classificaToken(const char *token, int linha) {
      if (strlen(token) == 0)
        return;
      //printf("----------------Token analisado: %s\n", token);
      if (isKeyword(token)) {
        printf("[LINHA %d] Palavra-chave: %s\n", linha, token);
      } else if (isTipo(token)) {
        printf("[LINHA %d] Tipo de dado: %s\n", linha, token);
      } else if (isFuncao(token)) {
        printf("[LINHA %d] Função: %s\n", linha, token);
      } else if (isVariavel(token)) {
        printf("[LINHA %d] Variável: %s\n", linha, token);
      } else if (isNumero(token)) {
        printf("[LINHA %d] Número: %s\n", linha, token);
      } else if (isOperador(token)) {
        printf("[LINHA %d] Operador: %s\n", linha, token);
      } else if (strlen(token) == 1 && strchr("+-*/^=<>{}[]();,", token[0])) {
        printf("[LINHA %d] Delimitador/Operador: %s\n", linha, token);
      } else if (token[0] == '"' && token[strlen(token) - 1] == '"') {
        printf("[LINHA %d] String: %s\n", linha, token);
      } else {
        printf("[LINHA %d] Token desconhecido: %s\n", linha, token);
      }
    }

    void tokenizeLine(const char *line, int num_linha) {
      printf("------------ ANALISANDO [LINHA %d] ------------\n", num_linha);
      char token[256];
      int i = 0, j = 0;
      int len = strlen(line);

      while (i < len) {
        char c = line[i];


        if (!verificaAsciiValido(c)) {
            printf("[LINHA %d] ERRO: Caractere inválido na tabela ASCII: %c (%d)\n",
            num_linha, c, c); i++; 
          continue;
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
          char* linha = (char*)safe_malloc(MAX_LINE);
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

          fclose(fp);
          return 0;
      }