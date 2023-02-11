#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <driver/ledc.h>


#define SWITCH_GPIO 15
#define LED_GPIO 2
#define IR_EMITTER_GPIO_NUM 4
#define IR_RECEIVER_GPIO_NUM 23

// void InfraRedEmissorTask(void *pvParameter)
// {
//     esp_rom_gpio_pad_select_gpio(IR_EMITTER_GPIO_NUM);
//     gpio_set_direction(IR_EMITTER_GPIO_NUM, GPIO_MODE_OUTPUT);

//     while (true) {
//         // Turn on the IR emitter
//         gpio_set_level(IR_EMITTER_GPIO_NUM, 1);
//         vTaskDelay(300 / portTICK_PERIOD_MS);

//         // Turn off the IR emitter
//         gpio_set_level(IR_EMITTER_GPIO_NUM, 0);
//         vTaskDelay(300 / portTICK_PERIOD_MS);
//     }
// }

void ir_receiver_task()
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1 << IR_RECEIVER_GPIO_NUM);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    // while (true) {
    //     if (gpio_get_level(IR_RECEIVER_GPIO_NUM) == 1) {
    //         printf("IR signal detected\n");
    //     }
    //     vTaskDelay(10 / portTICK_PERIOD_MS);
    // }
}






void InfraRedEmissorTask()
{
    esp_rom_gpio_pad_select_gpio(IR_EMITTER_GPIO_NUM);
    gpio_set_direction(IR_EMITTER_GPIO_NUM, GPIO_MODE_OUTPUT);
}


void switch_task(void *pvParameter)
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL<<SWITCH_GPIO);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    while(1) {
        if(gpio_get_level(SWITCH_GPIO) == 0) {
            gpio_set_level(LED_GPIO, 1);
            // printf("Ligou infra!");
            // xTaskCreate(InfraRedEmissorTask, "IR_task", 2048, NULL, 5, NULL);
            // gpio_set_level(LED_EXT,1);
            gpio_set_level(IR_EMITTER_GPIO_NUM, 1);
            if (gpio_get_level(IR_RECEIVER_GPIO_NUM) == 1) {
              printf("Sinal recebido\n");
              vTaskDelay(100 / portTICK_PERIOD_MS);
            }
            else{
              printf("SEM SINAL!!!");
            }

        } else {
            gpio_set_level(LED_GPIO, 0);
            // gpio_set_level(LED_EXT,0);
            gpio_set_level(IR_EMITTER_GPIO_NUM, 0);
        }
    }
}


void app_main()
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL<<LED_GPIO);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
    InfraRedEmissorTask();
    ir_receiver_task();

    xTaskCreate(&switch_task, "switch_task", 2048, NULL, 5, NULL);
}
