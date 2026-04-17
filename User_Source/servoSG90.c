#include "servoSG90.h"
#include "main.h"
#include "ssd1306_oled.h"
#include "fonts.h"
#include "ssd1306_conf.h"


extern TIM_HandleTypeDef htim2;
extern ADC_HandleTypeDef hadc1;

extern I2C_HandleTypeDef hi2c1;

uint16_t watersensor_adc;


void servo_angle_set(uint8_t angle)
{
	float pulse;
	if(angle > 180U) angle = 180U;
	
	pulse = MIN_SR + ((MAX_SR - MIN_SR) * (float)angle / 180.0f);
	
	__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, (uint32_t)pulse);
}

void servo_init(void)
{
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
}

uint16_t adc_read(uint8_t channel)
{
		/** Configure Regular Channel  */
	ADC_ChannelConfTypeDef sConfig = {0};
		
  sConfig.Channel = (channel == 1)? ADC_CHANNEL_1 : ADC_CHANNEL_2 ;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
	
  	uint16_t adc_value;
		HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
    {
        adc_value = HAL_ADC_GetValue(&hadc1);
    }
    HAL_ADC_Stop(&hadc1);
		return adc_value;
}


uint8_t joystick_position(uint16_t adc)
{		
    if(adc < ADC_left)
			return 0; //left
		else if(adc > ADC_right)
			return 180; // right
		else
			return 90; // center
}

uint8_t read_waterlevel(void)
{
			watersensor_adc = adc_read(1);
			return ( watersensor_adc > WATER_LEVEL_OVERFLOW );
}

uint8_t read_joystick(void)
{
	return joystick_position(adc_read(2));
}

uint8_t read_ir(void)
{
	return (HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_3) == GPIO_PIN_RESET);

}

void OLED_Init(void)
{
	ssd1306_Init();
}

void OLED_Clear(void)
{
    ssd1306_Fill(Black);
    ssd1306_UpdateScreen();
}

void OLED_Print(uint8_t x, uint8_t line, char *str)
{
    // Each line = 8 pixels height (for 6x8 or 7x10 adjust accordingly)
    uint8_t y = line * 10;  

    ssd1306_SetCursor(x, y);
    ssd1306_WriteString(str, Font_6x8, White);
}

void OLED_Update(void)
{
    ssd1306_UpdateScreen();
}
void OLED_Print_Line(uint8_t line, char *str)
{
    uint8_t y = line * 10;

    ssd1306_SetCursor(0, y);
    ssd1306_WriteString(str, Font_6x8, White);
}





