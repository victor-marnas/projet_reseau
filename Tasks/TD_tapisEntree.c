// Include main headers
#include "main.h"

void vTaskTapisEntree( void *pvParameters )
{
  subscription_t sensorToSubscribe;
  sensorToSubscribe.queue = q_sensorsTapisEntree;

  uint8_t t_cpt_type_manquante[3];
  uint8_t cpt_pieces_prises = 0;

  sensor_t sensorType0;
  sensor_t sensorType1;

  uint8_t typePiece = 0;

  while( 1 )
  {
      uint8_t type;
      for ( type = 0 ; type < 3 ; type++ )
        t_cpt_type_manquante[ type ] = 3;

      cpt_pieces_prises = 0;

      while( cpt_pieces_prises != 9 )
      {
          DeplacerTapisEntree( 1 );

          sensorToSubscribe.sensor = SENSOR_TYPE_S0;
          sensorToSubscribe.value = 1;

          xQueueSendToFront( q_newSubscriptions , ( void * const )&sensorToSubscribe , portMAX_DELAY );
          sensorToSubscribe.sensor = SENSOR_TYPE_S1;
          xQueueSendToFront( q_newSubscriptions , ( void * const )&sensorToSubscribe , portMAX_DELAY );

          sensorType0.value = 0;
          sensorType1.value = 0;
          xQueueReceive( q_sensorsTapisEntree , &sensorType0 , portMAX_DELAY );
          xQueueReceive( q_sensorsTapisEntree , &sensorType1 , 0 );

          typePiece = sensorType0.value | ( sensorType1.value << 1 );

          if( t_cpt_type_manquante[ typePiece ] != 0)
          {
              t_cpt_type_manquante[ typePiece ]--;

              sensorToSubscribe.sensor = SENSOR_BOUT_TAPIS_ENTREE;
              sensorToSubscribe.value = 1;

              xQueueSendToFront( q_newSubscriptions , ( void * const )&sensorToSubscribe , portMAX_DELAY );
              xQueueReceive( q_sensorsTapisEntree , ( void * const )NULL , portMAX_DELAY );

              DeplacerTapisEntree( 0 );

              xQueueSendToFront( q_pieceDispo , ( void * const )&typePiece , portMAX_DELAY );
              xQueueReceive( q_piecePrise , ( void * const )NULL , portMAX_DELAY );

              cpt_pieces_prises++;
          }
      }
  }
}

void DeplacerTapisEntree( uint8_t etat )
{
  actuators_t tapis;
  tapis.actuator = ACTUATOR_TAPIS_ENTREE;
  tapis.value = etat;

  xQueueSendToFront( q_actuators , (void * const)&tapis , portMAX_DELAY );
}
