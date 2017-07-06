#include "main.h"

void vTaskTransceiverRX( void *pvParameters )
{
	static const uint8_t rawMessageBufferSize = 192u;
	uint8_t messageIndex = 0u;
	uint8_t bitIndex = 0u;
	uint8_t rawMessageBuffer[ 192 ] = { 0u };
	uint8_t receivedBit = 0u;
	uint8_t numberOfReceivedBits = 0u;
	uint8_t numberOfRecessiveBits = 0u;
	uint8_t eofDetected = 0u;

	/* Enable and set EXTI_Line0 Interrupt */
	NVIC_SetPriority( EXTI0_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY - 1u );
	NVIC_EnableIRQ( EXTI0_IRQn );

	NVIC_SetPriority( TIM2_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY - 2u );
	NVIC_EnableIRQ( TIM2_IRQn );

	while( 1 )
	{
		// Blocking wait on received bits queue
		 xQueueReceive( q_rxBits, ( void * const )&receivedBit, portMAX_DELAY );

		 // Add read bits to buffer
		 rawMessageBuffer[ messageIndex ] = ( receivedBit << bitIndex++ );
		 numberOfReceivedBits++;

		 // Increment counter of recessive bits to detect EOF.
		 if ( 0u == receivedBit )
		 {
			 numberOfRecessiveBits++;
			 if ( 7u == numberOfRecessiveBits )
			 {
				 eofDetected = 1u;
				 // Reset counters
				 bitIndex = 0u;
				 messageIndex = 0u;
			 }
		 }
		 else
		 {
			 numberOfRecessiveBits = 0u;
			 // Increment counters
			 if ( 8u == bitIndex )
			 {
				 bitIndex = 0u;
				 messageIndex++;
				 if ( rawMessageBufferSize == messageIndex )
				 {
					 messageIndex = 0u;
				 }
			 }
		 }
	}
}
