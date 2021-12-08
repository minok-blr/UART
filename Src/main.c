/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Auto-generated by STM32CubeIDE
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

#include <stdint.h>
#include <string.h>
#include "../Inc/printf.h"
#include "../Inc/stm32f4xx.h"

#define RED_LED_PIN (1 << 14)
#define GREEN_LED_PIN (1 << 12)
#define USER_BUTTON_PIN (1 << 0)

volatile uint8_t buffer[6] = {'\0'};
volatile uint8_t bufferUART1[3] = {'\0'};
volatile uint8_t iter = 0;
volatile uint8_t iterShort = 0;

#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif

void USART_CONFIG()
{
	// enable gpio b and c clocks, and usart 1 and 6 clocks
	SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN); // enable A clock
	SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIODEN); // enable D clock
	SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOBEN); // enable B clock
	SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOCEN); // enable C clock
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_USART1EN);// USART1
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_USART6EN);// USART6

	// set PB6 TX and PC7 RX pins to alternate function mode
	SET_BIT(GPIOB->MODER, GPIO_MODER_MODER6_1);
	CLEAR_BIT(GPIOB->MODER, GPIO_MODER_MODER6_0);

	SET_BIT(GPIOC->MODER, GPIO_MODER_MODER7_1);
	CLEAR_BIT(GPIOC->MODER, GPIO_MODER_MODER7_0);

	// PB6, set AFRL6 to AF7(USART1) TX
	CLEAR_BIT(GPIOB->AFR[0], 1<<27);
	SET_BIT(GPIOB->AFR[0], 1<<26);
	SET_BIT(GPIOB->AFR[0], 1<<25);
	SET_BIT(GPIOB->AFR[0], 1<<24);

	// PC7, set AFRL7 to AF8(USART6) RX
	SET_BIT(GPIOC->AFR[0], 1<<31);
	CLEAR_BIT(GPIOC->AFR[0], 1<<30);
	CLEAR_BIT(GPIOC->AFR[0], 1<<29);
	CLEAR_BIT(GPIOC->AFR[0], 1<<28);

	// !================== TASK 4 ==================!

	// set PB7 (RX) pins to AFM
	SET_BIT(GPIOB->MODER, GPIO_MODER_MODER7_1);
	CLEAR_BIT(GPIOB->MODER, GPIO_MODER_MODER7_0);

	// set PC6 (TX) pins to AFM
	SET_BIT(GPIOC->MODER, GPIO_MODER_MODER6_1);
	CLEAR_BIT(GPIOC->MODER, GPIO_MODER_MODER6_0);

	// PC6, set AFRL6 to AF7(USART6) TX
	CLEAR_BIT(GPIOC->AFR[0], 1<<27);
	SET_BIT(GPIOC->AFR[0], 1<<26);
	SET_BIT(GPIOC->AFR[0], 1<<25);
	SET_BIT(GPIOC->AFR[0], 1<<24);

	// PB7, set AFRL7 to AF8(USART1) RX
	SET_BIT(GPIOB->AFR[0], 1<<31);
	CLEAR_BIT(GPIOB->AFR[0], 1<<30);
	CLEAR_BIT(GPIOB->AFR[0], 1<<29);
	CLEAR_BIT(GPIOB->AFR[0], 1<<28);

	// enable interrupts
	SET_BIT(USART1->CR1, USART_CR1_RXNEIE);
	SET_BIT(USART6->CR1, USART_CR1_RXNEIE);
	NVIC_EnableIRQ(37);
	NVIC_EnableIRQ(71);

	// !================== TASK 4 ==================!

	// UART CONFIG PB6 TX and PB7 RX
	SET_BIT(USART1->CR1, USART_CR1_UE); 	// 1. enable UE bit
	CLEAR_BIT(USART1->CR1, USART_CR1_M);  	// 2. set M bit for word len, in this case 8 bits
	CLEAR_BIT(USART1->CR1, USART_CR1_OVER8);// 3. oversampling by 16
	USART1->BRR = (11<<0) | (8<<4);			// 4. baud rate 8.6875, DIV_MANTISSA = 0x8, DIV_FRAC = 0xB 0b1011
	SET_BIT(USART1->CR1, USART_CR1_TE); 	// 5. set TE in USART_CR1 to send idle frame as 1st transmission
	SET_BIT(USART1->CR1, USART_CR1_RE); 	// 5. set RE in USART_CR1

	// 6. write to USART_DR(clears the TXE bit) and repeat for the whole buffer
	// 7. wait until TC == 1, when USART disabled/enters halt mode

	// UART CONFIG PC7 RX and PC6 TX
	SET_BIT(USART6->CR1, USART_CR1_UE); 	// 1. enable UE bit
	CLEAR_BIT(USART6->CR1, USART_CR1_M);  	// 2. set M bit for word len, in this case 8 bits
	CLEAR_BIT(USART6->CR1, USART_CR1_OVER8);// 3. oversampling by 16
	USART6->BRR = (11<<0) | (8<<4);			// 4. baud rate 8.6875, DIV_MANTISSA = 0x8, DIV_FRAC = 0xB 0b1011
	SET_BIT(USART6->CR1, USART_CR1_RE); 	// 5. set RE in USART_CR1
	SET_BIT(USART6->CR1, USART_CR1_TE); 	// 5. set TE in USART_CR1 to send idle frame as 1st transmission

}

