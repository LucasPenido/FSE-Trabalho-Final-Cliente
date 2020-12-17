#include "sensor_dht11.h"

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

void dht11_inicializa() {
    DHT11_init(GPIO_NUM_4);
}

struct dht11_reading dht11_leTemperaturaUmidade() {
    struct dht11_reading resultado;
    resultado = DHT11_read();
    return resultado;
}