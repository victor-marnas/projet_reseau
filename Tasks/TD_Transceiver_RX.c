#include "main.h"

void vTaskTransceiverRX( void *pvParameters )
{
	tCAN_msg msg[ 5 ];
	uint8_t msgIndex = 0u;

	static const uint8_t rawMessageBufferSize = 85u;
	uint8_t byteIndex = 0u;
	uint8_t bitIndex = 7u;
	uint8_t rawMessageBuffer[ 85 ] = { 0u };
	uint8_t rawMessageBitBuffer[ 255 ] = { 0u };
	uint8_t rawMessageBitBufferIndex = 0u;
	uint8_t receivedBit = 0u;

	uint8_t eofDetected = 0u;
	uint8_t firstBit = 0u;

	uint8_t numberOfReceivedBits = 0u;
	uint8_t numberOfRecessiveBits = 0u;

	/* Enable and set EXTI_Line0 Interrupt */
	NVIC_SetPriority( EXTI9_5_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY - 1u );
	NVIC_EnableIRQ( EXTI9_5_IRQn );

	NVIC_SetPriority( TIM2_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY - 2u );
	NVIC_EnableIRQ( TIM2_IRQn );

	while( 1 )
	{
		// Blocking wait on received bits queue
		 xQueueReceive( q_rxBits, ( void * const )&receivedBit, portMAX_DELAY );

		 // Add read bits to buffer
		 if ( ( 0u == byteIndex ) && ( 7u == bitIndex ) && ( 0u == firstBit ) )
		 {
			 rawMessageBuffer[ byteIndex ] = 0u;
			 firstBit = 1u;
		 }
		 else
		 {
			 firstBit = 0u;
		 }

		 if ( 0u == firstBit )
		 {
			 rawMessageBitBuffer[ rawMessageBitBufferIndex++ ] = receivedBit;
			 rawMessageBuffer[ byteIndex ] |= ( receivedBit << bitIndex );
			 numberOfReceivedBits++;

			 // Increment counter of recessive bits to detect EOF.
			 if ( RECESSIVE == receivedBit )
			 {
				 numberOfRecessiveBits++;
				 if ( 7u == numberOfRecessiveBits )
				 {
					 eofDetected = 1u;
					 // Reset counters
					 bitIndex = 7u;
					 byteIndex = 0u;
				 }
			 }
			 else
			 {
				 numberOfRecessiveBits = 0u;
			 }

			 // Increment counters
			 if ( 0u == bitIndex )
			 {
				 bitIndex = 7u;
				 byteIndex++;
				 if ( rawMessageBufferSize == byteIndex )
				 {
					 byteIndex = 0u;
				 }
				 rawMessageBuffer[ byteIndex ] = 0u;
			 }
			 else
			 {
				 bitIndex--;
			 }

			 if ( 1u == eofDetected )
			 {
				 TIM_Cmd( TIM2, DISABLE );
				 eofDetected = 0u;
	//			 displayData( rawMessageBuffer );
				 bitToMsg( rawMessageBuffer, 0, &( msg[ msgIndex ] ) );
				 xQueueSendToBack( q_rxMessages, (void*)&( msg[ msgIndex ] ), 0 );
				 numberOfReceivedBits = 0u;
				 numberOfRecessiveBits = 0u;

				 byteIndex = 0u;
				 bitIndex = 7u;

				 if ( 4u == msgIndex )
				 {
					msgIndex = 0u;
				 }
				 else
				 {
					 msgIndex++;
				 }
			 }
		}
	}
}
