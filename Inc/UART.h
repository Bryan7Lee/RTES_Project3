#ifndef __NUCLEO476_UART_H
#define __NUCLEO476_UART_H

#include "stm32l476xx.h"

#define BUFFER_SIZE (16)

void 	UART2_Init(void);
void 	UART2_GPIO_Init(void);
void 	USART_Init(USART_TypeDef * USARTx);
uint8_t USART_Read(USART_TypeDef * USARTx);
void 	USART_Write(USART_TypeDef * USARTx, uint8_t *buffer, uint32_t nBytes);
void 	USART_Delay(uint32_t us);
uint8_t USART_Read_NB (USART_TypeDef * USARTx);
void 	USART_IRQHandler(USART_TypeDef * USARTx, uint8_t *buffer, uint32_t * pRx_counter);

#endif /* __NUCLEO476_UART_H */
