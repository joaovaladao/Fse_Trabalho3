#ifndef MQTT_H
#define MQTT_H

void mqtt_start();

int get_low_power();

void mqtt_envia_mensagem(char * topico, char * mensagem);

#endif