

#include <string.h>
#include "stm32f10x.h"

/*FreeRtos includes*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"

#include "uart.h"



#define UART_TYPE       USART3
#define UART_PERIF      RCC_APB1Periph_USART3
#define UART_GPIO_PERIF RCC_APB2Periph_GPIOA
#define UART_GPIO_PORT  GPIOA
#define UART_GPIO_TX    GPIO_Pin_9
#define UART_GPIO_RX    GPIO_Pin_10
#define UART_DR_BASE	USART3->DR


#define DMA_TX_Channel	DMA1_Channel4
#define DMA_RX_Channel	DMA1_Channel5


#define TxBufferSize	128
#define RxBufferSize	128
#define RxFifoSize		32

static char TxBuffer[TxBufferSize];
static char RxBuffer[RxBufferSize];
static char RxFifo[RxFifoSize];
static int RxFifoCnt=0;
static int RxBufCnt=0;

static volatile char TxBusy = 0;

DMA_InitTypeDef TxDMAHandle;
DMA_InitTypeDef RxDMAHandle;



void uartDmaTxISR()
{
	DMA_ITConfig(DMA_TX_Channel, DMA_IT_TC, DISABLE);
	DMA_ClearITPendingBit(DMA1_IT_TC4);
	USART_DMACmd(UART_TYPE, USART_DMAReq_Tx, DISABLE);
	DMA_Cmd(DMA_TX_Channel, DISABLE);
  /*
    DMA_Cmd(DMA_TX_Channel, DISABLE);
    DMA_ClearFlag(DMA1_FLAG_TC4);
    */
	TxBusy = 0;
}

void uartDmaRxISR()
{
    DMA_Cmd(DMA_RX_Channel, DISABLE);
    DMA_ClearFlag(DMA1_FLAG_TC2);
}

void Uart3ISR()
{
	do
	{
		RxBuffer[RxBufCnt++] = RxFifo[RxFifoCnt++];
		if (RxBufCnt == RxBufferSize)
			RxBufCnt = 0;
	}while((RxFifoCnt < (RxFifoSize - DMA_GetCurrDataCounter(DMA_RX_Channel))));

	if(RxFifoCnt == RxFifoSize) 
	{
		RxFifoCnt = 0;  
	}
}


static void ConfigureUartDMA(void)
{
	/* DMA1 Channel (triggered by USART_Tx event) Config */
	DMA_DeInit(DMA_TX_Channel);  
	TxDMAHandle.DMA_PeripheralBaseAddr = UART_DR_BASE;
	TxDMAHandle.DMA_MemoryBaseAddr = (uint32_t)TxBuffer;
	TxDMAHandle.DMA_DIR = DMA_DIR_PeripheralDST;
	TxDMAHandle.DMA_BufferSize = 0;
	TxDMAHandle.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	TxDMAHandle.DMA_MemoryInc = DMA_MemoryInc_Enable;
	TxDMAHandle.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	TxDMAHandle.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	TxDMAHandle.DMA_Mode = DMA_Mode_Normal;
	TxDMAHandle.DMA_Priority = DMA_Priority_VeryHigh;
	TxDMAHandle.DMA_M2M = DMA_M2M_Disable;
	//DMA_Init(DMA_TX_Channel, &TxDMAHandle);


	/* DMA1 Channel (triggered by USART2 Rx event) Config */
	DMA_DeInit(DMA_RX_Channel);  
	RxDMAHandle.DMA_PeripheralBaseAddr = UART_DR_BASE;
	RxDMAHandle.DMA_MemoryBaseAddr = (uint32_t)RxFifo;
	RxDMAHandle.DMA_DIR = DMA_DIR_PeripheralSRC;
	RxDMAHandle.DMA_BufferSize = RxFifoSize;
	TxDMAHandle.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	TxDMAHandle.DMA_MemoryInc = DMA_MemoryInc_Enable;
	TxDMAHandle.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	TxDMAHandle.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	TxDMAHandle.DMA_Mode = DMA_Mode_Circular;
	TxDMAHandle.DMA_Priority = DMA_Priority_VeryHigh;
	TxDMAHandle.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA_RX_Channel, &RxDMAHandle);

	/* Enable DMA1_Channel Transfer Complete interrupt */
	//DMA_ITConfig(DMA_TX_Channel, DMA_IT_TC, ENABLE);  

}



