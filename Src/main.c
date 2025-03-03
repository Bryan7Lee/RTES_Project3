/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Auto-generated by STM32CubeIDE
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>  // Include this to use atoi
#include <string.h>
#include <pthread.h>

#include "SysClock.h"
#include "demo.h"
#include "UART.h"

void GPIO_Init(void);
void TIM2_PWM_Init(void);
void UART_Init(void);
void init_systick();

// UART Ports:
// ===================================================
// PA.2 = USART2_TX (AF7)
// PA.3 = USART2_RX (AF7)

#define TX_PIN 2
#define RX_PIN 3

int main(void)
{
    // Initialize GPIO, Timer, and UART
	System_Clock_Init(); // set System Clock = 80 MHz
	GPIO_Init();
    TIM2_PWM_Init();
    UART2_Init();
    init_systick();

    run_demo();
}

void GPIO_Init(void)
{
    // Enable GPIOA clock
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

    // Set PA5 to Alternate Function mode
    GPIOA->MODER &= ~(GPIO_MODER_MODE5);       // Clear PA5 mode bits
    GPIOA->MODER |= GPIO_MODER_MODE5_1;        // Set PA5 to AF mode

    // Set PA1 to Alternate Function mode
	GPIOA->MODER &= ~(GPIO_MODER_MODE1);       // Clear PA1 mode bits
	GPIOA->MODER |= GPIO_MODER_MODE1_1;        // Set PA1 to AF mode

    // Select AF1 (TIM2_CH1) for PA5
    GPIOA->AFR[0] |= (1 << GPIO_AFRL_AFSEL5_Pos);  // Set AF1 (TIM2) for PA5

    // Select AF1 (TIM2_CH2) for PA1
    GPIOA->AFR[0] |= (1 << GPIO_AFRL_AFSEL1_Pos);  // Set AF1 (TIM2) for PA1

    // Set PA5 and PA1 to Push-pull, No pull-up/down, High-speed
    GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR5;
    GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR1;
}

void TIM2_PWM_Init(void)
{
    // Enable TIM2 clock
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;

    // Configure the timer for PWM
    TIM2->PSC = 7999;   // Prescaler value for 80 MHz / (7999 + 1) = 10 kHz
    TIM2->ARR = 199;    // Auto-reload value for 10 kHz / (199 + 1) = 50 Hz
    TIM2->CCR1 = 6;    // Initial duty cycle (adjust as necessary)
    TIM2->CCR2 = 6;

    // Set PWM mode 1 on TIM2 CH1 (active until match, inactive otherwise)
    TIM2->CCMR1 &= ~(TIM_CCMR1_OC1M); // Clear OC1M bits
    TIM2->CCMR1 |= (0x6 << TIM_CCMR1_OC1M_Pos); // Set PWM mode 1 (110)
    TIM2->CCMR1 |= TIM_CCMR1_OC1PE; // Enable preload register on CCR1

    // Set PWM mode 1 on TIM2 CH2 (active until match, inactive otherwise)
	TIM2->CCMR1 &= ~(TIM_CCMR1_OC2M); // Clear OC2M bits
	TIM2->CCMR1 |= (0x6 << TIM_CCMR1_OC2M_Pos); // Set PWM mode 1 (110)
	TIM2->CCMR1 |= TIM_CCMR1_OC2PE; // Enable preload register on CCR2

    // Enable capture/compare for channel 1
    TIM2->CCER |= TIM_CCER_CC1E; // Enable TIM2 CH1 output
    TIM2->CCER |= TIM_CCER_CC2E; // Enable TIM2 CH2 output

    // Enable the counter
    TIM2->CR1 |= TIM_CR1_CEN; // Enable timer
}

void init_systick()
{
	// Use the SysTick global structure pointer to do the following in this
	// exact order with separate lines for each step:
	//
	// Disable SysTick by clearing the CTRL (CSR) register.
	SysTick->CTRL = 0UL;

	// Set the LOAD (RVR) to 15,999 to give us a 1 millisecond timer
	// System clock is 16MHz
	SysTick->LOAD |= (SysTick_LOAD_RELOAD_Msk & 15999UL);

	// Set the clock source bit in the CTRL (CSR) to the internal clock.
	SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;

	// Enable the interrupt bit
	SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;

	// Set the enable bit in the CTRL (CSR) to start the timer.
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk ;
}


