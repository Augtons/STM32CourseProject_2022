#include "main.h"
#include "smg.h"
#include "stdio.h"
#include "24cxx.h"
#include "pid.h"

static void init_smg();

extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;

extern ADC_HandleTypeDef hadc1;

uint16_t adc_val = 0;
uint16_t data = 0;

uint8_t pulse = 88;

smg_t smg;
pid_t pid = {
  .kp = 1.5,
  .ki = 0.00002,
  .max = 4095,
  .min = 0
};

uint8_t display_mode = 0;


void app_main() {
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2, pulse);
  
  AT24CXX_Init();

  init_smg();
  
  HAL_TIM_Base_Start_IT(&htim1);
  HAL_TIM_Base_Start_IT(&htim2);

  HAL_ADCEx_Calibration_Start(&hadc1);
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&adc_val, 1);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  simple_delay(500);
    
  switch (GPIO_Pin) {
    case KEY0_Pin: {
      if (HAL_GPIO_ReadPin(KEY0_GPIO_Port, KEY0_Pin) == 0) { // Read
        uint16_t high = (AT24CXX_ReadOneByte(2) << 8) & 0xFF00;
        uint16_t low = AT24CXX_ReadOneByte(3) & 0xFF;
        
        data = high | low;
      }
      break;
    }
    case KEY1_Pin: {
      if (HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == 0) { // Save
        uint16_t _data = adc_val;
        uint8_t high = (_data >> 8) & 0xFF;
        uint8_t low = (_data) & 0xFF;
       
        
        AT24CXX_WriteOneByte(2, high);
        AT24CXX_WriteOneByte(3, low);
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
  }
}


static void init_smg() {
  hc138_t hc138 = {
    .a0 = {GPIOC, GPIO_PIN_10},		// 低位
    .a1 = {GPIOC, GPIO_PIN_11},		
    .a2 = {GPIOC, GPIO_PIN_12},		// 高位
  };

  hc595_t hc595 = {
    .sclk = {GPIOB, GPIO_PIN_5},	// 上升沿下一位
    .lclk = {GPIOB, GPIO_PIN_4},	// 上升沿并行输出
    .ds = {GPIOB, GPIO_PIN_3}		// 数据输入端
  };
  smg_init(&hc138, &hc595, &smg);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  static char buf[16] = {0};
  static int count = 0;
  if (htim == &htim1) {
    smg_flush(&smg);
    pulse = (float)adc_val/4095 * 100;
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2, pulse);
    HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, !(pulse > 90 || pulse < 10));
  }
  if (htim == &htim2) {
    if (display_mode) {
      return;
    }
    float val = (float)adc_val/4095 * 3.3f;
    float dataf = (float)data/4095 * 3.3f;
    //sprintf(buf, "%.2f_%.2f_", dataf, val);
    val = pid_update(&pid, val);
    sprintf(buf, "%04d%04.2fV", data, val);
    
    smg_from_string(&smg, buf);
  }
}