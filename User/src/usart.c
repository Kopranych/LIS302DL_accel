#include "usart.h"

extern uint8_t buf_rx[16], buf_tx[16];



//---------------------------------------------------------------------------------
void usart_init(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	
	GPIO_InitTypeDef usart_bus;
	USART_InitTypeDef usart1;
	
	usart_bus.GPIO_Pin = TX|RX;
	usart_bus.GPIO_Mode = GPIO_Mode_AF;
	usart_bus.GPIO_Speed = GPIO_Speed_50MHz;
	usart_bus.GPIO_OType = GPIO_OType_PP;
	usart_bus.GPIO_PuPd = GPIO_PuPd_UP;
	
	GPIO_Init(USART_BUS, &usart_bus);
	GPIO_PinAFConfig(USART_BUS, GPIO_PinSource8, GPIO_AF_USART1);
	GPIO_PinAFConfig(USART_BUS, GPIO_PinSource9, GPIO_AF_USART1);
	
	usart1.USART_BaudRate = 9600;//
	usart1.USART_WordLength = USART_WordLength_8b;//
	usart1.USART_StopBits = USART_StopBits_1;
	usart1.USART_Parity = USART_Parity_No;//бит четности
	usart1.USART_Mode = USART_Mode_Tx|USART_Mode_Rx;
	usart1.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//аппаратный контроль потока RTS CTS

	USART_Init(USART3, &usart1);
	USART_Cmd(USART3, ENABLE);
	
	
	NVIC_EnableIRQ(USART3_IRQn);//разрешаем обшие прерывания
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);// локальные прерывания
}
//---------------------------------------------------------------------------------
void usart_dma_ini(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	
	//TX
	DMA_InitTypeDef dma_usart;
	dma_usart.DMA_Channel = DMA_Channel_4;
	dma_usart.DMA_PeripheralBaseAddr = (uint32_t)&USART3->DR;
	dma_usart.DMA_Memory0BaseAddr = (uint32_t)buf_tx;
	dma_usart.DMA_DIR =DMA_DIR_MemoryToPeripheral;
	dma_usart.DMA_BufferSize = sizeof(buf_tx);
	dma_usart.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	dma_usart.DMA_MemoryInc = DMA_MemoryInc_Enable;
	dma_usart.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	dma_usart.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	dma_usart.DMA_Mode = DMA_Mode_Normal;
	dma_usart.DMA_Priority = DMA_Priority_Medium;
	dma_usart.DMA_FIFOMode = DMA_FIFOMode_Disable;
	dma_usart.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
	dma_usart.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	dma_usart.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	
	DMA_Init(DMA1_Stream3, &dma_usart);
	//DMA_Cmd(DMA1_Stream3, ENABLE);
	
	NVIC_EnableIRQ(DMA1_Stream3_IRQn);
	DMA_ITConfig(DMA1_Stream3, DMA_IT_TC, ENABLE);
	USART_DMACmd(USART3, USART_DMAReq_Tx|USART_DMAReq_Rx, ENABLE);
	
	//RX
	dma_usart.DMA_Channel = DMA_Channel_4;
	dma_usart.DMA_PeripheralBaseAddr = (uint32_t)&USART3->DR;
	dma_usart.DMA_Memory0BaseAddr = (uint32_t)buf_rx;
	dma_usart.DMA_DIR = DMA_DIR_PeripheralToMemory;
	dma_usart.DMA_BufferSize = sizeof(buf_rx);
	dma_usart.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	dma_usart.DMA_MemoryInc = DMA_MemoryInc_Enable;
	dma_usart.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	dma_usart.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	dma_usart.DMA_Mode = DMA_Mode_Normal;
	dma_usart.DMA_Priority = DMA_Priority_Medium;
	dma_usart.DMA_FIFOMode = DMA_FIFOMode_Disable;
	dma_usart.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
	dma_usart.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	dma_usart.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	
	DMA_Init(DMA1_Stream1, &dma_usart);
	DMA_Cmd(DMA1_Stream1, ENABLE);
	
	NVIC_EnableIRQ(DMA1_Stream1_IRQn);
	DMA_ITConfig(DMA1_Stream1, DMA_IT_TC, ENABLE);
	USART_DMACmd(USART3, USART_DMAReq_Tx|USART_DMAReq_Rx, ENABLE);
}
//---------------------------------------------------------------------------------
void usart_txstr(char* str)
{
	while(*str)
	{
		while(!(USART_GetFlagStatus(USART3, USART_FLAG_TC)));
		USART_SendData(USART3, *str);	
		str++;
	}
}
//---------------------------------------------------------------------------------





