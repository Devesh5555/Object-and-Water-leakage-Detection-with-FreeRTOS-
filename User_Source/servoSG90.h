#ifndef __SERVOSG90_H__
#define __SERVOSG90_H__

#define MAX_SR 2500.0f
#define MIN_SR 500.0f
#define ADC_MIN     0
#define ADC_MAX     4090
#define ADC_CENTER  2750

#define ADC_left    100
#define ADC_right   3900

#define DEAD_BAND   500

#define WATER_LEVEL_OVERFLOW 2300

#include<stdio.h>
#include<stdint.h>

void servo_angle_set(uint8_t angle);
void servo_init(void);
uint16_t adc_read(uint8_t channel);
uint8_t joystick_position(uint16_t adc); //angle
uint8_t read_waterlevel(void);
uint8_t read_joystick(void);
uint8_t read_ir(void);
void OLED_Init(void);
void OLED_Clear(void);
void OLED_Print(uint8_t x, uint8_t line, char *str); // Implement Painding
void OLED_Print_Line(uint8_t line, char *str);

void OLED_Update(void);
#endif