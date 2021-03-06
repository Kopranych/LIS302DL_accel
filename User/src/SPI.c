
#include <SPI.h>
#include "LCD_HD44780.h"


extern uint8_t spi_in[7],spi_out[7];



//-----------------------------------------------------------------------------
void SPI_init(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOE, ENABLE);// ��������� ����� ��� SPI
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);// clock SPI
	
	SPI_InitTypeDef spi;
	GPIO_InitTypeDef spi_bus;
	GPIO_InitTypeDef spi_cs;
	
	spi_bus.GPIO_Pin = SPI_MOSI|SPI_MISO|SPI_SCK;
	spi_bus.GPIO_Mode = GPIO_Mode_AF;
	spi_bus.GPIO_Speed = GPIO_Speed_50MHz;
	spi_bus.GPIO_PuPd = GPIO_PuPd_NOPULL;
	spi_bus.GPIO_OType = GPIO_OType_PP;
	
	GPIO_Init(GPIOA, &spi_bus);// �������������� ���� �����	
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);
	
	
	spi_cs.GPIO_Pin = SPI_CS;
	spi_cs.GPIO_Mode = GPIO_Mode_OUT;
	spi_cs.GPIO_Speed = GPIO_Speed_50MHz;
	spi_cs.GPIO_PuPd = GPIO_PuPd_NOPULL;
	spi_cs.GPIO_OType = GPIO_OType_PP;
	
	GPIO_Init(GPIOE, &spi_cs);// �������������� ���� �����	
	CS_OFF();
	
	spi.SPI_Direction = SPI_Direction_2Lines_FullDuplex;//������ ����� ����� � ��������
	spi.SPI_Mode = SPI_Mode_Master;// 
	spi.SPI_DataSize = SPI_DataSize_8b;//
	spi.SPI_CPOL = SPI_CPOL_High;//�������� ��������� ����� ������� (��. ������� �� ������������ ������)
	spi.SPI_CPHA = SPI_CPHA_2Edge;// ���������� ������ �� ������� ������ ��������� ��������
	spi.SPI_NSS = SPI_NSS_Soft;// slave select ����������
	spi.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;// ������������ ��������
	spi.SPI_FirstBit = SPI_FirstBit_MSB;// �������� �������� ���� ������
	spi.SPI_CRCPolynomial = 7;//�����������
	
	SPI_Init(SPI1, &spi);
	SPI_Cmd(SPI1, ENABLE);
	
}

//-----------------------------------------------------------------------------

void SPI_DMA_init(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);// ��������� DMA2
	DMA_InitTypeDef dma;
	//TX
	dma.DMA_BufferSize = 2;// 
	dma.DMA_Channel = DMA_Channel_3;// 3� ����� DMA ��� SPI1
	dma.DMA_DIR = DMA_DIR_MemoryToPeripheral;//����������� �������� �� ������ � ��������� 
	dma.DMA_FIFOMode = DMA_FIFOMode_Disable;
	dma.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;// ���������� ������� FIFO 
	dma.DMA_Memory0BaseAddr = (uint32_t)spi_out;//��������� ����� ������
	dma.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	dma.DMA_MemoryInc = DMA_MemoryInc_Enable;// �������������� ����� ��������� ������ ��� �������� ������ ���������������
	dma.DMA_Mode = DMA_Mode_Normal;
	dma.DMA_PeripheralBaseAddr =(uint32_t)&(SPI1->DR);// ����� �������� ��������� (SPI)
	dma.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;// ������ ������ ���������
	dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;// ���������� ��������� ��������� (SPI1->DR)
	dma.DMA_Priority = DMA_Priority_Medium;
	
	DMA_Init(DMA2_Stream3, &dma);
	
	NVIC_EnableIRQ(DMA2_Stream3_IRQn);//�������� ���������� ����������
	DMA_ITConfig(DMA2_Stream3, DMA_IT_TC, ENABLE);// �������� ��������� ���������� DMA_IT_TC(Transmitted complete) ��� DMA2_Stream3 �� ��������� ��������
	SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);// ��������� ������ DMA �� SPI
	
	//RX
	dma.DMA_BufferSize = 2;// 
	dma.DMA_Channel = DMA_Channel_3;// 3� ����� DMA ��� SPI1
	dma.DMA_DIR = DMA_DIR_PeripheralToMemory;//����������� �������� ������ �� ��������� � ������ 
	dma.DMA_FIFOMode = DMA_FIFOMode_Disable;
	dma.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;// ���������� ������� FIFO 
	dma.DMA_Memory0BaseAddr = (uint32_t)spi_in;//��������� ����� ������
	dma.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	dma.DMA_MemoryInc = DMA_MemoryInc_Enable;// �������������� ����� ��������� ������ ��� �������� ������ ���������������
	dma.DMA_Mode = DMA_Mode_Normal;//������� ������ ����������, �� ��������� �������� ������ ���������� �������������
	dma.DMA_PeripheralBaseAddr =(uint32_t)&(SPI1->DR);// ����� �������� ��������� (SPI)
	dma.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;//�������� �������� ������
	dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;// ������ ������������ ������ ���������
	dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;// ���������� ��������� �� ��������� (SPI1->DR)
	dma.DMA_Priority = DMA_Priority_Medium;//����� ����� �������� ��������� DMA
	
	DMA_Init(DMA2_Stream0, &dma);
	
	NVIC_EnableIRQ(DMA2_Stream0_IRQn);//�������� ���������� ����������
	DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE);// �������� ��������� ���������� DMA_IT_TC(Transmitted complete) ��� DMA2_Stream3 �� ��������� ��������
	SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, ENABLE);// �������������/��������� ������ DMA �� SPI
}


