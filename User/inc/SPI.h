#ifndef SPI_H_
#define SPI_H_
#include "stm32f4xx.h"  


#define SPI_MOSI GPIO_Pin_7
#define SPI_MISO GPIO_Pin_6
#define SPI_SCK GPIO_Pin_5
#define SPI_CS GPIO_Pin_3
#define CS_ON() GPIO_ResetBits(GPIOE, GPIO_Pin_3);
#define CS_OFF() GPIO_SetBits(GPIOE, GPIO_Pin_3);
#define EMPTY 0x00


void SPI_init(void);//������������� SPI
void SPI_DMA_init(void);//������������� DMA ��� SPI
void Start_spi(uint8_t number);//Start SPI
void SPIwait(void);//�������� ��������� ��������
void Set_spi_out(uint8_t Adr, uint8_t data);//���������� ������ � spi_out
uint8_t Get_spi_in(uint8_t Adr);//��������� ������ �� SPI

#endif
