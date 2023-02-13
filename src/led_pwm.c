#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"

#define LED_1 2

void led_init()
{
  
  // Configuração do Timer
  ledc_timer_config_t timer_config = {
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .duty_resolution = LEDC_TIMER_8_BIT,
    .timer_num = LEDC_TIMER_0,
    .freq_hz = 1000,
    .clk_cfg = LEDC_AUTO_CLK
  };
  ledc_timer_config(&timer_config);

  // Configuração do Canal
  ledc_channel_config_t channel_config = {
    .gpio_num = LED_1,
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .channel = LEDC_CHANNEL_0,
    .timer_sel = LEDC_TIMER_0,
    .duty = 0,
    .hpoint = 0
  };
  ledc_channel_config(&channel_config);
}

void pwm_led(int i){
    if (i == 100) {
        for(int j = 0; j < 100; j++){
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, j);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
        vTaskDelay(10 / portTICK_PERIOD_MS);
        }
    }
    if (i == 0) {
        for(int j = 100; j >= 0; j--){
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, j);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
        vTaskDelay(10 / portTICK_PERIOD_MS);
        }
    }
    
}
