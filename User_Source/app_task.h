#ifndef __APPTASK_H__
#define __APPTASK_H__
#include "stdint.h"

//Variable Discription
typedef struct
{
    uint8_t water;
    uint8_t ir;
} SensorData_t;


//typedef struct {
//    uint8_t day;
//    uint8_t month;
//    uint8_t year;   // 00�99
//    uint8_t hour;
//    uint8_t min;
//    uint8_t sec;
//} DateTime_t;

//typedef struct
//{
//    DateTime_t date;
//    char msg[50];
//} LogEvent_t;

typedef struct
{
    uint8_t mode;        // 0=AUTO, 1=MANUAL
    uint8_t water;       // 0=OK, 1=DETECTED
    uint8_t ir;          // 0=NO, 1=YES
    uint16_t servo_angle;
    uint8_t state;       // 0=Normal, 1=Fault
} DisplayData_t;

typedef enum
{
    STATE_NORMAL,
    STATE_FAULT
} SystemState_t;

typedef enum
{
    MODE_AUTO,
    MODE_MANUAL
} SystemMode_t;

typedef struct
{
    uint8_t water;
    uint8_t ir;
    uint8_t button_pressed;
    uint8_t joystick;
} Event_t;


void App_RTOS_Init(void);
void Control_Task(void *arg);
void Sensor_Task(void *arg);
void Joystick_Task(void *arg)	;
void Display_Task(void *arg);
//void Logger_Task(void *arg);
//void Comm_Task(void *arg);
void Heartbeat_Task(void *arg);
//void RTC_GetDateTime(DateTime_t *dt);
void OLED_ShowStatus(DisplayData_t *disp);
void OLED_Welcome_Msg(void);
void LogEvent_Send(char *msg);
//void UART_SendLog(LogEvent_t *log);
#endif