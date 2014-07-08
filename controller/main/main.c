#include "stm32f10x.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "nvic.h"
#include "usec_time.h"
#include "uart.h"

#include <stdio.h>
#include <string.h>


void delay_ms(uint32_t count)
{
   volatile uint32_t i,j;
    for (i=count;i>0;i--)
        for (j=2395;j>0;j--);
}

void systemPowerOn(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  // Enable GPIO
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

  // Remap PB4
 // GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST , ENABLE);
 
  //Initialize the LED pins as an output
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  GPIO_SetBits(GPIOB, GPIO_Pin_6);		//power on
}

void LedInit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
  //Initialize the LED pins as an output
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_10 | GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  //Initialize the LED pins as an output
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);


  //Turn off the LED:s
  GPIO_SetBits(GPIOB, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_10 | GPIO_Pin_11);
  GPIO_SetBits(GPIOC, GPIO_Pin_4 | GPIO_Pin_5);
  
}

void BeepInit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  //Initialize the LED pins as an output
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIO_SetBits(GPIOB, GPIO_Pin_7);
		delay_ms(500);
  GPIO_ResetBits(GPIOB, GPIO_Pin_7);
  												  
}

void LedTestLoop(void)
{
	char flag = 0;
	int ii;
	
	for(ii=0; ii<10; ii++)
	{
		if (flag)
		{		
		  GPIO_SetBits(GPIOB, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_10 | GPIO_Pin_11);
		  GPIO_SetBits(GPIOC, GPIO_Pin_4 | GPIO_Pin_5);
		}
		else
		{			
		  GPIO_ResetBits(GPIOB, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_10 | GPIO_Pin_11);
		  GPIO_ResetBits(GPIOC, GPIO_Pin_4 | GPIO_Pin_5);
		}

		flag = ~flag;
		delay_ms(300);
	}
}

void test(void)
{
	systemPowerOn();
	LedInit();
	//BeepInit();
	LedTestLoop();
}

void ledBlink1(void *arg)
{
	char flag = 0;
	
	while(1)
	{
		if (flag)
		{		
		  GPIO_SetBits(GPIOB, GPIO_Pin_0); 
		}
		else
		{			
		  GPIO_ResetBits(GPIOB, GPIO_Pin_0); 
		}

		flag = ~flag;
		vTaskDelay(M2T(500));
	} 
}


void ledBlink2(void *arg)
{
	char flag = 0;
	char *buf = "this is frame\n";
	
	while(1)
	{
		if (flag)
		{		
		  GPIO_SetBits(GPIOB, GPIO_Pin_10); 
		}
		else
		{			
		  GPIO_ResetBits(GPIOB, GPIO_Pin_10); 
		}

		UartSendData(buf, strlen(buf));
	
		flag = ~flag;
		vTaskDelay(M2T(100));
	} 
}


void SystemTaskInit(void)
{
	nvicInit();
	initUsecTimer();
	systemPowerOn();
	InitUART();
	
	LedInit();
	LedTestLoop();
	
	xTaskCreate(ledBlink1, (const char * const)"LEDBLINK1",
              2*configMINIMAL_STACK_SIZE, NULL, /*Piority*/2, NULL);
	xTaskCreate(ledBlink2, (const char * const)"LEDBLINK2",
              2*configMINIMAL_STACK_SIZE, NULL, /*Piority*/2, NULL);

	vTaskStartScheduler();
	while(1);
}

int main(void)
{
	SystemTaskInit();

	return 0;
}

