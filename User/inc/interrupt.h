#ifndef INTERRUPT_H_
#define INTERRUPT_H_


#include "stm32f4xx.h"                  // Device header



void init_delay(void);
void delay_ms(uint16_t delay_temp); // функция задержки реализованная посредством прерываний SysTick
void SysTick_Handler(void);// вектор прерывания


#endif /*INTERRUPT_H_*/