void USART1_IRQHandler(void)
{
	bufferUART1[iterShort] = USART1->DR;
}

void USART6_IRQHandler(void)
{
	buffer[iter] = USART6->DR;
}

void sendChar(uint8_t letter, uint8_t iter)
{
	WRITE_REG(USART1->DR, letter);
	while(!(USART1->SR & (1<<6))); // wait for transmission to happen
}

void sendChar6(uint8_t letter, uint8_t iter)
{
	WRITE_REG(USART6->DR, letter);
	while(!(USART6->SR & (1<<6))); // wait for transmission to happen
}

void USART6_SendData(uint8_t choice){
	char *word = (char*)'0';
	if(choice == 1) word = "BUTTON";
	else word = "WRONG";

	if(choice == 10)
	{
		word="OK!";
	}
	if(choice == 11)
	{
		word = "NOT";
	}
	while(*word) {
		sendChar6(*word, iterShort);
		*(word++);
		iterShort++;
	}

}

void USART1_SendData(uint8_t choice){
	//uint8_t iter = 0;
	char *word = (char*)'0';
	if(choice == 1) word = "BUTTON";
	else word = "WRONG";

	if(choice == 10)
	{
		word="OK!";
	}
	if(choice == 11)
	{
		word = "NOT";
	}
	while(*word) {
		sendChar(*word, iter);
		*(word++);
		iter++;
	}

}

//void USART6_ReceiveData(){
//	uint8_t letter;
//	while(!(USART6->SR & (1<<5)));
//	letter = READ_REG(USART6->DR);
//	printf("%c\n", letter);
//}

int main(void)
{
	USART_CONFIG();

	// set PA0 to input
	CLEAR_BIT(GPIOA->MODER, GPIO_MODER_MODER0_1);
	CLEAR_BIT(GPIOA->MODER, GPIO_MODER_MODER0_0);

	// RED_LED PD14
	SET_BIT(GPIOD->MODER, GPIO_MODER_MODER14_0);
	CLEAR_BIT(GPIOD->MODER, GPIO_MODER_MODER14_1);

	// GREEN_LED PD12
	SET_BIT(GPIOD->MODER, GPIO_MODER_MODER12_0);
	CLEAR_BIT(GPIOD->MODER, GPIO_MODER_MODER12_1);

	uint8_t confidence = 0;
	uint8_t threshold = 15;
	uint8_t buttonSwitch = 0;
	uint8_t prevButtonState = 0;
	int counter = 0;

    /* Loop forever */
	for (;;) {
		int32_t buttonState = GPIOA->IDR & USER_BUTTON_PIN;

		if (confidence > threshold && buttonState == 0) buttonSwitch = 1;
		if (buttonState != 0 && prevButtonState != 0) confidence++;
		else confidence = 0;

		if (buttonSwitch == 1)
		{
			counter++;
			USART1_SendData(counter%2);

			//check what is received
			uint8_t strength = 0;
			char* word = "BUTTON";
			for (int i = 0; i < 6; i++) {
				if(buffer[i] == word[i]) strength++;
			}
			if(strength == 6)
			{
				// if "TIPKA" is received
				// turn on green light, wait a bit and turn it off
				char *wordOK = "OK!";
				USART6_SendData(10);

				strength = 0;
				for (int i = 0; i < 3; i++) {
					if(bufferUART1[i] == wordOK[i]) strength++;
				}
				if (strength == 3) {
					GPIOD->ODR |= GREEN_LED_PIN;
					for (int var = 0; var < 1000000; ++var) asm("nop");
					GPIOD->ODR &= ~GREEN_LED_PIN;
				}


				strength = 0;
				iter = 0;
				iterShort = 0;
			}
			else
			{
				strength = 0;
				char *wordNOT = "NOT";
				USART6_SendData(11);

				for (int i = 0; i < 3; i++) {
					if(bufferUART1[i] == wordNOT[i]) strength++;
				}
				if (strength == 3) {
					GPIOD->ODR |= RED_LED_PIN;
					for (int var = 0; var < 1000000; ++var) asm("nop");
					GPIOD->ODR &= ~RED_LED_PIN;
				}

				strength = 0;
				iter = 0;
				iterShort = 0;
			}
			confidence = 0;
			buttonSwitch = 0;
			prevButtonState = 0;
		}

		prevButtonState = buttonState;
		if(counter == 99) counter = 0;
	}
}
