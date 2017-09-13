#include "main.h"

void vTaskSniffer( void *pvParameters )
{
	tCAN_msg receivedMessage;
	while( 1 )
	{
		// Blocking wait on received bits queue
		xQueueReceive( q_rxMessages, ( void * const )&receivedMessage, portMAX_DELAY );

		LCD_DisplayStringLine( 12, (uint8_t*)"Received Frame" );

		if ( 1u == receivedMessage.isValid )
		{
			LCD_DisplayStringLine( 24, (uint8_t*)"Frame correct      " );
		}
		else
		{
			if (receivedMessage.error == BIT_STUFFING_ERR) {
				LCD_DisplayStringLine( 24, (uint8_t*)"Bit stuffing error" );
			}
			else if (receivedMessage.error == CRC_ERR) {
				LCD_DisplayStringLine( 24, (uint8_t*)"CRC error         " );
			}

			//LCD_DisplayStringLine( 24, (uint8_t*)"Frame NOT correct " );
		}

		char ID[ 15 ] = { 0u };
		sprintf( ID, "ID : 0x%03X", receivedMessage.ID );
		LCD_DisplayStringLine( 36, (uint8_t*)ID );

		char data[ 19 ] = { 0u };
		sprintf( data,
				 "0x%02X 0x%02X 0x%02X 0x%02X",
				 receivedMessage.data[ 0 ],
				 receivedMessage.data[ 1 ],
				 receivedMessage.data[ 2 ],
				 receivedMessage.data[ 3 ] );
		LCD_DisplayStringLine( 48, (uint8_t*)data );
		sprintf( data,
				 "0x%02X 0x%02X 0x%02X 0x%02X",
				 receivedMessage.data[ 4 ],
				 receivedMessage.data[ 5 ],
				 receivedMessage.data[ 6 ],
				 receivedMessage.data[ 7 ] );
		LCD_DisplayStringLine( 60, (uint8_t*)data );

		sprintf( data,
				 "crcR %04X crcC %04X",
				 receivedMessage.crc,
				 crc( &receivedMessage ) );
		LCD_DisplayStringLine( 84, (uint8_t*)data );
	}
}
