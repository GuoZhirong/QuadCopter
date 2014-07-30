#ifndef __UART_H__
#define __UART_H__


/*for internal invoke*/
void uartDmaTxISR();
void uartDmaRxISR();
void Uart3ISR();



void InitUART(void);
int UartSendData(char *data, int len);
int UartRecvData(char *data, int size);


#endif

