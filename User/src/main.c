#include "stm32f4xx.h"                  // Device header
#include "main.h"
#include "interrupt.h"
#include "SPI.h"
#include "LIS302DL.h"
#include "LCD_HD44780.h"



uint8_t SPI_EN = 0;

uint8_t buffer[10];


//------------------------------------------------------------
int main()
{
	uint8_t spidata;
	uint8_t buf[2];
	init_delay();
	init_perif();
	LCD_init_pin();
	LCD_init();
	LCD_write_str("HI SPI");
	delay_ms(400);//для того что бы успел проинициализироваться аксел
	SPI_init();
	SPI_DMA_init();
	
	Set_spi_out(0, READ_SPI|ID);//запрос ID
	Set_spi_out(1, EMPTY);//передаем пустые данные для приема
	Start_spi(2);// запуск DMA для передачи 2х байт
	SPIwait();
	
	spidata = Get_spi_in(1);
	sprintf(buf, "%u",spidata);
	kursor_adress(SEC_LINE);
	LCD_write_str(buf);
	
	Set_spi_out(0, 0x20);//set sample rate
	Set_spi_out(1, 0x77);//400Hz
	Start_spi(2);// запуск DMA для передачи 2х байт
	SPIwait();
	
	Set_spi_out(0, READ_MS|OUT_X);//запрашиваем данные по оси X
	Set_spi_out(1, EMPTY);//
	DMA_SetCurrDataCounter(DMA2_Stream0, 7);//установили количество считываемых байт
	DMA_SetCurrDataCounter(DMA2_Stream3, 7);
	clean_display();
	kursor_adress(FIRS_LINE);
	LCD_write_str("X");
	kursor_adress(SEC_LINE);
	LCD_write_str("Y");
	
	SPI_EN = 1;
	while(1)
	{
		spidata = buffer[0];
		sprintf(buf, "%u",spidata);
		kursor_adress(THIRD);
		LCD_write_str(buf);
		
		spidata = buffer[1];
		sprintf(buf, "%u",spidata);
		kursor_adress(THIRD_S);
		LCD_write_str(buf);	
	}

	
/*	
	CS_ON();
	SPI_I2S_SendData(SPI1, READ_SPI|ID);//отправляем адрес регистра старший бит RW = 1, читаем из; бит MS = 0, читаем только один байт
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY));//ждем освобождения шины
	spidata = SPI_I2S_ReceiveData(SPI1);
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY));//ждем освобождения шины
	SPI_I2S_SendData(SPI1, EMPTY);//отправляем нули для сдвига регистра
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY));//ждем освобождения шины
	spidata = SPI_I2S_ReceiveData(SPI1);//читаем данные
	CS_OFF();
	
	if(spidata == WHO_AM_I)
	{
		GPIO_SetBits(GPIOD, GREEN);
	}
	else 
		GPIO_SetBits(GPIOD, RED);
	sprintf(buf, "%u",spidata);
	kursor_adress(SEC_LINE);
//	LCD_write(spidata, RS_DATA);
	LCD_write_str(buf);
	CS_ON();
	while(1)
	{

		SPI_I2S_SendData(SPI1, (OUT_Z|READ_SPI));//отправляем адрес регистра старший бит RW = 1, читаем из; бит MS = 0, читаем только один байт
		while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY));//ждем освобождения шины
		spidata = SPI_I2S_ReceiveData(SPI1);
		while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY));//ждем освобождения шины
		SPI_I2S_SendData(SPI1, EMPTY);//отправляем нули для сдвига регистра
		while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY));//ждем освобождения шины
		spidata = SPI_I2S_ReceiveData(SPI1);//читаем данные

		sprintf(buf, "%u",spidata);
		kursor_adress(THIRD_S);
		LCD_write_str(buf);
//		spidata++;
//		LCD_write(spidata, RS_DATA);
	
	}
	
*/
}
