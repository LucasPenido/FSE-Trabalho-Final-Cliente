#include "nvs.h"

#include <stdio.h>

#include "esp_log.h"
#include "nvs_flash.h"

void nvs_armazenaInformacaoCadastro(char *nomeComodo) {
    ESP_ERROR_CHECK(nvs_flash_init());

    nvs_handle particao_padrao_handle;

    esp_err_t res_nvs = nvs_open("cadastroServ", NVS_READWRITE, &particao_padrao_handle);

    if (res_nvs == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE("NVS", "Namespace: cadastroServ, não encontrado");
    }
    esp_err_t res = nvs_set_i32(particao_padrao_handle, "cadastrado", 1);
    if (res != ESP_OK) {
        ESP_LOGE("NVS", "Não foi possível escrever no NVS (%s)", esp_err_to_name(res));
    }

    res = nvs_set_str(particao_padrao_handle, "comodo", nomeComodo);
    if (res != ESP_OK) {
        ESP_LOGE("NVS", "Não foi possível escrever no NVS (%s)", esp_err_to_name(res));
    }
    nvs_commit(particao_padrao_handle);
    nvs_close(particao_padrao_handle);
}

char *nvs_leInformacaoComodoArmazenado() {
    size_t tamanho;
    char *nomeComodo = malloc(30);
    nvs_handle particao_padrao_handle;

    // Inicia o acesso à partição padrão nvs
    ESP_ERROR_CHECK(nvs_flash_init());

    // Abre o acesso à partição nvs
    esp_err_t res_nvs = nvs_open("cadastroServ", NVS_READONLY, &particao_padrao_handle);

    if (res_nvs == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE("NVS", "Namespace: cadastroServ, não encontrado");
    } else {
        // Requisita a string com NULL para saber o tamanho da string armazenada;
        nvs_get_str(particao_padrao_handle, "comodo", NULL, &tamanho);
        esp_err_t res = nvs_get_str(particao_padrao_handle, "comodo", nomeComodo, &tamanho);

        switch (res) {
            case ESP_OK:
                ESP_LOGI("NVS", "Valor encontrado");
                break;
            case ESP_ERR_NOT_FOUND:
                ESP_LOGI("NVS", "Valor não encontrado");
                return NULL;
            default:
                ESP_LOGE("NVS", "Erro ao acessar o NVS (%s)", esp_err_to_name(res));
                return NULL;
                break;
        }

        nvs_close(particao_padrao_handle);
    }
    return nomeComodo;
}

int32_t nvs_leInformacaoCadastroArmazenadao() {
    // Inicia o acesso à partição padrão nvs
    ESP_ERROR_CHECK(nvs_flash_init());

    int32_t cadastradoServidor = 0;
    nvs_handle particao_padrao_handle;

    // Abre o acesso à partição nvs
    esp_err_t res_nvs = nvs_open("cadastroServ", NVS_READONLY, &particao_padrao_handle);

    if (res_nvs == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE("NVS", "Namespace: cadastroServ, não encontrado");
    } else {
        esp_err_t res = nvs_get_i32(particao_padrao_handle, "cadastrado", &cadastradoServidor);

        switch (res) {
            case ESP_OK:
                ESP_LOGI("NVS", "ESP ja cadastrada");
                break;
            case ESP_ERR_NOT_FOUND:
                ESP_LOGI("NVS", "ESP nao cadastrada");
                return 0;
            default:
                ESP_LOGE("NVS", "Erro ao acessar o NVS (%s)", esp_err_to_name(res));
                return -1;
                break;
        }

        nvs_close(particao_padrao_handle);
    }
    return cadastradoServidor;
}