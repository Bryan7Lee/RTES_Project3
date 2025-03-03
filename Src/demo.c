#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>  // Include this to use atoi
#include <string.h>
#include <pthread.h>

#include "demo.h"
#include "stm32l476xx.h"
#include "UART.h"

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
//uint32_t recipe[15] = {
//		MOV+1,
//		WAIT+10,
//		MOV+5,
//		WAIT+10,
//		MOV+0,
//		RECIPE_END,
//		RECIPE_END,
//		RECIPE_END,
//		RECIPE_END,
//		RECIPE_END,
//		RECIPE_END,
//		RECIPE_END,
//		RECIPE_END,
//		RECIPE_END,
//		RECIPE_END
//};
uint32_t loop_recipe[15] = {0};

// array for both servos
servo servos[2];

// ints
uint32_t five_bits = 0;
uint32_t opcode = 0;
uint32_t counter = 0;
uint32_t moving_counter = 0;

// booleans
_Bool servo_moved = false;
_Bool step_done = false;

// buffers
static uint8_t buffer[200];
static char input_buffer[100]="This is garbage.\n";


//******************************************************************************************
// This function is to handle SysTick Timer
//******************************************************************************************
void SysTick_Handler(void)
{
	counter++; // tick counter for three_seconds_elapsed variable for auto mode counting
	moving_counter++;

	if (counter > 1600){
		step_done = true;
		counter = 0;
	}
	if (moving_counter > 2500){
		servo_moved = true;
		moving_counter = 0;
	}
}

int printf (const char *format, ...) {
    va_list aptr;
    int ret;

    __builtin_va_start(aptr, format);
    ret = vsprintf ((char*)buffer, format, aptr);
    __builtin_va_end(aptr);

    USART_Write (USART2, buffer, ret);

    return ret;
}

void perform_recipe_step(servo *serv){
	if (serv -> inLoop){	// only enter when servo is in loop command
		five_bits = (loop_recipe[serv -> loopStep] & 0x1F);		// get last 5 bits of command
		opcode = (loop_recipe[serv -> loopStep] & 0xE0);		// get opcode
		if (opcode == MOV){
			moving_counter = 0;
			if (serv -> id == 1){
				TIM2->CCR1 = 6 + (4 * five_bits);
			}
			else if (serv -> id == 2){
				TIM2->CCR2 = 6 + (4 * five_bits);
			}
			serv -> loopStep += 1;
			serv -> position = five_bits;
			while(!servo_moved){};
			servo_moved = false;
		}
		else if (opcode == WAIT){
			if (serv -> waitStep < five_bits){
				serv -> waitStep += 1;
			}
			else {
				serv -> waitStep = 1;
				serv -> loopStep += 1;
			}
		}

		if (serv -> loopRecipeLen == serv -> loopStep){
			serv -> loopStep = 0;
			serv -> loopCount++;
			if (serv -> loopCount == serv -> loopIters){
				serv -> loopCount = 0;
				serv -> inLoop = false;
				serv -> recipeStep += 1;
			}
		}
	}
	else {		// if servo is not in a loop command
		five_bits = (recipe[serv -> recipeStep] & 0x1F);		// get last 5 bits of command
		opcode = (recipe[serv -> recipeStep] & 0xE0);			// get opcode
		if (opcode == MOV){
			moving_counter = 0;
			if (serv -> id == 1){
				TIM2->CCR1 = 6 + (4 * five_bits);
			}
			else if (serv -> id == 2){
				TIM2->CCR2 = 6 + (4 * five_bits);
			}
			serv -> recipeStep += 1;
			serv -> position = five_bits;
			while(!servo_moved){};
			servo_moved = false;
		}
		else if (opcode == WAIT){
			if (serv -> waitStep < five_bits){
				serv -> waitStep += 1;
			}
			else {
				serv -> waitStep = 1;
				serv -> recipeStep += 1;
			}
		}
		else if (opcode == LOOP){
			if (five_bits == 0){
				serv -> recipeStep += 1;
			}
			else {
				serv -> loopIters = five_bits;
				serv -> inLoop = true;
				serv -> loopStep = 0;
				serv -> loopCount = 0;
				uint32_t loop_check = (serv -> recipeStep + 1);
				uint32_t loop_index = 0;
				memset(loop_recipe, 0, 15*sizeof(uint32_t));
				// populate loop recipe
				while (1) {
					if ((recipe[loop_check] & 0xE0) == END_LOOP){
						serv -> loopRecipeLen = loop_index;
						break;
					}
					else {
						loop_recipe[loop_index] = recipe[loop_check];
						loop_check++;
						loop_index++;
					}
				}
			}
		}
		else if (opcode == END_LOOP){
			serv -> recipeStep += 1;
		}
		else if (opcode == RECIPE_END){
			serv -> recipeStep = 0;
			serv -> recState = ended;
			printf("Recipe completed\r\n\r\n");
		}
		else{
			serv -> recipeStep = 0;
			serv -> recState = error;
			printf("Error in recipe!!\r\n\r\n");
		}
	}


	return;
}




