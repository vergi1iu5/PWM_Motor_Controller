/**
 * @file main.h
 * @author Ivan Ramos <izr0006@auburn.edu>
 * @version 1.3
 * @brief Interface for defining all variables and functions used 
 * for implementing the PWM interface to a motor controller. The duty
 * cycle is controlled by a 16-key keypad.
 * @description 
 *
 * This is the file for defining all macros and functions required to interface a 16-key
 * keypad, a PWM generator, and a DC motor. This module also comes with a way to add LEDs
 * which displays the current speed of the motor (0 - 100%). To start, you need to call 
 * pinSetup(), setupKeypadPins(), TIM10_PWM_setup(_freq) where _freq is the PWM frequency, 
 * and finally enableInterrupts(). Pooling buttonPressed will indicated if a button in the
 * keypad has been pressed. The function desipherButton() will then take the variable buttonPressed
 * to determine the state of the LEDs and dutyCycle. 
 *
 */

#ifndef MAIN_H
#define MAIN_H

#include "stm32l1xx.h"
/**
 * @defgroup PIN_MASKS Masks for configuring GPIO and ports used for PWM and clocks.
 * 
 *
 * @{
 */
#define PB3_0_OUTPUT (0x000000FF) ///<Mask to set PB 3 through 0 as output
#define PB7_4_OUTPUT (0x0000FF00) ///<Mask to set PB 7 through 4 as output
#define GPIO_OUTPUT (0x55555555)  ///<Mask for setting GPIO pins as output
#define GPIO_INPUT (0x00000000)   ///<Mask for setting GPIO pins as input
#define GPIO_PULLUP (0x55555555)  ///<Mask for setting GPIO pins with internal pull-up resistors
#define GPIO_PULLDOWN (0xAAAAAAAA)///<Mask for setting GPIO pins with internal pull-down resistors
#define BIT0 (0x0001)
#define BIT1 (0x0002)
#define BIT2 (0x0004)
#define BIT3 (0x0008)
#define CK_INT (0x00200000)		  ///<Defualt configuration for Clock speed
#define PIN_LOW (0x00)		      ///<Set pin to LOW state
#define PIN_HIGH (0xFF)           ///<Set pin to HIGH state
#define PIN_PW (0x0F)		      ///<Set pin to period required
#define AF3 (0x33333333);         ///<Alternate function 3 (PWM)
/* @} */

static uint16_t buttonPressed; ///<Captures the exact key pressed with each bit indicating 0 - F
uint16_t LEDS; ///<Variable to show the needed state to display number 0 - 10 in binary (ex: 2 -> 0010)
uint16_t colums; ///<Variable to store the state of the columns when the keypad is pressed
uint16_t rows; ///<Variable to store the state of the rows when the keypad is pressed 
uint16_t output;
uint32_t pulseWidthLookup[9] = {0}; ///<Lookup table to calculate the exact CCP register value needed to set PWM dutycycle 0 - 100%
uint32_t period_cc; ///<Value needed to set the PWM period requested
uint32_t pulsewidth_base; ///<Base number used to calculate pulseWidthLookup[] table. 
uint8_t i;

/*!
 * Setup all pins needed to display LEDs and control the motor.
 */
void pinSetup(void);
/*!
 * Setup all pins needed to take in input from the keypad. Provide which keys will be used
 * as input and which will be used as output.
 *
 * @param outputPinsMask pin group to be used as output. Valid inputs: PB3_0_OUTPUT or PB7_4_OUTPUT
 */
void setupKeypadPins(uint32_t);
/*!
 * Enable all interrupts required.
 */
void enableInterrupts(void);
/*!
 * Take in the button pressed variable and return the required state for the LEDs.
 *
 * @param code button pressed value
 * @return state required to be displayed by LEDs
 */
uint16_t desipherButton(uint16_t);
/*!
 * Generate a short delay.
 */
void delay(void);
/*!
 * Update the state of the pins controlling the LEDs.
 *
 * @param display value to set LEDs to
 */
void updateLEDs(uint16_t);
/*!
 * External interrupt vector handler. This function has the logic required to
 * take in the key pressed.
 */
void EXTI1_IRQHandler(void);
/*!
 * Setup timer 10 to function as a PMW module with the provided period.
 *
 * @param period Period to set to.
 */
void TIM10_PWM_setup(uint32_t);
/*!
 * Setup timer 10 pins necessary to extract the internal PWM module signal
 *
 * @param state valid valuse: PIN_LOW (duty cycle = 0%), PIN_HIGH (duty cycle = 100%), or PIN_PW (duty_cycle 10 - 90%)
 */
void TIM10_pinSetup(uint8_t);
/*!
 * Update the state of the PWM10 module to a new duty cycle.
 *
 * @param pulsewidth index 0 - 10. Indicates 0 - 100% duty cycle.
 */
void PWM10_update(uint8_t);

#endif