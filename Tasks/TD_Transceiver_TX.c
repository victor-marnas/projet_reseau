#include "main.h"

void vTaskTransceiverTX( void *pvParameters )
{
	tCAN_msg msg;
	uint8_t rawMessageBuffer[ 17u ];

	NVIC_SetPriority( TIM2_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY - 1u );
	NVIC_EnableIRQ( TIM2_IRQn );

	while( 1 )
	{
		// Blocking wait on received message queue
		 xQueueReceive( q_txMessages, ( void * const )&msg, portMAX_DELAY );
	}
}
