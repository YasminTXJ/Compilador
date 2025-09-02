#include "../ArquivosH/globals.h"



// Definição e inicialização
size_t total_memory_used = 0;
size_t max_memory_used = 0;
size_t memory_limit = DEFAULT_MEMORY_LIMIT;

char escopo_atual[50] = "global";
char funcao_anterior[50] = "global";
char tipo_atual[20] = "";
char nome_atual[50] = "";
char valor_atual[100] = "";

TokenNode *token_list = NULL;
TokenNode *token_list_tail = NULL;
int tokenCount = 0;
int flag_escopo_palavra_reservada = 0;


bool principalExiste = false; // definição da variável global
