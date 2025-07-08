#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINHAS 100
#define MAX_TOKENS_POR_LINHA 50
#define MAX_TAMANHO_TOKEN 64
#define MAX_TAMANHO_LINHA 256

int main() {
    FILE *arquivo = fopen("Codigo1.txt", "r");
    if (!arquivo) {
        perror("Erro ao abrir o arquivo");
        return 1;
    }

    // Vetor para armazenar os tokens de cada linha
    char tokens[MAX_LINHAS][MAX_TOKENS_POR_LINHA][MAX_TAMANHO_TOKEN];
    int total_linhas = 0;

    char linha[MAX_TAMANHO_LINHA];

    while (fgets(linha, sizeof(linha), arquivo)) {
        if (total_linhas >= MAX_LINHAS) {
            printf("Limite de linhas excedido\n");
            break;
        }

        int token_index = 0;
        char *token = strtok(linha, " \t\n");

        while (token != NULL && token_index < MAX_TOKENS_POR_LINHA) {
            strncpy(tokens[total_linhas][token_index], token, MAX_TAMANHO_TOKEN - 1);
            tokens[total_linhas][token_index][MAX_TAMANHO_TOKEN - 1] = '\0';
            token_index++;
            token = strtok(NULL, " \t\n");
        }

        total_linhas++;
    }

    fclose(arquivo);

    // Exibe os tokens por linha
    printf("\n=== TOKENS POR LINHA ===\n");
    for (int i = 0; i < total_linhas; i++) {
        printf("Linha %d:\n", i + 1);
        for (int j = 0; j < MAX_TOKENS_POR_LINHA && tokens[i][j][0] != '\0'; j++) {
            printf("  Token: %s\n", tokens[i][j]);
        }
    }

    return 0;
}
