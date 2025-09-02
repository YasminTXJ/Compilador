#include "../ArquivosH/mensagens.h"
#include <stdio.h>

void imprimeMensagemErro(const char *mensagem, const char *token, int linha) {
  printf("-={********************************************************}="
         "-\n");
  printf("%s ('%s') - LINHA %d\n", mensagem, token, linha);
  printf("-={********************************************************}="
         "-\n");
}

void imprimeMensagemErroSemToken(const char *mensagem, int linha) {
  printf("-={********************************************************}="
         "-\n");
  printf("%s  - LINHA %d\n", mensagem, linha);
  printf("-={********************************************************}="
         "-\n");
}

void alertaSemanticoComToken(const char *mensagem, const char *token,
                             int linha) {
  printf("-={********************************************************}="
         "-\n");
  printf("Alerta Semântico: %s (%s) - LINHA %d\n", mensagem, token, linha);
  printf("-={********************************************************}="
         "-\n");
}

void alertaSemantico(const char *mensagem, int linha) {
  printf("-={********************************************************}="
         "-\n");
  printf("Alerta Semântico: %s - LINHA %d\n", mensagem, linha);
  printf("-={********************************************************}="
         "-\n");
}
