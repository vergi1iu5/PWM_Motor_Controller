/**
 * @file main.c
 * @author Ivan Ramos <iz0006@auburn.edu>
 * @version 1.3
 * @brief Source file to implement all functionality found in main.h
 * @section DESCRIPTION
 *
 * This source file implements all the logic to prvide the dunctionality
 * outline in the header file. This file is for the STM32L100 family. 
 * Port A1 is used as the input for the keypad, Port B is used to set-up
 * the active-high state needed for the keypad, and Port C as the output
 * for the LEDs. The LEDs display the last pressed key. Finally, timer
 * TM10 is used to provide the PWM signal needed to control the DC-motor.
 */
#include "main.h"
#include "stm32l1xx.h"

int main(){
	//Setup all pins needed to operate the keypad and DC motor
	pinSetup();
	//Initialize TIM10 to function as a 1kHz PWM signal out
	TIM10_PWM_setup(1000);
	//Enable all interrupts needed and clear display variables
	enableInterrupts();
	buttonPressed = 0;
	LEDS = 0xFF;
	//Display initial state of LEDs (all ON)
	updateLEDs(LEDS);
	
	while(1){
		//Wait for any button to be pressed
		if(buttonPressed){
			//Take in the value pressed and determine state for LEDs
			LEDS = desipherButton(buttonPressed);
			updateLEDs(LEDS);
			//If button pressed is between 0 - A, then update PWM signal duty cycle
			if(LEDS <= 10){PWM10_update(LEDS);}
			//Clear all display variables and allow for show delay (de-bouncing)
			buttonPressed = 0;
			LEDS = 0x00FF;
			delay();
		}
	}
}
/*!
 * Initialize all pins needed to operate Keypad and DC motor PWM signal. This module
 * first eneables all clocks needed to configure the GPIO ports. It then sets PA1 as 
 * an input port (used to read when any key is pressed), PC as output (display LEDs), 
 * and part of PB as output (pull-up for keypad) and the other part as input (read keypad)
 *
 * @return None.
 */
void pinSetup(){
	//Enable clocks used to configure GPIO ports A,B, and C
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN;
	
	//Set PA1 as input with pull down resistor
	GPIOA->MODER &= ~(GPIO_MODER_MODER1);
	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR1);
	GPIOA->PUPDR |= (GPIO_PUPDR_PUPDR1 & GPIO_PULLDOWN);
	//Set PC7-0 as output and to LOW state
	GPIOC->MODER &= ~(
		GPIO_MODER_MODER7 |
		GPIO_MODER_MODER6 |
		GPIO_MODER_MODER5 |
		GPIO_MODER_MODER4 |
		GPIO_MODER_MODER3 |
		GPIO_MODER_MODER2 |
		GPIO_MODER_MODER1 |
		GPIO_MODER_MODER0);
	GPIOC->MODER |= (
		GPIO_MODER_MODER7_0 |
		GPIO_MODER_MODER6_0 |
		GPIO_MODER_MODER5_0 |
		GPIO_MODER_MODER4_0 |
		GPIO_MODER_MODER3_0 |
		GPIO_MODER_MODER2_0 |
		GPIO_MODER_MODER1_0	|
		GPIO_MODER_MODER0_0);
		
	GPIOC->ODR &= ~(
		GPIO_ODR_ODR_7 |
		GPIO_ODR_ODR_6 |
		GPIO_ODR_ODR_5 |
		GPIO_ODR_ODR_4 |
		GPIO_ODR_ODR_3 |
		GPIO_ODR_ODR_2 |
		GPIO_ODR_ODR_1 |
		GPIO_ODR_ODR_0);
	//Setup PB3-0 as output and 7-4 as input
	setupKeypadPins(PB3_0_OUTPUT);
}
/*!
 * Eable all interrupts needed to operate full system. It eneables
 * external interrupt vector 1 with a falling edge and internal
 * pull-up resitors. This is to detect when a key is pressed and 
 * pulls PA1 low.
 *
 * @return None.
 */
