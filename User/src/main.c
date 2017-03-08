#include "stm32f4xx.h"                  // Device header
#include "main.h"
#include "interrupt.h"
#include "SPI.h"
#include "LIS302DL.h"
#include "LCD_HD44780.h"
#include "usart.h"


uint8_t SPI_EN = 0;

uint8_t buffer[8];
uint8_t spi_in[7],spi_out[7];
uint8_t Spi_work = 0;//флаг работы SPI
int8_t buf[2];
char usart_buf[16];
char str[17];
int count = 0, usart_w = 0, usart_r = 0, flag_spi = 0;
uint8_t buf_rx[8], buf_tx[8];

//RX
void DMA2_Stream0_IRQHandler(void)
{
	uint16_t temp = 0;
	if(DMA_GetITStatus(DMA2_Stream0, DMA_IT_TCIF0))//если прием выполнен
	{
		DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TCIF0);//очищаем флаг приема
		CS_OFF();//выключаем чип селект
		Spi_work = 0;//SPI не работает выключаем флаг
		int32_t temp2 = 0;
		buf_tx[0] = 0xAA;
		buf_tx[1] = 0xBB;
		
		temp = (((uint16_t)spi_in[2])<<8)|((uint16_t)spi_in[1]);
/*		if(temp == 0x007F)
			temp = 0x0000;
		else 
		{			
			temp2 = 0x00FF - temp;
//		temp2 = temp2/14;
			temp = temp - temp2;
		}
		if(temp >= 0x8000)
		{
			temp -= 0x8000;
		}
		else
			temp += 0x8000;
*/		buf_tx[2] = (uint8_t)(temp>>8);//X
		buf_tx[3] = (uint8_t)temp;
		
		temp = (((uint16_t)spi_in[4])<<8)|((uint16_t)spi_in[3]);
/*		if(temp == 0x007F)
			temp = 0x0000;
		else 
		{			
			temp2 = 0x00FF - temp;
//		temp2 = temp2/14;
			temp = temp - temp2;
		}
		if(temp >= 0x8000)
		{
			temp -= 0x8000;
		}
		else
			temp += 0x8000;
*/		buf_tx[4] = (uint8_t)(temp>>8);//Y
		buf_tx[5] = (uint8_t)temp;
		
		temp = (((uint16_t)spi_in[6])<<8)|((uint16_t)spi_in[5]);
	/*	if(temp == 0x007F)
			temp = 0x0000;
		else 
		{			
			temp2 = 0x00FF - temp;
//		temp2 = temp2/14;
			temp = temp - temp2;
		}
		if(temp >= 0x8000)
		{
			temp -= 0x8000;
		}
		else
			temp += 0x8000;
*/		buf_tx[6] = (uint8_t)(temp>>8);//X
		buf_tx[7] = (uint8_t)temp;
	
//		flag_spi = 1;
		DMA_SetCurrDataCounter(DMA1_Stream3, 8);
		DMA_Cmd(DMA1_Stream3, ENABLE);
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

//------------------------------------------------------------

void USART3_IRQHandler(void) 
{
//	if(USART_GetITStatus(USART3, USART_IT_TXE))//если прерывание по передаче
//	{
//	USART_ClearITPendingBit(USART3, USART_IT_TXE);//очищаем флаг
//		usart_buf = USART_ReceiveData(USART3);		
/*		if(usart_buf[count]!= 0)
		{
			USART_SendData(USART3, usart_buf[count]);
			count++;
		}
		else 
				count = 0;
				USART_ITConfig(USART3, USART_IT_TXE, DISABLE);//выключаем прерывания по передаче
*/		
	if(USART_GetITStatus(USART3, USART_IT_RXNE))//если прерывание по приёму
	{
		USART_ClearITPendingBit(USART3, USART_IT_RXNE);//очищаем флаг

			usart_buf[usart_w] = USART_ReceiveData(USART3);
			if(count<16)
			{
				usart_w++;
				count++;
			}
			else 
			{
				usart_w = 0;
				count = 0;
			}	
	}

}
//--------------------------------------------------------------------------
//USART RX 
void DMA1_Stream1_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA1_Stream1, DMA_IT_TCIF1))
	{
		DMA_ClearITPendingBit(DMA1_Stream1, DMA_IT_TCIF1);
		for(int i = 0; i < 16; i++)
		{
			usart_buf[i] = buf_rx[i];

		}

	}
}
//--------------------------------------------------------------------------
//USART TX
void DMA1_Stream3_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA1_Stream3, DMA_IT_TCIF3))
	{
		DMA_ClearITPendingBit(DMA1_Stream1, DMA_IT_TCIF3);
		
	}
}
//--------------------------------------------------------------------------

int main()
{
	uint8_t spidata;
	uint8_t buf[2];
	init_delay();
//	init_perif();
	LCD_init_pin();
	LCD_init();
	LCD_write_str("HI SPI");
	delay_ms(400);//для того что бы успел проинициализироваться аксел
	SPI_init();
	SPI_DMA_init();
	usart_init();
	usart_dma_ini();
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
	LCD_write_str("Z");
	
	SPI_EN = 1;
	while(1)
	{
/*		spidata = buffer[0];
		sprintf(buf, "%u",spidata);
		kursor_adress(THIRD);
		LCD_write_str(buf);
		
		spidata = buffer[2];
		sprintf(buf, "%u",spidata);
		kursor_adress(THIRD_S);
		LCD_write_str(buf);	
		if(flag_spi)
		{
			for(int i = 0; i<3; i++){
				buf_tx[i] = buffer[i];
			}
			flag_spi = 0;
		}
		DMA_Cmd(DMA1_Stream3, ENABLE);
*/	}

	
/*	код без прерывания
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
