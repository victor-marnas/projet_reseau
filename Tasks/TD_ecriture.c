// Include main headers
#include "main.h"

void vTaskEcritureActionneur( void *pvParameters )
{
  portTickType xLastWakeTime = xTaskGetTickCount();

  uint8_t actuatorsValues = 0;
  uint8_t previousActuatorsValues = 0;

  actuators_t bufferActuator;

  while( 1 )
  {
	xQueueReceive( q_actuators , ( const void * )&bufferActuator , portMAX_DELAY ) ;

	if ( bufferActuator.value == 1 )
      actuatorsValues |= 1 << bufferActuator.actuator;
    else
      actuatorsValues &= 0xFF ^ ( 1 << bufferActuator.actuator );

    while ( xQueueReceive( q_actuators , ( const void * )&bufferActuator , 0 ) != errQUEUE_EMPTY )
    {
      if ( bufferActuator.value == 1 )
        actuatorsValues |= 1 << bufferActuator.actuator;
      else
        actuatorsValues &= 0xFF ^ ( 1 << bufferActuator.actuator );
    }

    char label[ 15 ];
	sprintf( label , "Actuators : %02X" , actuatorsValues );
	LCD_DisplayStringLine( 48 , (uint8_t*) label );

    xSemaphoreTake( sem_I2C_BUS , portMAX_DELAY );
    WriteActuators( &actuatorsValues );
    xSemaphoreGive( sem_I2C_BUS );

   // vTaskDelayUntil( &xLastWakeTime , ACTUATORS_UPDATE_PERIOD / portTICK_RATE_MS );
  }
}


void WriteActuators( uint8_t * actuators )
{
  I2C_WritePCFRegister( PCF_B_WRITE_ADDR , *actuators );
}
