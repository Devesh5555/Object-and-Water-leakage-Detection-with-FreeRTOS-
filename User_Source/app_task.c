
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "servoSG90.h"
#include "app_task.h"
#include "main.h"
#include "ssd1306_oled.h"
#include "fonts.h"
#include "ssd1306_conf.h"
#include "string.h"

extern UART_HandleTypeDef huart2;

/* Task Handles */
TaskHandle_t controlTaskHandle;

/* Queues */
QueueHandle_t sensorQueue;
QueueHandle_t displayQueue;
QueueHandle_t logQueue;
QueueHandle_t commQueue;

 // Create tasks, queues, etc
void App_RTOS_Init(void)
{
	//Interrupt Setup for Button
		HAL_NVIC_SetPriority(EXTI2_IRQn,5,0);//
	  HAL_NVIC_EnableIRQ(EXTI2_IRQn);
	
	 /* Create Queues */
    sensorQueue   = xQueueCreate(5, sizeof(SensorData_t));
    displayQueue  = xQueueCreate(1, sizeof(DisplayData_t));

    /* Create Tasks */
    xTaskCreate(Control_Task, "CTRL", 256, NULL, 3, &controlTaskHandle);
    xTaskCreate(Sensor_Task, "SNS", 256, NULL, 2, NULL);
    xTaskCreate(Display_Task, "DISP", 1024, NULL, 2, NULL);
    xTaskCreate(Heartbeat_Task, "HB", 128, NULL, 0, NULL);
	
		HAL_UART_Transmit(&huart2,(uint8_t*)"Application Initialized\n",23,1000);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (GPIO_Pin == GPIO_PIN_2)
    {
        vTaskNotifyGiveFromISR(controlTaskHandle, &xHigherPriorityTaskWoken);

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void Sensor_Task(void *arg)
{
    SensorData_t data;

    while (1)
    {
        data.water = read_waterlevel();
        data.ir    = read_ir();

        xQueueSend(sensorQueue, &data, 0);

        vTaskDelay(pdMS_TO_TICKS(100));
//				HAL_UART_Transmit(&huart2,(uint8_t*)"Sensor_Task Working\n",20,1000);
    }
}

//void Control_Task(void *arg)
//{
//    SensorData_t sensor;
//    DisplayData_t disp;
//		disp.ir=0;
//		disp.mode=0;//AUTO
//		disp.servo_angle = 180;
//		disp.state=0;
//		disp.water=0;
//	
//	
//    while (1)
//    {
//        /* Wait for sensor data */
//        if (xQueueReceive(sensorQueue, &sensor, pdMS_TO_TICKS(110)))
//        {
//						disp.water = sensor.water;
//            disp.ir    = sensor.ir;
//					
//            if (sensor.water || sensor.ir )
//            {	
//								disp.state = 1; // Fault
//								if(sensor.water)
//								{
//                servo_angle_set(0);  // close valve							  
//                disp.servo_angle = 0;
//								}
//						}
//						else
//						{
//							disp.state = 0;
//							servo_angle_set(180); 
//						}
//						xQueueSend(displayQueue, &disp, 0);
//         }
//        
//				
//				// 3. Mode switch (from ISR)
//        if (ulTaskNotifyTake(pdTRUE, 0))
//        {
//					disp.mode = (disp.mode)?0:1;
//        }
//				
//				// 2. Joystick (only in MANUAL)
//        if (disp.mode == 1)
//        {
//                	uint8_t angle = read_joystick();
//									if(angle != 90)
//									{
//										disp.servo_angle = angle;
//										servo_angle_set(angle);
//									}
//        }
//        
//				
//				xQueueSend(displayQueue, &disp, 0);

//        vTaskDelay(pdMS_TO_TICKS(100));
//						HAL_UART_Transmit(&huart2,(uint8_t*)"Control Task Working\n",22,1000);
//    }
//}

void Control_Task(void *arg)
{
    SensorData_t sensor;
    DisplayData_t disp;
    Event_t event = {0};

    SystemState_t state = STATE_NORMAL;
    SystemMode_t  mode  = MODE_AUTO;

    uint8_t servo_angle = 180;

    while (1)
    {
        /* 1. Get sensor data */
        if (xQueueReceive(sensorQueue, &sensor, pdMS_TO_TICKS(100)))
        {
            event.water = sensor.water;
            event.ir    = sensor.ir;
					
						/* 3. State Transition */
        switch (state)
        {
            case STATE_NORMAL:
                if (event.water || event.ir)
                    state = STATE_FAULT;
                break;

            case STATE_FAULT:
                if (!(event.water || event.ir))
                    state = STATE_NORMAL;
                break;
        }
				
        }

        /* 2. Button Event */
        if (ulTaskNotifyTake(pdTRUE, 0))
        {
            mode = (mode == MODE_AUTO) ? MODE_MANUAL : MODE_AUTO;
        }

        

        /* 4. Action based on STATE + MODE */

        if (mode == MODE_AUTO)
        {
            if (state == STATE_FAULT)
            {
                servo_angle = 0;   // Close
            }
            else
            {
                servo_angle = 180; // Open
            }
        }
        else // MANUAL
        {
            uint8_t angle = read_joystick();
            if (angle != 90)
                servo_angle = angle;
        }

        servo_angle_set(servo_angle);

        /* 5. Prepare Display Data */
        disp.water = event.water;
        disp.ir    = event.ir;
        disp.mode  = mode;
        disp.state = state;
        disp.servo_angle = servo_angle;

        /* 6. Send to display (LATEST ONLY) */
        xQueueOverwrite(displayQueue, &disp);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}


void Display_Task(void *arg)
{
    DisplayData_t disp;

    while (1)
    {
        if (xQueueReceive(displayQueue, &disp, portMAX_DELAY))
        {
					HAL_UART_Transmit(&huart2,(uint8_t*)"DISP\n",5,100);
					OLED_ShowStatus(&disp);
//					HAL_UART_Transmit(&huart2,(uint8_t*)"OLED Screen Updated\n",21,1000);
			  }
    }
}

void OLED_ShowStatus(DisplayData_t *disp)
{
    ssd1306_Fill(Black);
		
			if(disp->state == 0)
				OLED_Print_Line(1, "*** NORMAL ***");
			else
				OLED_Print_Line(1, "*** FAULT ***");
			
			if(!(disp->water || disp->ir))
			{
				OLED_Print_Line(2, "WL:NRM   IR:NOBJ");
			}
			else
			{
				if(disp->ir && disp->water )
					OLED_Print_Line(2, "WL:OVRF  IR:OBJD");
				else if(disp->water)
					OLED_Print_Line(2, "WL:OVERF  IR:NOBJ");
				else
					OLED_Print_Line(2, "WL:NRM   IR:OBJD");
			}
			
			if(disp->mode==0)
				OLED_Print_Line(3, "MODE:AUTO");
			else
				OLED_Print_Line(3, "MODE:MANUAL");
			
			if(disp->servo_angle == 180)
				OLED_Print_Line(4, "Volve:Close");
			else if(disp->servo_angle == 0)
				OLED_Print_Line(4, "Volve:Open");

    
			
}


void Heartbeat_Task(void *arg)
{
    while (1)
    {
        HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_8);
        vTaskDelay(pdMS_TO_TICKS(500));
//				HAL_UART_Transmit(&huart2,(uint8_t*)"Heartbeat_Task Working\n",26,1000);
    }
}




void OLED_Welcome_Msg(void)
{
		// Set cursor to a position (centered for 128x64 display, using 11x18 font)
    ssd1306_SetCursor(10, 14); // Adjust as needed for your display/font

    // Print the welcome message
    ssd1306_WriteString("SMART NODE v1", Font_11x18, White);
    ssd1306_SetCursor(10, 35);
    ssd1306_WriteString("Initializing...", Font_11x18, White);
    // Update the display
    ssd1306_UpdateScreen();
}
