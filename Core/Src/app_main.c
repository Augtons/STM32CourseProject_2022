#include "main.h"
#include "stdio.h"

#include "smg.h"

extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim1;

smg_t smg;
uint8_t pulse = 50;

// 1 means 20204083
// 0 means PWM
uint8_t display_mode = 0;

void app_main(void) {
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2, pulse);

  hc138_t hc138 = {
    .a0 = {GPIOC, GPIO_PIN_10},
    .a1 = {GPIOC, GPIO_PIN_11},
    .a2 = {GPIOC, GPIO_PIN_12},
  };

  hc595_t hc595 = {
    .sclk = {GPIOB, GPIO_PIN_5},
    .lclk = {GPIOB, GPIO_PIN_4},
    .ds = {GPIOB, GPIO_PIN_3}
  };
	
  smg_init(&hc138, &hc595, &smg);
  HAL_TIM_Base_Start_IT(&htim1);
	
  smg_from_string(&smg, "_____0.50");
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  simple_delay(500);
  uint8_t num = 0;
    
  switch (GPIO_Pin) {
    case KEY0_Pin: {
      if (HAL_GPIO_ReadPin(KEY0_GPIO_Port, KEY0_Pin) == 0) { // Up
        if (pulse < 100){
          pulse++;
        }
        __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2, pulse);
        HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, !(pulse > 90 || pulse < 10));
      }
      break;
    }
    case KEY1_Pin: {
      if (HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == 0) { // Down
        if (pulse > 0){
          pulse--;
        }
        __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2, pulse);
        HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, !(pulse > 90 || pulse < 10));
      }
      break;
    }
    case KEY2_Pin: {
      if (HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin) == 0) {
        display_mode = !display_mode;
      }
      break;
    }
    default:
      return;
  }
  if (display_mode) {
    smg_from_string(&smg, "20204083");
  } else {
    if (pulse >= 100) {
      smg_from_string(&smg, "_____1.00");
    } else {
      char buf[12] = {0};
      snprintf(buf, 12, "_____0.%02d", pulse);
      smg_from_string(&smg, buf);
    }
  }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim == &htim1) {
    smg_flush(&smg);
  }
}
