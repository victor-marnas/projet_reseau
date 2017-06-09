// Include main headers
#include "main.h"

void vTaskAcquisitionCapteur( void *pvParameters )
{
  portTickType xLastWakeTime = xTaskGetTickCount();

  subscription_t bufferSubscriptions;
  xQueueHandle t_QueueTable[ NUMBER_OF_SENSORS ];

  uint16_t sensorsToCheck = 0;
  uint16_t sensorsValuesToCheck = 0;

  uint16_t sensors = 0;


  /*while (1)
  {
      uint8_t sensors = I2C_ReadPCFRegister( PCF_A_READ_ADDR );
      I2C_WritePCFRegister( PCF_B_WRITE_ADDR , sensors );
      vTaskDelay(100);
  }*/

  while( 1 )
  {
	STM_EVAL_LEDOn( LED4 );

    while ( xQueueReceive( q_newSubscriptions , ( void * const )&bufferSubscriptions , 0 ) != errQUEUE_EMPTY )
    {
       sensorsToCheck |= 0x0001 << bufferSubscriptions.sensor;

      if ( bufferSubscriptions.value == 1u )
        sensorsValuesToCheck |= 1u << bufferSubscriptions.sensor;
      else
        sensorsValuesToCheck &= 0xFFFF ^ ( 1u << bufferSubscriptions.sensor );

      t_QueueTable[ bufferSubscriptions.sensor ] = bufferSubscriptions.queue;
    }

	char label[ 10 ];
	sprintf( label , "%04X %04X" , sensorsToCheck , sensorsValuesToCheck );
	LCD_DisplayStringLine( 36 , (uint8_t*) label );

    xSemaphoreTake( sem_I2C_BUS , portMAX_DELAY );
    ReadSensors( &sensors );
    xSemaphoreGive( sem_I2C_BUS );

    uint8_t sensorIndex;

    for ( sensorIndex = 0 ; sensorIndex < NUMBER_OF_SENSORS ; sensorIndex++ )
    {
      uint16_t isSensorTested = sensorsToCheck & ( 1u << sensorIndex );

      if ( isSensorTested )
      {
        sensor_t sensor;
        sensor.sensor = sensorIndex;
        sensor.value = ( sensorsValuesToCheck >> sensorIndex ) & 1;

        if ( ( ( sensors >> sensorIndex ) & 1 ) == sensor.value )
        {
            xQueueSendToFront( t_QueueTable[ sensorIndex ] , (void * const)&sensor , portMAX_DELAY );

            sensorsToCheck &= 0xFFFF ^ ( 1u << sensorIndex );
        }
      }
    }

	STM_EVAL_LEDOff( LED4 );
    vTaskDelayUntil( &xLastWakeTime , SENSORS_UPDATE_PERIOD / portTICK_RATE_MS );
  }
}


void ReadSensors( uint16_t * sensors )
{
  *sensors = I2C_ReadPCFRegister( PCF_C_READ_ADDR ) << 8;
  *sensors |= I2C_ReadPCFRegister( PCF_A_READ_ADDR );
}
