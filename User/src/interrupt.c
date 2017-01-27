
#include "interrupt.h"
#include "SPI.h"

uint16_t delay_count = 0;
extern uint8_t SPI_EN;

void init_delay(void)
{
	SysTick_Config(SystemCoreClock/1000);//инициализация SysTick запуск прерываний раз в 1/1000 сек.
}

//------------------------------------------------------------
void SysTick_Handler(void)// вектор прерывания
{
	if(delay_count>0)
		delay_count--;
	
	if(SPI_EN)// раз в 1мс запускае чтение данных со SPI
	{
		CS_ON();
		DMA_Cmd(DMA2_Stream0, ENABLE);//запускаем работу DMA
		DMA_Cmd(DMA2_Stream3, ENABLE);
	}
}
//------------------------------------------------------------
void delay_ms(uint16_t delay_temp) // функция задержки реализованная посредством прерываний SysTick
{
	delay_count = delay_temp;
	while(delay_count);
}
//------------------------------------------------------------