void enableInterrupts(){
	//Configure EXT1 to take in interrupts from PA
	SYSCFG->EXTICR[0] &= ~(SYSCFG_EXTICR1_EXTI1);
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI1_PA;
	//Configure EXTI1 to trigger on a falling edge
	EXTI->RTSR &= ~(EXTI_RTSR_TR1);
	EXTI->FTSR |= EXTI_FTSR_TR1;
	//Configure EXTI1 to have internal pull-up resistors
	EXTI->IMR |= EXTI_IMR_MR1;
	EXTI->PR |= EXTI_PR_PR1;
	//Enable NVIC interrepts and clear any pendin interrupts
	NVIC_EnableIRQ(EXTI1_IRQn);
	NVIC_ClearPendingIRQ(EXTI1_IRQn);
	//Enable global interrupts
	__enable_irq();
}
/*!
 * Setup all the pins required to operate the Keypad. Pin Mask provided will
 * be used indicate which part of PB is to serve as input and which part to 
 * serve as output.
 *
 * @param outputPinsMask pins in PB to serve as output. Rest will serve as input.
 * @return None.
 */
void setupKeypadPins(uint32_t outputPinsMask){
	//Determine pins to be used as input
	uint32_t inputPinsMask = ~((0xFFFF0000)|(outputPinsMask));
	//Setput pins to be used as output and to state LOW
	GPIOB->MODER &= ~(outputPinsMask);
	GPIOB->MODER |= (outputPinsMask & GPIO_OUTPUT);
	GPIOB->ODR &= ~(outputPinsMask);
	//Set the rest of the pins as input with internal pull-up resistors
	GPIOB->MODER &= ~(inputPinsMask);
	GPIOB->MODER |= (inputPinsMask & GPIO_INPUT);	
	GPIOB->PUPDR &= ~(inputPinsMask);
	GPIOB->PUPDR |= (inputPinsMask & GPIO_PULLUP);
}
/*!
 * Update state of PC to show the provided LED state
 *
 * @param display LED state to be displayed.
 * @return None.
 */
void updateLEDs(uint16_t display){
	
	GPIOC->ODR &= (0xFFFFFF00);
	GPIOC->ODR |= display;
	
}
/*!
 * Take in the row and column pressed in the keypad and return the exact 
 * key pressed. Keypad is layed out as follows:
 * 
 *                           1 2 3 A
 *                           4 5 6 B
 *                           7 8 9 C
 *                           * 0 # F
 *
 * @param code 16-bit number where the MSB contains the row pressed as comming from 
 * keypad. (i.e. if row 3 pressed, then MSB shows 1011)
 * @return None.
 */
uint16_t desipherButton(uint16_t code){
	//Extract row portion and turn to active-high indication of row pressed
	rows = (0x00F0)&(code);
	rows = rows >> 4;
	rows = ~rows;
	//Exctact column portion and turn to active-high indication of column pressed
	colums = (0x000F)&(code);
	colums = ~colums;
	
	output = 0;
	
	if (colums & BIT3){output = 1;} //first column
	else if (colums & BIT2){output = 2;} //second column
	else if (colums & BIT1){output = 3;} //third column
	else if (colums & BIT0){ //fourth column, add one as row increases and return
		output = 10;
		if (rows & BIT3){output += 0;}
		else if (rows & BIT2){output += 1;}
		else if (rows & BIT1){output += 2;}
		else if (rows & BIT0){output += 3;}
		return output;
	}
	//Columns 1 - 3 pressed, so add multiples of 3 depending on row pressed
	if (rows & BIT3){output += 0;} //First row pressed
	else if (rows & BIT2){output += 3;} //Second row pressed
	else if (rows & BIT1){output += 6;} //third row pressed
	else if (rows & BIT0){ //Last row pressed, column now determines the value (either *, 0, or #)
		else if (colums & BIT3){output = 14;}
		else if (colums & BIT2){output = 0;}
		else if (colums & BIT1){output = 15;}
	}
	
	return output;
	
}
/*!
 * Initialize all hardware and variables required to operate TIM10 as 
 * a PWM signal output. This function takes in the desired period and 
 * calculates all values for a 0 - 100% duty cycle signal. It then 
 * initializes the module with the desired period and sets it to operate 
 * as a PWM signal.
 *
 * @param period period for the PWM signal to operate in
 * @return None.
 */
