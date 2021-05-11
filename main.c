#include "main.h"
#include "stm32l1xx.h"

int main(){
	
	pinSetup();
	TIM10_PWM_setup(1000);
	enableInterrupts();
	buttonPressed = 0;
	LEDS = 0xFF;
	updateLEDs(LEDS);
	
	while(1){
		if(buttonPressed){
			LEDS = desipherButton(buttonPressed);
			updateLEDs(LEDS);
			if(LEDS <= 10){PWM10_update(LEDS);}
			buttonPressed = 0;
			LEDS = 0x00FF;
			delay();
		}
	}
}

void pinSetup(){
	
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN;
	
	//Set PA1 as input with pull down resistor
	GPIOA->MODER &= ~(GPIO_MODER_MODER1);
	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR1);
	GPIOA->PUPDR |= (GPIO_PUPDR_PUPDR1 & GPIO_PULLDOWN);
	
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
	
	setupKeypadPins(PB3_0_OUTPUT);
}

void enableInterrupts(){
	
	SYSCFG->EXTICR[0] &= ~(SYSCFG_EXTICR1_EXTI1);
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI1_PA;
	
	EXTI->RTSR &= ~(EXTI_RTSR_TR1);
	EXTI->FTSR |= EXTI_FTSR_TR1;
	EXTI->IMR |= EXTI_IMR_MR1;
	EXTI->PR |= EXTI_PR_PR1;
	
	NVIC_EnableIRQ(EXTI1_IRQn);
	NVIC_ClearPendingIRQ(EXTI1_IRQn);
	
	__enable_irq();
}

void setupKeypadPins(uint32_t outputPinsMask){
	
	
	uint32_t inputPinsMask = ~((0xFFFF0000)|(outputPinsMask));
	
	GPIOB->MODER &= ~(outputPinsMask);
	GPIOB->MODER |= (outputPinsMask & GPIO_OUTPUT);
	GPIOB->ODR &= ~(outputPinsMask);
	
	GPIOB->MODER &= ~(inputPinsMask);
	GPIOB->MODER |= (inputPinsMask & GPIO_INPUT);	
	GPIOB->PUPDR &= ~(inputPinsMask);
	GPIOB->PUPDR |= (inputPinsMask & GPIO_PULLUP);
}

void updateLEDs(uint16_t display){
	
	GPIOC->ODR &= (0xFFFFFF00);
	GPIOC->ODR |= display;
	
}

uint16_t desipherButton(uint16_t code){
	
	rows = (0x00F0)&(code);
	rows = rows >> 4;
	rows = ~rows;
	
	colums = (0x000F)&(code);
	colums = ~colums;
	
	output = 0;
	
	if (colums & BIT3){output = 1;}
	if (colums & BIT2){output = 2;}
	if (colums & BIT1){output = 3;}
	if (colums & BIT0){
		output = 10;
		if (rows & BIT3){output += 0;}
		if (rows & BIT2){output += 1;}
		if (rows & BIT1){output += 2;}
		if (rows & BIT0){output += 3;}
		return output;
	}
	
	if (rows & BIT3){output += 0;}
	if (rows & BIT2){output += 3;}
	if (rows & BIT1){output += 6;}
	if (rows & BIT0){
		if (colums & BIT3){output = 14;}
		if (colums & BIT2){output = 0;}
		if (colums & BIT1){output = 15;}
	}
	
	return output;
	
}

void TIM10_PWM_setup(uint32_t period){
	
	RCC->APB1ENR |= RCC_APB2ENR_TIM10EN;
	
	period_cc = (CK_INT / period) - 1;
	pulsewidth_base = (period_cc + 1) / 10;
	pulseWidthLookup[0] = pulsewidth_base;
	
	for(i = 1; i < 9; i++){
		pulseWidthLookup[i] = pulseWidthLookup[i - 1] + pulsewidth_base;
	}
	
	TIM10->ARR = period_cc;
	TIM10->CCR1 = 0;
	
	TIM10->CCMR1 &= ~(TIM_CCR1_CCR1);
	TIM10->CCMR1 &= ~(TIM_CCMR1_CC1S);
	TIM10->CCMR1 &= ~(TIM_CCMR1_OC1M);
	TIM10->CCMR1 |= TIM_CCMR1_OC1M_1 + TIM_CCMR1_OC1M_2;
	
	TIM10->CCER &= ~(TIM_CCER_CC1P);
	TIM10->CCER |= TIM_CCER_CC1E;
	
	TIM10_pinSetup(PIN_LOW);
}

void TIM10_pinSetup(uint8_t state){
	
	if(state == PIN_HIGH | state == PIN_LOW){
		GPIOA->MODER &= ~(GPIO_MODER_MODER6);
		GPIOA->MODER |= GPIO_MODER_MODER6_0;
		
		GPIOA->ODR &= ~(GPIO_ODR_ODR_6);
		GPIOA->ODR &= (GPIO_ODR_ODR_6 | state);
	}
	else{
		GPIOA->MODER &= ~(GPIO_MODER_MODER6);
		GPIOA->MODER |= GPIO_MODER_MODER6_1;
		
		GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL6);
		GPIOA->AFR[0] |= GPIO_AFRL_AFSEL6 & AF3;
	}
	
}

void PWM10_update(uint8_t pulseWidth_idx){
	
	if(pulseWidth_idx == 0){
		TIM10_pinSetup(PIN_LOW);
		TIM10->CR1 &= ~TIM_CR1_CEN;
	}
	else if(pulseWidth_idx == 10){
		TIM10_pinSetup(PIN_HIGH);
		TIM10->CR1 &= ~TIM_CR1_CEN;
	}
	else{
		TIM10_pinSetup(PIN_PW);
		TIM10->CCR1 =  pulseWidthLookup[pulseWidth_idx - 1];
		TIM10->CR1 |= TIM_CR1_CEN;
	}
	
}

void delay (void) 
{
	int i,j,n;
	for (i=0; i<05; i++) { //outer loop
		for (j=0; j<10; j++) { //inner loop
			n = j; //dummy operation for single-step test
		}		 //do nothing
	}
}

void EXTI1_IRQHandler(){
	
	__disable_irq();
	
	buttonPressed = (0x000000F0) & (GPIOB->IDR);
	//buttonPressed = buttonPressed << 4;
	
	setupKeypadPins(PB7_4_OUTPUT);
	
	delay();

	buttonPressed |= ((0x0000000F) & (GPIOB->IDR));
	//buttonPressed |= ((0x0000000F) & (GPIOB->IDR)) >> 4;

	setupKeypadPins(PB3_0_OUTPUT);
	
	EXTI->PR |= EXTI_PR_PR1;
	NVIC_ClearPendingIRQ(EXTI1_IRQn);
	
	__enable_irq();
	
}