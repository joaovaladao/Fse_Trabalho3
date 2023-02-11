#include <stdio.h>
#include <string.h>
#include <driver/adc.h>
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "freertos/semphr.h"



#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_system.h"


#include "lwip/err.h"
#include "lwip/sys.h"

#include "mqtt.h"
#include "dht11.h"

//#define VOICE_SENSOR_GPIO_NUM 15
//#define VOICE_SENSOR_ADC_CHANNEL ADC2_CHANNEL_3

SemaphoreHandle_t conexaoWifiSemaphore;
SemaphoreHandle_t conexaoMQTTSemaphore;

#define IR_SENSOR_GPIO_NUM 36

//#define IR_OBSTACLE_AVOIDANCE_SENSOR_GPIO_NUM 22

//static const char *TAG = "Infrared Obstacle Avoidance Sensor";

/* The examples use WiFi configuration that you can set via project configuration menu
   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_SSID      CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_MAXIMUM_RETRY  CONFIG_ESP_MAXIMUM_RETRY

#if CONFIG_ESP_WIFI_AUTH_OPEN
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
#elif CONFIG_ESP_WIFI_AUTH_WEP
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
#elif CONFIG_ESP_WIFI_AUTH_WPA_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WAPI_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK
#endif

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char *TAG = "wifi station";

static int s_retry_num = 0;


static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (pasword len => 8).
             * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
             * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
	     * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
             */
            .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}   

void conectadoWifi(void * params)
{
  while(true)
  {

        UBaseType_t uxSemaphoreCount = uxSemaphoreGetCount(conexaoMQTTSemaphore);
    printf("Value of semaphore before taking it: %d\n", uxSemaphoreCount);
    BaseType_t xSemaphoreTakeResult = xSemaphoreTake(conexaoWifiSemaphore, portMAX_DELAY);
        if (xSemaphoreTakeResult == pdTRUE)
        {
            mqtt_start();
        }
        else
        {
            printf("Semaphore could not be taken, value of xSemaphoreTakeResult: %d\n", xSemaphoreTakeResult);
        }
  }
}

// int voice_sensor_init(void)
// {
//     // // Configure the input pin for the voice sensor
//     // printf("Configuring voice sensor GPIO...\n");
//     // gpio_config_t io_conf = {
//     //     .intr_type = GPIO_INTR_DISABLE,
//     //     .mode = GPIO_MODE_INPUT,
//     //     .pin_bit_mask = 1LL << VOICE_SENSOR_GPIO_NUM,
//     //     .pull_down_en = 0,
//     //     .pull_up_en = 0
//     // };
//     // gpio_config(&io_conf);

//     adc1_config_width(ADC_WIDTH_BIT_12);
//     adc1_config_channel_atten(VOICE_SENSOR_ADC_CHANNEL, ADC_ATTEN_DB_11);

//     return 0;
// }

void IR_obstacle_avoidance_sensor_init(void)
{
    // gpio_config_t io_conf;
    // io_conf.intr_type = GPIO_INTR_DISABLE;
    // io_conf.mode = GPIO_MODE_INPUT;
    // io_conf.pin_bit_mask = (1ULL<<IR_OBSTACLE_AVOIDANCE_SENSOR_GPIO_NUM);
    // io_conf.pull_down_en = 0;
    // io_conf.pull_up_en = 0;
    // gpio_config(&io_conf);

    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC2_CHANNEL_3, ADC_ATTEN_DB_11);
}

void trataComunicacaoComServidor(void * params)
{
  char mensagem[50];

  //Initialize DHT11
  DHT11_init(GPIO_NUM_4);

    UBaseType_t uxSemaphoreCount = uxSemaphoreGetCount(conexaoMQTTSemaphore);
    printf("Value of semaphore before taking it: %d\n", uxSemaphoreCount);

  if(xSemaphoreTake(conexaoMQTTSemaphore, portMAX_DELAY))
  {

    while(true)
    {
       sprintf(mensagem, "{\"temperature\": %d}",  DHT11_read().temperature);
       mqtt_envia_mensagem("v1/devices/me/telemetry", mensagem);
       printf("%s \n", mensagem);

       sprintf(mensagem, "{\"umidade\": %d}", DHT11_read().humidity);
       mqtt_envia_mensagem("v1/devices/me/telemetry", mensagem);
       printf("%s \n", mensagem);

       //int sensor_value = sensor_voice();
       //int value = gpio_get_level(VOICE_SENSOR_GPIO_NUM);
        //printf("voice sensor value: %d\n", value);
        // adc1_config_width(ADC_WIDTH_BIT_12);
        // adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_0);
        // uint32_t voice_sensor_value = adc1_get_raw(ADC1_CHANNEL_0);
        // //ESP_LOGI("Voice Sensor Value", "Value: %d", voice_sensor_value);
        // printf("Voice sensor value: %ld\n", voice_sensor_value);

    //     int reading = adc1_get_raw(VOICE_SENSOR_ADC_CHANNEL);
    //     printf("Voice Sensor Reading: %d\n", reading);

    //     sprintf(mensagem, "{\"som\": %d}", reading);
    //    mqtt_envia_mensagem("v1/devices/me/telemetry", mensagem);
    //    printf("%s \n", mensagem);

    //    int proximidade = gpio_get_level(IR_OBSTACLE_AVOIDANCE_SENSOR_GPIO_NUM);;
    //     printf("proximidade: %d", proximidade);

        //int raw_value = adc1_get_raw(ADC2_CHANNEL_3);

        // Convert the raw ADC value to a voltage value
        //float voltage = adc1_to_voltage(raw_value);

        // Print the ADC value and voltage to the console
        //printf("ADC Value: %d, Voltage: %.2fV\n", raw_value, voltage);


    //     sprintf(mensagem, "{\"proximidade\": %d}", proximidade);
    //    mqtt_envia_mensagem("v1/devices/me/telemetry", mensagem);
    //    printf("%s \n", mensagem);

       // outros sensores

       vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }
  else 
  {
    printf("Não foi possível se conectar ao servidor MQTT");
  }
}

void app_main(void){

    //Initialize NVS
    esp_err_t ret = nvs_flash_init();

    // //Initialize DHT11
    // DHT11_init(GPIO_NUM_4);

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    //voice_sensor_init();
    IR_obstacle_avoidance_sensor_init();
    
    conexaoWifiSemaphore = xSemaphoreCreateBinary();
    conexaoMQTTSemaphore = xSemaphoreCreateBinary();


    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();

    xSemaphoreGive(conexaoMQTTSemaphore);
    xSemaphoreGive(conexaoWifiSemaphore);

    xTaskCreate(&conectadoWifi,  "Conexão ao MQTT", 4096, NULL, 1, NULL);
    xTaskCreate(&trataComunicacaoComServidor, "Comunicação com Broker", 4096, NULL, 1, NULL);
    //xTaskCreate(&sensor_voice, "Sensor de voz", 4096, NULL, 1, NULL);    
}