void TIM10_PWM_setup(uint32_t period){
	//Enable clock required to configure TIM10
	RCC->APB1ENR |= RCC_APB2ENR_TIM10EN;
	//Calculate the clock counts required create the period requested
	period_cc = (CK_INT / period) - 1;
	//Calculate the base value (i.e. at 10%) for the entire range of duty cycles.
	pulsewidth_base = (period_cc + 1) / 10;
	//Initialize lookup table
	pulseWidthLookup[0] = pulsewidth_base;
	//Add all values to lookup table
	for(i = 1; i < 9; i++){
		pulseWidthLookup[i] = pulseWidthLookup[i - 1] + pulsewidth_base;
	}
	//Add clock counts to ARR (timer output toggles when it reaches this count)
	TIM10->ARR = period_cc;
	TIM10->CCR1 = 0;
	//Set timer to PWM functionality
	TIM10->CCMR1 &= ~(TIM_CCR1_CCR1);
	TIM10->CCMR1 &= ~(TIM_CCMR1_CC1S);
	TIM10->CCMR1 &= ~(TIM_CCMR1_OC1M);
	TIM10->CCMR1 |= TIM_CCMR1_OC1M_1 + TIM_CCMR1_OC1M_2;
	//Enable capture compare interrupts and clear any pending ones
	TIM10->CCER &= ~(TIM_CCER_CC1P);
	TIM10->CCER |= TIM_CCER_CC1E;
	//Set up timer pins and start at low state (GPIO out -> LOW)
	TIM10_pinSetup(PIN_LOW);
}
/*!
 * Set the pin output to the required state.
 *
 * @param State to set pin. Valid inputs:
 * -PIN_LOW: Duty cycle = 0%
 * -PIN_PW: Duty cycle = 10 - 90%
 * -PIN_HIGH: Duty cycle = 100%
 * @return None.
 */
void TIM10_pinSetup(uint8_t state){
	
	if(state == PIN_HIGH | state == PIN_LOW){
		//Set pin to GPIO and state either high or low
		GPIOA->MODER &= ~(GPIO_MODER_MODER6);
		GPIOA->MODER |= GPIO_MODER_MODER6_0;
		
		GPIOA->ODR &= ~(GPIO_ODR_ODR_6);
		GPIOA->ODR &= (GPIO_ODR_ODR_6 | state);
	}
	else{//Set pin to PWM output alternative function
		GPIOA->MODER &= ~(GPIO_MODER_MODER6);
		GPIOA->MODER |= GPIO_MODER_MODER6_1;
		
		GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL6);
		GPIOA->AFR[0] |= GPIO_AFRL_AFSEL6 & AF3;
	}
	
}
/*!
 * Update the duty cycle of the PWM signal out. It takes the clock counts necessary
 * from the lookup table. If duty cycle is 0 or 100%, then it just sets pin to low or
 * high respectivetly. 
 *
 * @param pulseWidth_inx Index to indicate which duty cycle to choose from lookup table
 */
void PWM10_update(uint8_t pulseWidth_idx){
	
	if(pulseWidth_idx == 0){ //Set pin to function as GPIO and LOW
		TIM10_pinSetup(PIN_LOW);
		TIM10->CR1 &= ~TIM_CR1_CEN;
	}
	else if(pulseWidth_idx == 10){ //Set pin to function as GPIO and HIGH
		TIM10_pinSetup(PIN_HIGH);
		TIM10->CR1 &= ~TIM_CR1_CEN;
	}
	else{//Set pin to alternative function and add clock counts neccesesary
		TIM10_pinSetup(PIN_PW);
		TIM10->CCR1 =  pulseWidthLookup[pulseWidth_idx - 1];
		TIM10->CR1 |= TIM_CR1_CEN;
	}
	
}
/*!
 * Generate a small software delay. WARNING: some compilers optimize this
 * function out. 
 *
 * @return None.
 */
void delay (void) 
{
	int i,j,n;
	for (i=0; i<05; i++) { //outer loop
		for (j=0; j<10; j++) { //inner loop
			n = j; //dummy operation for single-step test
		}		 //do nothing
	}
}
/*!
 * IRQ handler for external interrupt vector 1. This function is triggered when PA1
 * sees a low state. It then reads the state of pins PB7-4, changes PB7-4 to output-high
 * and PB3-0 to input and reads the input. This inidcates the value of the rows and columns
 * when the button is pressed.
 *
 * @return None. 
 */
void EXTI1_IRQHandler(){
	//Disable all interrupts
	__disable_irq();
	//Read rows
	buttonPressed = (0x000000F0) & (GPIOB->IDR);
	//buttonPressed = buttonPressed << 4;
	//Set PB7-4 as output-high and PB3-0 as input
	setupKeypadPins(PB7_4_OUTPUT);
	//Short delay to allow settling
	delay();
	//Read in state of columns
	buttonPressed |= ((0x0000000F) & (GPIOB->IDR));
	//buttonPressed |= ((0x0000000F) & (GPIOB->IDR)) >> 4;
	//Return to initial state
	setupKeypadPins(PB3_0_OUTPUT);
	//Clear all interrupts and exit
	EXTI->PR |= EXTI_PR_PR1;
	NVIC_ClearPendingIRQ(EXTI1_IRQn);
	
	__enable_irq();
	
}