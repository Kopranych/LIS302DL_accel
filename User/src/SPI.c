
#include <SPI.h>
#include "LCD_HD44780.h"


extern uint8_t spi_in[7],spi_out[7];



//-----------------------------------------------------------------------------
void SPI_init(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOE, ENABLE);// тактируем порты для SPI
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);// clock SPI
	
	SPI_InitTypeDef spi;
	GPIO_InitTypeDef spi_bus;
	GPIO_InitTypeDef spi_cs;
	
	spi_bus.GPIO_Pin = SPI_MOSI|SPI_MISO|SPI_SCK;
	spi_bus.GPIO_Mode = GPIO_Mode_AF;
	spi_bus.GPIO_Speed = GPIO_Speed_50MHz;
	spi_bus.GPIO_PuPd = GPIO_PuPd_NOPULL;
	spi_bus.GPIO_OType = GPIO_OType_PP;
	
	GPIO_Init(GPIOA, &spi_bus);// иницифлизируем сами порты	
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);
	
	
	spi_cs.GPIO_Pin = SPI_CS;
	spi_cs.GPIO_Mode = GPIO_Mode_OUT;
	spi_cs.GPIO_Speed = GPIO_Speed_50MHz;
	spi_cs.GPIO_PuPd = GPIO_PuPd_NOPULL;
	spi_cs.GPIO_OType = GPIO_OType_PP;
	
	GPIO_Init(GPIOE, &spi_cs);// иницифлизируем сами порты	
	CS_OFF();
	
	spi.SPI_Direction = SPI_Direction_2Lines_FullDuplex;//полный режим прием и передача
	spi.SPI_Mode = SPI_Mode_Master;// 
	spi.SPI_DataSize = SPI_DataSize_8b;//
	spi.SPI_CPOL = SPI_CPOL_High;//наяально положение такта высокое (см. даташит на используемый датчик)
	spi.SPI_CPHA = SPI_CPHA_2Edge;// считывание данных со второго фронта тактового импульса
	spi.SPI_NSS = SPI_NSS_Soft;// slave select програмный
	spi.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;// предделитель скорости
	spi.SPI_FirstBit = SPI_FirstBit_MSB;// передача старшего бита первым
	spi.SPI_CRCPolynomial = 7;//поумолчанию
	
	SPI_Init(SPI1, &spi);
	SPI_Cmd(SPI1, ENABLE);
	
}

//-----------------------------------------------------------------------------

void SPI_DMA_init(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);// тактируем DMA2
	DMA_InitTypeDef dma;
	//TX
	dma.DMA_BufferSize = 2;// 
	dma.DMA_Channel = DMA_Channel_3;// 3й канал DMA для SPI1
	dma.DMA_DIR = DMA_DIR_MemoryToPeripheral;//направление передачи от памяти к переферии 
	dma.DMA_FIFOMode = DMA_FIFOMode_Disable;
	dma.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;// предельный уровень FIFO 
	dma.DMA_Memory0BaseAddr = (uint32_t)spi_out;//указываем адрес данных
	dma.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	dma.DMA_MemoryInc = DMA_MemoryInc_Enable;// инкрементируем адрес регистров памяти для передачи данных последовательно
	dma.DMA_Mode = DMA_Mode_Normal;
	dma.DMA_PeripheralBaseAddr =(uint32_t)&(SPI1->DR);// адрес регистра переферии (SPI)
	dma.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;// размер данных переферии
	dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;// увеличение указателя переферии (SPI1->DR)
	dma.DMA_Priority = DMA_Priority_Medium;
	
	DMA_Init(DMA2_Stream3, &dma);
	
	NVIC_EnableIRQ(DMA2_Stream3_IRQn);//включаем глобальные прерывания
	DMA_ITConfig(DMA2_Stream3, DMA_IT_TC, ENABLE);// включаем локальные прерывания DMA_IT_TC(Transmitted complete) для DMA2_Stream3 по окончанию передачи
	SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);// разрешаем работу DMA со SPI
	
	//RX
	dma.DMA_BufferSize = 2;// 
	dma.DMA_Channel = DMA_Channel_3;// 3й канал DMA для SPI1
	dma.DMA_DIR = DMA_DIR_PeripheralToMemory;//направление передачи данных от переферии к памяти 
	dma.DMA_FIFOMode = DMA_FIFOMode_Disable;
	dma.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;// предельный уровень FIFO 
	dma.DMA_Memory0BaseAddr = (uint32_t)spi_in;//указываем адрес данных
	dma.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	dma.DMA_MemoryInc = DMA_MemoryInc_Enable;// инкрементируем адрес регистров памяти для передачи данных последовательно
	dma.DMA_Mode = DMA_Mode_Normal;//предача данных циклически, то следующая передача данных начинается автоматически
	dma.DMA_PeripheralBaseAddr =(uint32_t)&(SPI1->DR);// адрес регистра переферии (SPI)
	dma.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;//пакетная передача данных
	dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;// размер передаваемых данных переферии
	dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;// увеличение указателя на переферию (SPI1->DR)
	dma.DMA_Priority = DMA_Priority_Medium;//важен когда работают несколько DMA
	
	DMA_Init(DMA2_Stream0, &dma);
	
	NVIC_EnableIRQ(DMA2_Stream0_IRQn);//включаем глобальные прерывания
	DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE);// включаем локальные прерывания DMA_IT_TC(Transmitted complete) для DMA2_Stream3 по окончанию передачи
	SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, ENABLE);// согласовываем/разрешаем работу DMA со SPI
}


