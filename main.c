
/*************************************************************************************************
 * 					ITS-PLC Startup Project
 *
 * 					(c) Polytech Montpellier - MEA
 *
 * 					20/09/2015 - Laurent Latorre
 *
 * 					FreeRTOS getting started project to work with ITS-PLC Simulator
 *
 * 					Creates 3 asynchronous tasks :
 *
 * 					- Graphical User Interface (TouchScreen and I/O extender interface)
 * 					- Task1 : Actuator A00 toggle
 * 					- Task2 : Actuator A01 copies the state of Sensor S00
 *
 *************************************************************************************************/


// Local defines

#define	PAD_WIDTH	56
#define PAD_HEIGHT	36
#define PAD_MARGIN	3

#define DBG_OFFLINE	0
#define	DBG_ONLINE	1

#include "main.h"

// Trace Labels
// traceLabel user_event_channel;

// Main program
int main( void )
{
	// ARM function, disable subpreemption
	NVIC_SetPriorityGrouping( NVIC_PriorityGroup_4 );

    // Init board LEDS
    STM_EVAL_LEDInit(LED3);
    STM_EVAL_LEDInit(LED4);

    // Init LCD
	LCD_Init();
	LCD_LayerInit();
	LTDC_Cmd(ENABLE);
	LCD_SetLayer(LCD_FOREGROUND_LAYER);
	LCD_Clear(0xFFFF);
	LCD_SetTextColor(0x0000);
	LCD_SetFont(&Font12x12);

    // Start IO Touchpad
    IOE_Config();

    // Init
    InitTransceiverRX( );
    // Timer 2 interrupt for CAN acquisition
    InitTimer( );

    // Launch FreeRTOS Trace recording
    /*
    vTraceInitTraceData();
    uiTraceStart();
	*/
    // Initialize resources management variable
    // user_event_channel = xTraceOpenLabel("UEV");

    // SNIFFER is defined in user_task.h
#ifdef SNIFFER
    q_rxBits = xQueueCreate( 1024, sizeof( uint8_t ) );
	q_rxMessages = xQueueCreate( 10, sizeof( tCAN_msg ) );
    xTaskCreate( vTaskSniffer , "Task_Sniffer" , 128 , NULL , 10 , NULL );
    xTaskCreate( vTaskTransceiverRX , "Task_Transceiver_RX" , 1024 , NULL , 20 , NULL );
#else
    xSemTimerSend = xSemaphoreCreateBinary( );
    q_txMessages = xQueueCreate( 1, sizeof( uint8_t ) );
    xTaskCreate( vTaskTransceiverTX , "Task_Transceiver_TX" , 1024 , ( void* )bitsToTransmit , 20 , NULL );
    xTaskCreate( vTaskInjection , "Task_Fault_Injection" , 1024 , ( void* )bitsToTransmit , 10 , NULL );
#endif

    LCD_DisplayStringLine(12, (uint8_t*) "Init done");

    // Start the Scheduler
    vTaskStartScheduler();

    LCD_DisplayStringLine( 36 , (uint8_t*) "Init succeed" );

    while(1)
    {
    	// The program should never be here...
    }
}

void EXTI9_5_IRQHandler( void )
{
	if( RESET != EXTI_GetITStatus( EXTI_Line5 ) )
	{
		TIM_Cmd( TIM2, DISABLE );

		// Toggle output for timing debug
		if ( RESET == GPIO_ReadOutputDataBit( GPIOA, GPIO_Pin_6 ) )
		{
			GPIO_SetBits( GPIOA, GPIO_Pin_6 );
		}
		else
		{
			GPIO_ResetBits( GPIOA, GPIO_Pin_6 );
		}
		// GPIO_SetBits( GPIOA, GPIO_Pin_6 );

		uint8_t toSend = (uint8_t)GPIO_ReadInputDataBit( GPIOA, GPIO_Pin_5 );
		portBASE_TYPE higherPriorityWoken = pdFALSE;

		// Clear the EXTI line 5 pending bit
		EXTI_ClearITPendingBit( EXTI_Line5 );

		// Set timer AutoReload Register to 500 ( 1/250000kbits/s + ~25% )
		TIM2->ARR = 2238; //1679;
		TIM_SetCounter( TIM2, 0 );
		TIM_Cmd( TIM2, ENABLE );

		xQueueSendToBackFromISR( q_rxBits, (void*)&toSend, &higherPriorityWoken );

		// GPIO_ResetBits( GPIOA, GPIO_Pin_6 );

		portEND_SWITCHING_ISR( higherPriorityWoken );
	}
}

void TIM2_IRQHandler( void )
{
#ifdef SNIFFER
	if( RESET != TIM_GetITStatus( TIM2, TIM_IT_Update ) )
	{
		// Toggle output for timing debug
		if ( RESET == GPIO_ReadOutputDataBit( GPIOA, GPIO_Pin_6 ) )
		{
			GPIO_SetBits( GPIOA, GPIO_Pin_6 );
		}
		else
		{
			GPIO_ResetBits( GPIOA, GPIO_Pin_6 );
		}


		// Clear interrupt
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

		uint8_t toSend = (uint8_t)GPIO_ReadInputDataBit( GPIOA, GPIO_Pin_5 );
		portBASE_TYPE higherPriorityWoken = pdFALSE;

		// 4 us = 1/( 250 000 kbits/s )
		TIM2->ARR = 1679;  //360

		xQueueSendToBackFromISR( q_rxBits, (void*)&toSend, &higherPriorityWoken );

		portEND_SWITCHING_ISR( higherPriorityWoken );
	}
#else
	if( RESET != TIM_GetITStatus( TIM2, TIM_IT_Update ) )
	{
		portBASE_TYPE higherPriorityWoken = pdFALSE;

		// Clear interrupt
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

		xSemaphoreGiveFromISR( xSemTimerSend, &higherPriorityWoken );

		portEND_SWITCHING_ISR( higherPriorityWoken );
	}
#endif
}
