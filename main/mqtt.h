#ifndef MQTT_H
#define MQTT_H

#define MQTT_CONNECTED_BIT BIT1

void mqtt_start();

void mqtt_envia_mensagem(char* topico, char* mensagem);

void mqtt_cadastra_esp_servidor();

void mqtt_inscreveCanalDispositivo();

void mqtt_enviaTempUmidServidorCentral(void* params);

void mqtt_informaEstado();

#endif