void run_demo(){
	//pthread_create(NULL, NULL, nBGet, NULL);
	char one_char;
	int num_entered = 0;
	char commands[2] = {'x', 'x'};

	for (int i = 0; i < 2; i++){
		servos[i].id = i+1;
		servos[i].recipeStep = 0;
		servos[i].recState = ended;
		servos[i].loopIters = 0;
		servos[i].loopStep = 0;
		servos[i].loopRecipeLen = 0;
		servos[i].inLoop = false;
		servos[i].waitStep = 1;
		servos[i].position = 0;
	}

	printf("\r\n%s\r\n", "Valid commands: p, c, r, l, n, s");
	printf("> ");
	uint32_t input_index = 0;
	memset(input_buffer, 0, 100);

	while (1)
	{
		// get input
		one_char = USART_Read_NB(USART2);
		if(one_char == 0x0D) // Enter Key
		{
			printf("\r\n%s\r\n", "\0");
			printf("> ");
			num_entered = 0;
			commands[0] = input_buffer[input_index-2];
			commands[1] = input_buffer[input_index-1];
			memset(input_buffer, 0, 100);
			input_index = 0;
		}
		else if((one_char != '\0') && (num_entered < 2))
		{
			if (one_char == 'p' || one_char == 'P' || 	\
				one_char == 'c' || one_char == 'C' ||	\
				one_char == 'r' || one_char == 'R' || 	\
				one_char == 'l' || one_char == 'L' || 	\
				one_char == 'n' || one_char == 'N' || 	\
				one_char == 's' || one_char == 'S')
			{
				input_buffer[input_index++] = one_char;
				printf("\r> %s", input_buffer);
				num_entered++;
			}
		}

		// perform recipe step for each servo
		for (int i = 0; i < 2; i++){
			switch(servos[i].recState){
			case running:
				if (commands[i] == 'p' || commands[i] == 'P'){
					servos[i].recState = paused;
					commands[i] = 'x';
					break;
				}
				else if (commands[i] == 's' || commands[i] == 'S'){
					servos[i].recipeStep = 0;
					servos[i].loopCount = 0;
					servos[i].loopIters = 0;
					servos[i].loopStep = 0;
					servos[i].loopRecipeLen = 0;
					servos[i].inLoop = false;
					servos[i].waitStep = 1;
					servos[i].position = 0;
				}
				perform_recipe_step(&servos[i]);
				commands[i] = 'x';
				break;
			case paused:
				if (commands[i] == 'c' || commands[i] == 'C'){
					servos[i].recState = running;
					perform_recipe_step(&servos[i]);
					commands[i] = 'x';
					break;
				}
				else if (commands[i] == 's' || commands[i] == 'S'){
					servos[i].recipeStep = 0;
					servos[i].recState = running;
					servos[i].loopCount = 0;
					servos[i].loopIters = 0;
					servos[i].loopStep = 0;
					servos[i].loopRecipeLen = 0;
					servos[i].inLoop = false;
					servos[i].waitStep = 1;
					servos[i].position = 0;
					perform_recipe_step(&servos[i]);
					commands[i] = 'x';
					break;
				}
				else if (commands[i] == 'r' || commands[i] == 'R'){
					if (servos[i].position != 0){
						servos[i].position -= 1;
						if (servos[i].id == 1){
							TIM2->CCR1 = 6 + (4 * servos[i].position);
						}
						else {
							TIM2->CCR2 = 6 + (4 * servos[i].position);
						}
					}
					commands[i] = 'x';
					break;
				}
				else if (commands[i] == 'l' || commands[i] == 'L'){
					if (servos[i].position != 5){
						servos[i].position += 1;
						if (servos[i].id == 1){
							TIM2->CCR1 = 6 + (4 * servos[i].position);
						}
						else {
							TIM2->CCR2 = 6 + (4 * servos[i].position);
						}
					}
					commands[i] = 'x';
					break;
				}
				commands[i] = 'x';
				break;
			case ended:
				if (commands[i] == 's' || commands[i] == 'S'){
					servos[i].recipeStep = 0;
					servos[i].recState = running;
					servos[i].loopCount = 0;
					servos[i].loopIters = 0;
					servos[i].loopStep = 0;
					servos[i].loopRecipeLen = 0;
					servos[i].inLoop = false;
					servos[i].waitStep = 1;
					servos[i].position = 0;
					perform_recipe_step(&servos[i]);
					commands[i] = 'x';
					break;
				}
				commands[i] = 'x';
				break;
			case error:
				if (commands[i] == 's' || commands[i] == 'S'){
					servos[i].recipeStep = 0;
					servos[i].recState = running;
					servos[i].loopCount = 0;
					servos[i].loopIters = 0;
					servos[i].loopStep = 0;
					servos[i].loopRecipeLen = 0;
					servos[i].inLoop = false;
					servos[i].waitStep = 1;
					servos[i].position = 0;
					perform_recipe_step(&servos[i]);
					commands[i] = 'x';
					break;
				}
				commands[i] = 'x';
				break;
			}

		}
		while(!step_done){};	//wait at end until 300ms passed total
		counter = 0;
		step_done = false;
	}
}



