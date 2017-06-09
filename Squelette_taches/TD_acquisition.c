void vTaskAcquisitionCapteur( void )
{
	xLastWakeTime = xTaskGetTickCount();
	
	subscription_t subscriptionsTable[ NUMBER_OF_CAPTORS ];
	subscription_t bufferSubscriptions;
	
	uint16_t sensorsToCheck = 0;
	uint16_t sensorsValuesToCheck = 0;
	
	uint16_t sensors = 0;
	
	while( 1 )
	{
		while ( xQueueReceive( q_newSubscriptions , ( const void * )bufferSubscriptions , 0 ) != errQUEUE_EMPTY )
		{
			sensorsToCheck |= 0x0001 << bufferSubscriptions.sensor;
			
			if ( bufferSubscriptions.value == 1u )
				sensorsValuesToCheck |= 1u << bufferSubscriptions.sensor;
			else
				sensorsValuesToCheck &= 0xFF ^ ( 1u << bufferSubscriptions.sensor );
		}
		
		xSemaphoreTake( s_I2C_BUS );
		ReadSensors( &sensors );
		xSemaphoreGive( s_I2C_BUS );
		
		uint8_t sensorIndex;
		
		for ( sensorIndex = 0 ; sensorIndex < NUMBER_OF_CAPTORS ; sensorIndex++ )
		{
			uint16_t isSensorTested = sensorsToCheck & ( 1u << sensorIndex )
			
			if ( isSensorTested )
			{
				uint8_t value = sensorsValuesToCheck >> sensorIndex;
				
				if ( ( sensors >> sensorIndex ) == value )
				{
					xQueueSendToFront( testedSubscription->queue , value , 0 );
				}
			}
		}	
	}
}


void ReadSensors( uint16_t * sensors )
{
	*sensors = I2C_ReadPCFRegister( PCF_B_READ_ADDR ) << 8;	
	*sensors |= I2C_ReadPCFRegister( PCF_A_READ_ADDR );
}
