#include "gpio.h"

#include <stdio.h>

xQueueHandle filaDeInterrupcao;

static void IRAM_ATTR gpio_isr_handler(void *args) {
    int pino = (int)args;
    xQueueSendFromISR(filaDeInterrupcao, &pino, NULL);
}

void trataInterrupcaoBotao(void *params) {
    int pino;

    while (true) {
        if (xQueueReceive(filaDeInterrupcao, &pino, portMAX_DELAY)) {
            // De-bouncing
            int estado = gpio_get_level(pino);
            mqtt_informaEstado();  // botão precionado
            if (estado == 0) {
                gpio_isr_handler_remove(pino);
                while (gpio_get_level(pino) == estado) {
                    vTaskDelay(50 / portTICK_PERIOD_MS);
                }
                mqtt_informaEstado();  // botão desprecionado

                // Habilitar novamente a interrupção
                vTaskDelay(50 / portTICK_PERIOD_MS);
                gpio_isr_handler_add(pino, gpio_isr_handler, (void *)pino);
            }
        }
    }
}

int gpio_estadoSaida() {
    return gpio_get_level(SAIDA);
}

int gpio_estadoEntrada() {
    return gpio_get_level(ENTRADA);
}

void gpio_configuraEntrada(void *params) {
    gpio_pad_select_gpio(ENTRADA);
    gpio_set_direction(ENTRADA, GPIO_MODE_INPUT);
    gpio_pulldown_en(ENTRADA);
    gpio_pullup_dis(ENTRADA);

    gpio_set_intr_type(ENTRADA, GPIO_INTR_ANYEDGE);

    filaDeInterrupcao = xQueueCreate(10, sizeof(int));
    xTaskCreate(trataInterrupcaoBotao, "TrataBotao", 2048, NULL, 1, NULL);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(ENTRADA, gpio_isr_handler, (void *)ENTRADA);
}

void gpio_trocaEstadoSaida() {
    int estado;
    gpio_reset_pin(SAIDA);
    gpio_set_direction(SAIDA, GPIO_MODE_INPUT_OUTPUT);  // Para que a função gpio_get_level pegasse o valor certo, foi preciso utilizar o modo GPIO_MODE_INPUT_OUTPUT
    estado = gpio_get_level(SAIDA);
    gpio_set_level(SAIDA, !estado);
}