#include "main.h"


void InitTransceiverRX( void )
{
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_SYSCFG, ENABLE );
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA, ENABLE );

	GPIO_InitTypeDef GPIO_InitStruct;
	EXTI_InitTypeDef EXTI_InitStruct;

	GPIO_InitStruct.GPIO_Pin	= GPIO_Pin_5;
	GPIO_InitStruct.GPIO_Mode	= GPIO_Mode_IN; 	// we want the pins to be an input
	GPIO_InitStruct.GPIO_Speed	= GPIO_Speed_50MHz; // this sets the GPIO modules clock speed
	GPIO_InitStruct.GPIO_PuPd	= GPIO_PuPd_DOWN; // this sets the pullup / pulldown resistors to be inactive
	GPIO_Init( GPIOA, &GPIO_InitStruct );

	GPIO_InitStruct.GPIO_Pin	= GPIO_Pin_6;
	GPIO_InitStruct.GPIO_Mode	= GPIO_Mode_OUT; 	// we want the pins to be an output
	GPIO_InitStruct.GPIO_Speed	= GPIO_Speed_50MHz; // this sets the GPIO modules clock speed
	GPIO_InitStruct.GPIO_PuPd	= GPIO_PuPd_NOPULL; // this sets the pullup / pulldown resistors to be inactive
	GPIO_Init( GPIOA, &GPIO_InitStruct );

	// Connect EXTI5 Line to PA5 pin
	SYSCFG_EXTILineConfig( EXTI_PortSourceGPIOA, EXTI_PinSource5 );

	// Configure EXTI5 line
	EXTI_InitStruct.EXTI_Line = EXTI_Line5;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_Init( &EXTI_InitStruct );
}


void InitTimer( void )
{
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM2, ENABLE );

	TIM_TimeBaseInitTypeDef TIM_InitStruct;

	TIM_InitStruct.TIM_Prescaler = 0;
	TIM_InitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_InitStruct.TIM_Period = 839;
	TIM_InitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_InitStruct.TIM_RepetitionCounter = 0;

	TIM_TimeBaseInit( TIM2, &TIM_InitStruct );

	TIM_ITConfig( TIM2, TIM_IT_Update, ENABLE );
	TIM_Cmd( TIM2, DISABLE );
}


