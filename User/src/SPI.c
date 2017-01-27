
#include <SPI.h>
#include "LCD_HD44780.h"

uint8_t spi_in[7],spi_out[7];
uint8_t Spi_work = 0;//флаг работы SPI
extern uint8_t buffer[10];
int8_t buf[2];

//-----------------------------------------------------------------------------
//RX
void DMA2_Stream0_IRQHandler(void)
{
	uint8_t temp = 0;
	if(DMA_GetITStatus(DMA2_Stream0, DMA_IT_TCIF0))//если прием выполнен
	{
		DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TCIF0);//очищаем флаг приема
		CS_OFF();//выключаем чип селект
		Spi_work = 0;//SPI не работает выключаем флаг
		
		temp = spi_in[1];//X
/*		//переводим данные с +-128 на от 0 до 256
		if(temp >= 0x80)
		{
			temp = temp - 0x80;
		}
		else
			temp = temp + 0x80;
*/		
		buffer[0] = temp;
		
		temp = spi_in[3];//Y
		//переводим данные с +-128 на от 0 до 256
/*		if(temp >= 0x80)
		{
			temp = temp - 0x80;
		}
		else
			temp = temp + 0x80;
*/		
		buffer[1] = temp;
		
		temp = spi_in[5];//Z
/*		//переводим данные с +-128 на от 0 до 256
		if(temp >= 0x80)
		{
			temp = temp - 0x80;
		}
		else
			temp = temp + 0x80;
*/		
		buffer[2] = temp;			

	}
}

//-----------------------------------------------------------------------------
//TX
void DMA2_Stream3_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA2_Stream3, DMA_IT_TCIF3))//если 
	{
		DMA_ClearITPendingBit(DMA2_Stream3, DMA_IT_TCIF3);
		
	}
}
//-----------------------------------------------------------------------------
void Start_spi(uint8_t number)
{
	DMA_SetCurrDataCounter(DMA2_Stream0, number);//установили количество считываемых байт
	DMA_SetCurrDataCounter(DMA2_Stream3, number);
	CS_ON();
	Spi_work = 1;
	DMA_Cmd(DMA2_Stream0, ENABLE);//запускаем работу DMA
	DMA_Cmd(DMA2_Stream3, ENABLE);
	
	
}
//-----------------------------------------------------------------------------
void SPIwait(void)
{
	while(Spi_work);
}
//-----------------------------------------------------------------------------
void Set_spi_out(uint8_t Adr, uint8_t data)
{
	if(Adr < 7)
	{
		spi_out[Adr] = data;
	}
}
//-----------------------------------------------------------------------------
uint8_t Get_spi_in(uint8_t Adr)
{
	return spi_in[Adr];
}


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
	dma.DMA_DIR = DMA_DIR_PeripheralToMemory;//направление передачи от переферии к памяти 
	dma.DMA_FIFOMode = DMA_FIFOMode_Disable;
	dma.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;// предельный уровень FIFO 
	dma.DMA_Memory0BaseAddr = (uint32_t)spi_in;//указываем адрес данных
	dma.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	dma.DMA_MemoryInc = DMA_MemoryInc_Enable;// инкрементируем адрес регистров памяти для передачи данных последовательно
	dma.DMA_Mode = DMA_Mode_Normal;
	dma.DMA_PeripheralBaseAddr =(uint32_t)&(SPI1->DR);// адрес регистра переферии (SPI)
	dma.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;// размер данных переферии
	dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;// увеличение указателя переферии (SPI1->DR)
	dma.DMA_Priority = DMA_Priority_Medium;
	
	DMA_Init(DMA2_Stream0, &dma);
	
	NVIC_EnableIRQ(DMA2_Stream0_IRQn);//включаем глобальные прерывания
	DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE);// включаем локальные прерывания DMA_IT_TC(Transmitted complete) для DMA2_Stream3 по окончанию передачи
	SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, ENABLE);// разрешаем работу DMA со SPI
}


