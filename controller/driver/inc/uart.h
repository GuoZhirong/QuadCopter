#ifndef __UART_H__
#define __UART_H__


#include <stdbool.h>


#define UART_TYPE       USART1
#define UART_PERIF      RCC_APB2Periph_USART1

#define UART_DMA_IRQ    DMA1_Channel4_IRQn
#define UART_DMA_IT_TC  DMA1_IT_TC4
#define UART_DMA_CH     DMA1_Channel4

#define UART_GPIO_PERIF RCC_APB2Periph_GPIOA
#define UART_GPIO_PORT  GPIOA
#define UART_GPIO_TX    GPIO_Pin_9
#define UART_GPIO_RX    GPIO_Pin_10
 

#endif

