/*
 * demo.h
 *
 *  Created on: Feb 25, 2025
 *      Author: danlaudico
 */

#ifndef INC_DEMO_H_
#define INC_DEMO_H_

#define RECIPE_END (0)
#define MOV (32)
#define WAIT (64)
#define LOOP (128)
#define END_LOOP (160)

enum recipeState{
	running,
	paused,
	ended,
	error
};

typedef struct{
	int id;
	int recipeStep;
	enum recipeState recState;
	int loopCount;
	int loopIters;
	int loopStep;
	int loopRecipeLen;
	_Bool inLoop;
	int waitStep;
	int position;
} servo;

char UART_ReceiveChar(void);
void UART_Write(char *str);
void UART_SendChar(char c);
void UART_SendString(char* str);
void UART_Delay(uint32_t us);
void run_demo(void);
servo run_recipe(uint32_t recipe[15]);
void SysTick_Handler(void);
int printf (const char *format, ...);
void perform_recipe_step(servo *serv);
void run_demo(void);



#endif /* INC_DEMO_H_ */
