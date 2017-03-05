#ifndef USART_H_
#define USART_H_

#include "stm32f4xx.h"


#define USART_BUS GPIOD
#define TX 				GPIO_Pin_8
#define RX 				GPIO_Pin_9		


void usart_init(void);
void usart_dma_ini(void);
void usart_txstr(char* str);
#endif

