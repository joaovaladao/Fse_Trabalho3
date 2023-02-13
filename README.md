# Trabalho 3 - ESP32

## Alunos
|Matrícula | Aluno |
| -- | -- |
| 18/0100831  |  Gabriel Avelino |
| 18/0103431  |  Joao Victor Valadão |
| 18/0101617  |  Guilherme Richter |

## Objetivos

Esse trabalho tem com objetivo utilizar a esp32 e alguns sensores utilizando conexão MQTT e o módulo WI-FI da própria placa ESP, implementando no ThingsBoad. Utilizando de conceitos de eletrônica e sistemas embarcados para implementar esses sensores e a ESP em uma protoboard para realizar suas conexões.

## Componentes do Sistema
O Servidor Central Thingsboard será composto por:

 - Serviço disponibilizado pelo Laboratório de Software (Lappis);
 - Disponibiliza um Broker MQTT;
 - Serviço de Cadastro, registro e monitoramento de dispositivos;
 - Serviço de criação de Dashboards.

Cliente ESP32 distribuído - Energia:

 - Dev Kit ESP32;
 - Botão (Presente na placa);
 - LED (Presente na placa);
 - Sensores: Botão, Sensor Emissor de InfraVermelho, Sensor Receptor de InfraVermelho, Sensor DHT11 de temperatura, Sensor Detector de Ruído e Sensor de presença InfraVermelho;

Cliente ESP32 distribuído - Bateria:

 - Dev Kit ESP32;
 - Botão (Presente na placa);
 - LED (Presente na placa).

## Linguagem e Bibliotecas

Linguagem: 
- C

Bibliotecas: 
- <stdio.h>
- <string.h>
- <driver/adc.h>
- "nvs_flash.h"
- "esp_wifi.h"
- "esp_event.h"
- "esp_http_client.h"
- "esp_log.h"
- "freertos/semphr.h"
- <driver/ledc.h>
- "freertos/FreeRTOS.h"
- "freertos/task.h"
- "freertos/event_groups.h"
- "driver/gpio.h"
- "sdkconfig.h"
- "esp_system.h"
- "lwip/err.h"
- "lwip/sys.h"
- "mqtt.h"
- "dht11.h"

## Arquitetura de arquivos

Ao criar um projeto pelo PlataformIO já é criado uma arquitetura de arquivos, onde contém pastas como Include, lib,test e src. Falaremos da pasta src onde possui os arquivos com a lógica do projeto. Nessa pasta contém:

- <b>main.c</b>: A main é onde possui toda a lógica do projeto, onde define e ativa todos os sensores presentes, realiza a conexão WI-FI e chama o protocolo MQTT.

- <b>mqtt.c</b>: Arquivo que define o protocolo MQTT, onde inicializa e possui funções para enviar mensagens.

- <b>mqtt.h</b>: Onde exporta as funções do mqtt.c

- <b>cJSON.c</b>: Biblioteca para fazer parser dos dados.

- <b>cJSON.h</b>: Exporta as funções do cJSON.c

- <b>led_pwm.c</b>: Arquivo que define o LED (interno da ESP32) e realiza o pwm.

- <b>led_pwm.h</b>: Exporta as funções do led_pwm.c

- <b>protocol_exemples_common.h</b>: Serve para exportar as funções do git do MQTT

Além disso, temos outra branch chamada sistemaIRE, onde possui o arquivo sistemaInfraVermelho.c.

- sistemaInfraVermelho.c: Arquivo onde possui um sistema de controle remoto, possuindo um botão e um sensor Emissor de Infra Vermelho.

## Como funciona

- clonar o repositório

- instalar o PlataformIO

- Adicionar os sensores na protoboard e conectar nas portas corretas da ESP32

- Fazer o Build e o upload para a plaquinha

- Testar os sensores
