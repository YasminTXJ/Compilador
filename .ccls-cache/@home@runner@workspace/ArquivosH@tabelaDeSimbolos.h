#include "tabela_simbolos.h"
#include "globals.h"  // acesso à tabela_simbolos global

// Função auxiliar que retorna o nome base da variável (ex.: remove tamanho de array)
extern char *getNomeBase(const char *nome); // declare se estiver em outro módulo

Simbolo *buscarSimbolo(Simbolo *tabela, const char *nome, const char *funcao) {
    Simbolo *atual = tabela_simbolos;
    char nomeBase[50];
    while (atual != NULL) {
        // pega só o nome da variável sem o tamanho do array
        if (strcmp(atual->tipo, "decimal") == 0 || strcmp(atual->tipo, "texto") == 0) {
            strcpy(nomeBase, getNomeBase(atual->nome));
        }
        if (strcmp(atual->tipo, "decimal") != 0 && strcmp(atual->tipo, "texto") != 0) {
            strcpy(nomeBase, getNomeBase(atual->nome));
        }
        if (strcmp(nomeBase, getNomeBase(nome)) == 0 && strcmp(atual->escopo, funcao) == 0) {
            return atual; // Encontrou
        }
        atual = atual->prox;
    }
    return NULL; // Não existe
}

Simbolo *adicionarSimbolo(Simbolo *inicio, const char *tipo, const char *nome,
                          const char *valor, const char *funcao) {
    Simbolo *existe = buscarSimbolo(inicio, nome, funcao);
    if (existe != NULL) {
        if (strlen(tipo) > 0) {
            printf("-={********************************************************}=-\n");
            printf("ERRO SEMÂNTICO:: Variável '%s' já declarada no escopo '%s'.\n",
                   nome, funcao);
            printf("-={********************************************************}=-\n");
            exit(1);
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

void imprimirTabela(Simbolo *inicio) {
    printf("\n%-10s %-15s %-15s %-20s\n", "Tipo", "Nome", "Valor", "Função");
    printf("-------------------------------------------------------------\n");
    while (inicio) {
        printf("%-10s %-15s %-15s %-20s\n", inicio->tipo, inicio->nome,
               inicio->valor, inicio->escopo);
        inicio = inicio->prox;
    }
}

Simbolo *atribuirValor(Simbolo *inicio, const char *nome, const char *valor,
                       const char *funcao) {
    Simbolo *existe = buscarSimbolo(inicio, nome, funcao);

    if (existe != NULL) {
        strcpy(existe->valor, valor);
        return existe;
    } else {
        printf("-={********************************************************}=-\n");
        printf("Variável '%s' - Impossível atribuir valor '%s' a uma variável não declarada.\n",
               nome, valor);
        printf("-={********************************************************}=-\n");
        exit(1);
    }
}
