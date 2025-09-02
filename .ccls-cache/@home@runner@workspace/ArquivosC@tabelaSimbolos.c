#include "../ArquivosH/tabelaSimbolos.h"
#include "../ArquivosH/globals.h" // acesso à tabela_simbolos global
#include "../ArquivosH/verificaSintaxe.h"

Simbolo *tabela_simbolos = NULL;
extern char *
getNomeBase(const char *nome); // declare se estiver em outro módulo

Simbolo *buscarSimbolo(Simbolo *tabela, const char *nome, const char *funcao) {
  Simbolo *atual = tabela_simbolos;
  char nomeBase[50];
  while (atual != NULL) {
    // pega só o nome da variável sem o tamanho do array
    if (strcmp(atual->tipo, "decimal") == 0 ||
        strcmp(atual->tipo, "texto") == 0) {
      strcpy(nomeBase, getNomeBase(atual->nome));
    }
    if (strcmp(atual->tipo, "decimal") != 0 &&
        strcmp(atual->tipo, "texto") != 0) {
      strcpy(nomeBase, getNomeBase(atual->nome));
    }
    if (strcmp(nomeBase, getNomeBase(nome)) == 0 &&
        strcmp(atual->escopo, funcao) == 0) {
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
      printf(
          "-={********************************************************}=-\n");
      printf("ERRO SEMÂNTICO:: Variável/funcao '%s' já declarada no escopo "
             "'%s'.\n",
             nome, funcao);
      printf(
          "-={********************************************************}=-\n");
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
  printf("\n%-10s %-15s %-15s %-30s\n", "Tipo", "Nome", "Valor", "Função");
  printf("-------------------------------------------------------------\n");
  while (inicio) {
    printf("%-10s %-15s %-15s %-20s\n", inicio->tipo, inicio->nome,
           inicio->valor, inicio->escopo);
    inicio = inicio->prox;
  }
}
void trimFim(char *str) {
    int i = strlen(str) - 1;
    while (i >= 0 && isspace((unsigned char)str[i])) {
        str[i] = '\0';
        i--;
    }
}


bool verificaTipoDadoAtribuidoAVariavel(char *tipo_atual,
  char *valor){
  //printf("TIPO ATUAL: %s  - VALOR ATUAL: %s\n", tipo_atual, valor);
  //printf("Eh inteiro: %d  - Eh decimal: %d  - Eh string: %d\n", ehInteiro(valor), ehDecimal(valor), ehString(valor));
  trimFim(valor);
  //printf("eh variavel: %d\n", ehVariavel(valor));
  if (ehVariavel(valor)==1){
    //printf("########################\nValor é variavel\n");
    char tipo_var[20] = "";
    Simbolo *sim = buscarSimbolo(tabela_simbolos, valor, escopo_atual);
    if (sim != NULL) {
        strcpy(tipo_var, sim->tipo);
       // printf("copiei o tipo da variavel %s\n", tipo_var);

        if (strcmp(tipo_atual, tipo_var) == 0) {
            return true;
        }
    } else {
        printf("Erro semântico: variável |%s| não encontrada no escopo %s\n", valor, escopo_atual);
    }
    if(strcmp(tipo_atual, tipo_var)==0){
      return true;
    }
  }



  //se o valor for um numero, verifica se é inteiro ou decimal

    //verifica se o tipo da variavel é compativel com o valor atribuido
    if (strcmp(tipo_atual, "inteiro") == 0 && ehInteiro(valor) == 1){
        return true; 
    }
    if (strcmp(tipo_atual, "decimal") == 0 && ehDecimal(valor) == 1){
        return true;
    }
    if (strcmp(tipo_atual, "texto") == 0 && ehString(valor) == 1){
        return true;
    }
    return false;
}
Simbolo *atribuirValor(Simbolo *inicio, const char *nome, char *valor,
                       const char *funcao,int linha) {
  Simbolo *existe = buscarSimbolo(inicio, nome, funcao);

  if (existe != NULL) {
    char valorrr[100] = "";
    strcpy(valorrr, valor);
    // Verificar se o tipo do valor é compativel com o tipo da variavel
    if(verificaTipoDadoAtribuidoAVariavel(existe->tipo,valorrr) == false){
        printf("-={********************************************************}="
               "-\n");
          printf("Alerta Semântico atribuindo: Tipo de dado '%s' não compativel com o valor '%s' - LINHA %d\n", existe->tipo, valorrr, linha);
        printf("-={********************************************************}="
               "-\n");
       return NULL;
    }else{
      // Apenas atribuindo valor -> atualiza
      strcpy(existe->valor, valor);
      return existe;
    }

    // printf("Variável '%s' atualizada para valor '%s'.\n", nome, valor);

    return existe;
  }else {
    printf("-={********************************************************}=-\n");
    printf("Variável '%s' - Impossível atribuir valor '%s' a uma variável não "
           "declarada.\n",
           nome, valor);
    printf("-={********************************************************}=-\n");
    exit(1);
  }
}