static void ConfigureUartNvic(void)
{
	
    NVIC_SetPriority(USART3_IRQn, 0x02); 
    NVIC_SetPriority(DMA1_Channel4_IRQn, 0x03); 
    NVIC_SetPriority(DMA1_Channel5_IRQn, 0x04); 
    NVIC_EnableIRQ(USART3_IRQn);
	NVIC_EnableIRQ(DMA1_Channel4_IRQn);
	NVIC_EnableIRQ(DMA1_Channel5_IRQn);
}


void InitUART(void)
{
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
  
	RCC_APB2PeriphClockCmd(UART_GPIO_PERIF, ENABLE);
	RCC_APB2PeriphClockCmd(UART_PERIF, ENABLE);


	// Configure USART_Rx as input floating 
	GPIO_InitStructure.GPIO_Pin = UART_GPIO_RX;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(UART_GPIO_PORT, &GPIO_InitStructure);

	//Configure USART_Tx as alternate function push-pull 
	GPIO_InitStructure.GPIO_Pin = UART_GPIO_TX;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(UART_GPIO_PORT, &GPIO_InitStructure);

	//Configure USART
	USART_DeInit(UART_TYPE);
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(UART_TYPE, &USART_InitStructure);

	//configure tx dma
	DMA_DeInit(DMA_TX_Channel);  
	TxDMAHandle.DMA_PeripheralBaseAddr = UART_DR_BASE;
	TxDMAHandle.DMA_MemoryBaseAddr = (uint32_t)TxBuffer;
	TxDMAHandle.DMA_DIR = DMA_DIR_PeripheralDST;
	TxDMAHandle.DMA_BufferSize = 0;
	TxDMAHandle.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	TxDMAHandle.DMA_MemoryInc = DMA_MemoryInc_Enable;
	TxDMAHandle.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	TxDMAHandle.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	TxDMAHandle.DMA_Mode = DMA_Mode_Normal;
	TxDMAHandle.DMA_Priority = DMA_Priority_VeryHigh;
	TxDMAHandle.DMA_M2M = DMA_M2M_Disable;
	
	//uart nvic
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	//tx dma nvic
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	
	//Enable USART_Rx DMA Receive request 
	//USART_DMACmd(UART_TYPE, USART_DMAReq_Rx|USART_DMAReq_Tx, ENABLE);

	// Enable USART_Rx Receive interrupt
	USART_ITConfig(UART_TYPE, USART_IT_RXNE, ENABLE);


	// Enable DMA1 Channel_Tx
	//DMA_Cmd(DMA_TX_Channel, ENABLE);
	// Enable DMA1 Channel6 
	//DMA_Cmd(DMA_RX_Channel, ENABLE);  

	// Enable the USART_Tx 
	USART_Cmd(UART_TYPE, ENABLE);
	
}


#define CCR_ENABLE_SET  ((uint32_t)0x00000001)

int UartSendData(char *data, int len)
{
	
    memcpy(TxBuffer, data, len);
    TxDMAHandle.DMA_BufferSize = len;
    // Wait for DMA to be free
    while(DMA_TX_Channel->CCR & CCR_ENABLE_SET);
	
    DMA_Init(DMA_TX_Channel, &TxDMAHandle);
    // Enable the Transfer Complete interrupt
    DMA_ITConfig(DMA_TX_Channel, DMA_IT_TC, ENABLE);
    USART_DMACmd(UART_TYPE, USART_DMAReq_Tx, ENABLE);
    DMA_Cmd(DMA_TX_Channel, ENABLE);

	TxBusy = 1;
	return 0;
}

int UartRecvData(char *data, int size)
{
	return 0;
}

