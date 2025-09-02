#ifndef Mensagens_H
#define Mensagens_H

void imprimeMensagemErro(const char *mensagem, const char *token, int linha);

void imprimeMensagemErroSemToken(const char *mensagem, int linha);

void alertaSemanticoComToken(const char *mensagem, const char *token,
                             int linha);

void alertaSemantico(const char *mensagem, int linha);

#endif