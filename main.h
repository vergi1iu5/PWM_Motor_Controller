
#include "stm32l1xx.h"

#define PB3_0_OUTPUT (0x000000FF)
#define PB7_4_OUTPUT (0x0000FF00)
#define GPIO_OUTPUT (0x55555555)
#define GPIO_INPUT (0x00000000)
#define GPIO_PULLUP (0x55555555)
#define GPIO_PULLDOWN (0xAAAAAAAA)
#define BIT0 (0x0001)
#define BIT1 (0x0002)
#define BIT2 (0x0004)
#define BIT3 (0x0008)
#define CK_INT (0x00200000)
#define PIN_LOW (0x00)
#define PIN_HIGH (0xFF)
#define PIN_PW (0x0F)
#define AF3 (0x33333333);

static uint16_t buttonPressed;
uint16_t LEDS;
uint16_t colums;
uint16_t rows;
uint16_t output;
uint32_t pulseWidthLookup[9] = {0};
uint32_t period_cc;
uint32_t pulsewidth_base;
uint8_t i;

void pinSetup(void);
void setupKeypadPins(uint32_t);
void enableInterrupts(void);
uint16_t desipherButton(uint16_t);
void delay(void);
void updateLEDs(uint16_t);
void EXTI1_IRQHandler(void);
void TIM10_PWM_setup(uint32_t);
void TIM10_pinSetup(uint8_t);
void PWM10_update(uint8_t);
