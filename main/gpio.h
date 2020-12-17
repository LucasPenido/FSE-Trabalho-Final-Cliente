#ifndef GPIO_H
#define GPIO_H

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "mqtt.h"
#include "sdkconfig.h"

#define ENTRADA 0
#define SAIDA 2

int gpio_estadoSaida();
int gpio_estadoEntrada();
void gpio_configuraEntrada();
void gpio_trocaEstadoSaida();

#endif
