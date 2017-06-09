// Include main headers
#include "main.h"

void vTaskTapisSortie( void *pvParameters )
{
  subscription_t sensorPalette;
  sensorPalette.sensor = SENSOR_PALETTE;
  sensorPalette.queue = q_sensorsTapisSortie;

  uint8_t presencePalette = 0;

  sensor_t dummy;

  while( 1 )
  {
      DeplacerTapisSortie( 1 );

      sensorPalette.value = 1;
      xQueueSendToFront( q_newSubscriptions , &sensorPalette , portMAX_DELAY );

      xQueueReceive( q_sensorsTapisSortie , ( void * const )&dummy , portMAX_DELAY );

      DeplacerTapisSortie( 0 );

      presencePalette = 1;
      xQueueSendToFront( q_caisseArrivee , &presencePalette , portMAX_DELAY );

      xQueueReceive( q_caissePleine , ( void * const )&dummy , portMAX_DELAY );

      DeplacerTapisSortie( 1 );

      sensorPalette.value = 0;
      xQueueSendToFront( q_newSubscriptions , &sensorPalette , portMAX_DELAY );

      xQueueReceive( q_sensorsTapisSortie , ( void * const )&dummy , portMAX_DELAY );
  }
}

void DeplacerTapisSortie( uint8_t etat )
{
  actuators_t tapis;
  tapis.actuator = ACTUATOR_TAPIS_SORTIE;
  tapis.value = etat;

  xQueueSendToFront( q_actuators , (void * const)&tapis , portMAX_DELAY );
}
