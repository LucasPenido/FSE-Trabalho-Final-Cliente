#ifndef NVS_H
#define NVS_H
#include <stdint.h>

void nvs_armazenaInformacaoCadastro(char *nomeComodo);
char *nvs_leInformacaoComodoArmazenado();
int32_t nvs_leInformacaoCadastroArmazenadao();

#endif