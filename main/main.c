#include <stdio.h>

#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "freertos/semphr.h"
#include "gpio.h"
#include "mqtt.h"
#include "nvs.h"
#include "sensor_dht11.h"
#include "wifi.h"

xSemaphoreHandle conexaoWifiSemaphore;
xSemaphoreHandle conexaoMQTTSemaphore;
xSemaphoreHandle cadastroEspSemaphore;
// EventGroupHandle_t s_wifi_event_group;
EventGroupHandle_t s_mqtt_event_group;

char nomeComodo[50];

void conectadoWifi(void* params) {
    while (true) {
        if (xSemaphoreTake(conexaoWifiSemaphore, portMAX_DELAY)) {
            mqtt_start();
        }
    }
}

int verificaCadastramentoServidorCentral() {
    int cadastradoServidor = nvs_leInformacaoCadastroArmazenadao();
    if (xSemaphoreTake(conexaoMQTTSemaphore, portMAX_DELAY)) {
        if (cadastradoServidor == 0) {
            mqtt_cadastra_esp_servidor();
            ESP_LOGI("CLIENTE", "AGUARDANDO CONFIRMAÇÃO CADASTRO");
            if (xSemaphoreTake(cadastroEspSemaphore, portMAX_DELAY)) {
                nvs_armazenaInformacaoCadastro(nomeComodo);
            }
        } else {
            char* temp = nvs_leInformacaoComodoArmazenado();
            strcpy(nomeComodo, temp);
            free(temp);
            mqtt_inscreveCanalDispositivo();
        }
        xSemaphoreGive(conexaoMQTTSemaphore);
    }

    return cadastradoServidor;
}

void app_main(void) {
    gpio_configuraEntrada();
    dht11_inicializa();

    // Inicializa o NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    conexaoWifiSemaphore = xSemaphoreCreateBinary();
    conexaoMQTTSemaphore = xSemaphoreCreateBinary();
    cadastroEspSemaphore = xSemaphoreCreateBinary();

    wifi_start();
    xTaskCreate(&conectadoWifi, "Conexão ao MQTT", 4096, NULL, 1, NULL);

    verificaCadastramentoServidorCentral();
    // xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
    // xEventGroupWaitBits(s_mqtt_event_group, MQTT_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

    xTaskCreate(&mqtt_enviaTempUmidServidorCentral, "Comunicação com Broker", 4096, NULL, 1, NULL);
}
