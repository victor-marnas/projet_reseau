#include "main.h"


void delay_us( uint32_t delay )
{
	uint32_t i = delay * 90u;
	for ( ; i > 0u; i-- )
	{
		__asm( "NOP" );
	}
}

void vTaskInjection( void *pvParameters )
{
	uint8_t i = 0u;
	tCAN_msg msg;
	uint8_t * rawMessageBuffer = (uint8_t *)pvParameters;
	uint8_t size = 20u;

	const TickType_t xDelay = 200 / portTICK_PERIOD_MS;
	TickType_t xLastWakeTime = xTaskGetTickCount();

	while( 1 )
	{
		// Send message every 1s
		 vTaskDelayUntil( &xLastWakeTime, xDelay );

		 msg.ID = 0u;
		 msg.RTR = 1u;

		 for ( i = 0u; i < 8u; i++ )
		 {
			 msg.data[ i ] = 0xFFu;
		 }

		 msg.dataLength = 8u;
		 msg.error = 0u;
		 msg.isValid = 1u;
		 msg.crc = crc(&msg);//0x00u;
		 for ( i = 0u; i < 20u; i++ )
		 {
			 rawMessageBuffer[ i ] = 0x55u;
		 }
		 msgToBit( &msg, rawMessageBuffer, &size );

 		 xQueueSendToBack( q_txMessages, (void*)&size, portMAX_DELAY );
	}
}


void vTaskTransceiverTX( void *pvParameters )
{
	NVIC_SetPriority( TIM2_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY - 2u );
	NVIC_EnableIRQ( TIM2_IRQn );

	uint8_t * toSend = (uint8_t *)pvParameters;
	uint8_t size = 0u;
	uint8_t i = 0u;
	uint8_t j = 0u;
	uint8_t bitToSend = 0u;
	uint8_t bitSent = 0u;

	while( 1 )
	{
		// Blocking wait on received message queue
		xQueueReceive( q_txMessages, ( void* )&size, portMAX_DELAY );

		bitSent = 0u;

		TIM2->ARR = 1800;
		TIM_SetCounter( TIM2, 0 );
		TIM_Cmd( TIM2, ENABLE );

		for( i = 0u; i < ( size >> 3 ); i++ )
		{
			for( j = 0u; j < 7u; j++ )
			{
				if( bitSent < size )
				{

					xSemaphoreTake( xSemTimerSend, portMAX_DELAY );

					bitSent++;
					bitToSend = ( ( toSend[ i ] & ( 1u << j ) ) >> j );

					GPIO_WriteBit( GPIOA, GPIO_Pin_3, bitToSend );
				}
			}
		}

		TIM_Cmd( TIM2, DISABLE );
	}
}

