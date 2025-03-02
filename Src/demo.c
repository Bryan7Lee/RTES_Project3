#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>  // Include this to use atoi
#include <string.h>
#include <pthread.h>

#include "demo.h"
#include "UART.h"
#include "stm32l476xx.h"

uint32_t recipe[15] = {
		MOV+0,
		MOV+5,
		MOV+0,
		MOV+3,
		LOOP+3,
		MOV+1,
		MOV+4,
		END_LOOP,
		MOV+0,
		MOV+2,
		WAIT+1,
		MOV+3,
		WAIT+23,
		MOV+5,
		RECIPE_END
};

servo servos[2];

char input[100] = "N/A";
char userCommandInput[USER_COMMAND_INPUT_LENGTH] = "NN"; // array should only need 4 entries; example. NN<CR><\0>

// ints
uint32_t five_bits = 0;
uint32_t opcode = 0;
uint32_t counter = 0; // systick timer
uint32_t timer_val = 0;
uint32_t servoOneIndex = 0; // recipe index for servo 1
uint32_t servoTwoIndex = 0; // recipe index for servo 2

// booleans
_Bool timer_done = false;
_Bool servo_moved = false;
_Bool in_mov = false;
_Bool in_wait = false;
_Bool new_input = false;

servo run_recipe(uint32_t recipe[15])
{
	for (int i = 0; i < 15; i++){
		five_bits = (recipe[i] & 0x1F);		// get last 5 bits of command
		opcode = (recipe[i] & 0xE0);		// get opcode
		if (opcode == MOV){					// handle MOV command
			counter = 0;
			in_mov = true;
			TIM2->CCR1 = 6 + (4 * five_bits);
			while(!servo_moved);
			servo_moved = false;
			in_mov = false;
		}
		else if (opcode == WAIT){			// handle WAIT command
			timer_val = five_bits * 1600;
			counter = 0;
			in_wait = true;
			while(!timer_done); 			// wait for specified amount of time
			timer_done = false;
			in_wait = false;
		}
		else if (opcode == LOOP){			// handle LOOP command
			if (five_bits != 0){
				uint32_t loop[20] = {0};
				uint32_t loop_check = i+1;
				uint32_t loop_index = 0;
				// populate LOOP
				while (1) {
					if ((recipe[loop_check] & 0xE0) == END_LOOP){
						break;
					}
					else {
						loop[loop_index] = recipe[loop_check];
						loop_check++;
						loop_index++;
					}
				}
				// perform LOOP
				for (int j = 0; j < five_bits; j++){
					run_recipe(loop);
				}
			}
		}
		else if (opcode == END_LOOP){		// handle loop end
		}
		else if (opcode == RECIPE_END){		// handle recipe end
			//return 1;
		}
		else{								// handle illegal opcode
			//return 0;
		}
	}
	//return 1;
}

void run_demo()
{
	// variables
	char inputChar; // one character read from UART to add to input buffer array
	uint32_t input_index = 0; // ASDH

	// fresh start
	memset(userCommandInput, '\0', USER_COMMAND_INPUT_LENGTH);

	// infinite non-blocking loop
	while (1) {
		inputChar = USART_Read_NB(USART2);
		if (inputChar != 0x0D) // if <carriage return> was NOT entered
		{
			userCommandInput[input_index++] = inputChar;
			printf("\r%s", userCommandInput);
		}


	}
}

//uint32_t do_command(uint32_t command){
//	five_bits = (command & 0x1F);		// get last 5 bits of command
//	opcode = (command & 0xE0);		// get opcode
//	if (opcode == MOV){					// handle MOV command
//		counter = 0;
//		in_mov = true;
//		TIM2->CCR1 = 6 + (4 * five_bits);
//		while(!servo_moved);
//		servo_moved = false;
//		in_mov = false;
//	}
//	else if (opcode == WAIT){			// handle WAIT command
//		timer_val = five_bits * 1600;
//		counter = 0;
//		in_wait = true;
//		while(!timer_done); 			// wait for specified amount of time
//		timer_done = false;
//		in_wait = false;
//	}
//	else if (opcode == LOOP){			// handle LOOP command
//		if (five_bits != 0){
//			uint32_t loop[20] = {0};
//			uint32_t loop_check = i+1;
//			uint32_t loop_index = 0;
//			// populate LOOP
//			while (1) {
//				if ((recipe[loop_check] & 0xE0) == END_LOOP){
//					break;
//				}
//				else {
//					loop[loop_index] = recipe[loop_check];
//					loop_check++;
//					loop_index++;
//				}
//			}
//			// perform LOOP
//			for (int j = 0; j < five_bits; j++){
//				run_recipe(loop);
//			}
//		}
//	}
//	else if (opcode == END_LOOP){		// handle loop end
//	}
//	else if (opcode == RECIPE_END){		// handle recipe end
//		return 1;
//	}
//	else{								// handle illegal opcode
//		return 0;
//	}
//}

//******************************************************************************************
// This function is to handle SysTick Timer
//******************************************************************************************
void SysTick_Handler(void)
{
	counter++; // tick counter for three_seconds_elapsed variable for auto mode counting

	if (in_wait && (counter > timer_val))
	{
		timer_done = true;
		counter = 0;
	}
	if (in_mov && (counter > 2000))
	{
		servo_moved = true;
		counter = 0;
	}
}

	//valid_run = run_recipe(recipe);
	//    if (!valid_run){
	//    	UART_SendString("RECIPE ERROR: INVALID OPCODE FOUND\r\n");
	//    }
	//    else{
	//    	UART_SendString("Recipe completed successfully\r\n");
	//    }

//	char buffer[10];
//	int pulse_width = 26;  // Default 1ms pulse width
//
//	while (1) {
//		// Prompt user for new pulse width
//
//		UART_SendString("Enter a command (valid options are P, C, R, L, N, S):");
//
//	    int i = 0;
//	    char c;
//
//	    // Read input from UART
//	    while ((c = UART_ReceiveChar()) != '\r') {
//	        buffer[i++] = c;
//	        UART_SendChar(c);  // Echo back the character
//	    }
//	    buffer[i] = '\0';  // Null-terminate the string
//	    UART_SendString("\n\r");
//
//	    // Convert input to integer
//	    pulse_width = atoi(buffer);
//
//	    // Validate pulse width (ensure it's between 10 and 200 microseconds)
//	    if (pulse_width < 1 || pulse_width > 30) {
//	        UART_SendString("Invalid input! Please enter a value between 4 and 20.\n\r");
//	    } else {
//	            // Adjust PWM duty cycle based on the user input
//	        TIM2->CCR1 = pulse_width;
//	        UART_SendString("Pulse width updated!\n\r");
//	    }
//
//	}




