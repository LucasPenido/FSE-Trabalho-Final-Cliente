#include "mqtt.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "cJSON.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "gpio.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "mqtt_client.h"
#include "sensor_dht11.h"
#include "wifi.h"

#define MATRICULA CONFIG_MATRICULA
#define TAG "MQTT"
#define CADASTRAR 0
#define CONFIRMA_CADASTRO 1
#define TROCA_ESTADO_SAIDA 2

extern xSemaphoreHandle conexaoMQTTSemaphore;
extern xSemaphoreHandle cadastroEspSemaphore;
extern char nomeComodo[30];
EventGroupHandle_t s_wifi_event_group;

// extern EventGroupHandle_t s_mqtt_event_group;
esp_mqtt_client_handle_t client;

char *obtemMacAddress() {
    uint8_t base_mac_addr[6] = {0};
    char *macAddress = malloc(20);
    esp_efuse_mac_get_default(base_mac_addr);

    snprintf(macAddress, 20,
             "%x:%x:%x:%x:%x:%x", base_mac_addr[0], base_mac_addr[1], base_mac_addr[2], base_mac_addr[3], base_mac_addr[4], base_mac_addr[5]);

    return macAddress;
}

void mqtt_informaEstado() {
    char topicoCadastro[60];
    char *macAddress = obtemMacAddress();
    cJSON *root;

    root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "mac", macAddress);
    cJSON_AddNumberToObject(root, "estado_saida", gpio_estadoSaida());
    cJSON_AddNumberToObject(root, "estado_entrada", !gpio_estadoEntrada());

    snprintf(topicoCadastro, 60, "fse2020/%s/%s/estado", MATRICULA, nomeComodo);

    char *json = cJSON_Print(root);

    mqtt_envia_mensagem(topicoCadastro, json);
}

void trataMensagem(int tamanhoMensagem, char *mensagem) {
    cJSON *mqttMensagem = cJSON_Parse(mensagem);
    int tipo = cJSON_GetObjectItem(mqttMensagem, "tipo")->valueint;

    if (tipo == CONFIRMA_CADASTRO) {
        strcpy(nomeComodo, cJSON_GetObjectItem(mqttMensagem, "comodo")->valuestring);
        xSemaphoreGive(cadastroEspSemaphore);
    } else if (tipo == TROCA_ESTADO_SAIDA) {
        gpio_trocaEstadoSaida();
        mqtt_informaEstado();
    }
}

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event) {
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            xSemaphoreGive(conexaoMQTTSemaphore);
            // xEventGroupSetBits(s_mqtt_event_group, MQTT_CONNECTED_BIT);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            // xEventGroupClearBits(s_mqtt_event_group, MQTT_CONNECTED_BIT);
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED");
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED");
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED");
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            // printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            // printf("DATA=%.*s\r\n", event->data_len, event->data);
            trataMensagem(event->data_len, event->data);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}

void mqtt_start() {
    // s_mqtt_event_group = xEventGroupCreate();

    esp_mqtt_client_config_t mqtt_config = {
        .uri = "mqtt://test.mosquitto.org",
    };
    client = esp_mqtt_client_init(&mqtt_config);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}

void mqtt_envia_mensagem(char *topico, char *mensagem) {
    int message_id = esp_mqtt_client_publish(client, topico, mensagem, 0, 1, 0);
    ESP_LOGI(TAG, "Mesnagem enviada, ID: %d", message_id);
}

void mqtt_inscreveCanalDispositivo() {
    char topicoCadastro[60];
    char *macAddress = obtemMacAddress();

    snprintf(topicoCadastro, 60, "fse2020/%s/dispositivos/%s", MATRICULA, macAddress);

    esp_mqtt_client_subscribe(client, topicoCadastro, 0);
    free(macAddress);
}

void mqtt_cadastra_esp_servidor() {
    char topicoCadastro[60];
    char *macAddress = obtemMacAddress();
    cJSON *root;

    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "tipo", CADASTRAR);
    cJSON_AddStringToObject(root, "mac", macAddress);

    char *json = cJSON_Print(root);

    snprintf(topicoCadastro, 60, "fse2020/%s/dispositivos/%s", MATRICULA, macAddress);

    mqtt_envia_mensagem(topicoCadastro, json);
    esp_mqtt_client_subscribe(client, topicoCadastro, 0);
    free(macAddress);
}

void mqtt_enviaTempUmidServidorCentral(void *params) {
    char topicoTemperatura[60], topicoUmidade[60];
    cJSON *rootTemperatura;
    cJSON *rootUmidade;
    char *macAddress = obtemMacAddress();

    rootTemperatura = cJSON_CreateObject();
    rootUmidade = cJSON_CreateObject();

    cJSON_AddStringToObject(rootTemperatura, "mac", macAddress);
    cJSON_AddStringToObject(rootUmidade, "mac", macAddress);

    free(macAddress);

    snprintf(topicoTemperatura, 65, "fse2020/%s/%s/temperatura", MATRICULA, nomeComodo);
    snprintf(topicoUmidade, 65, "fse2020/%s/%s/umidade", MATRICULA, nomeComodo);

    if (xSemaphoreTake(conexaoMQTTSemaphore, portMAX_DELAY)) {
        while (true) {
            if (xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY)) {
                struct dht11_reading result;
                result = dht11_leTemperaturaUmidade();

                if (result.status == 0) {
                    cJSON_AddNumberToObject(rootTemperatura, "temperatura", result.temperature);
                    cJSON_AddNumberToObject(rootUmidade, "umidade", result.humidity);

                    char *jsonTemperatura = cJSON_Print(rootTemperatura);
                    char *jsonUmidade = cJSON_Print(rootUmidade);

                    mqtt_envia_mensagem(topicoTemperatura, jsonTemperatura);
                    mqtt_envia_mensagem(topicoUmidade, jsonUmidade);

                    cJSON_DeleteItemFromObject(rootTemperatura, "temperatura");
                    cJSON_DeleteItemFromObject(rootUmidade, "umidade");
                } else {
                    ESP_LOGE("DHT11", "Erro ao obter dados");
                }
                vTaskDelay(30000 / portTICK_PERIOD_MS);
            }
        }
    }
}