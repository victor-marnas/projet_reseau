// Include main headers
#include "main.h"

void vTaskTapisEntree( void *pvParameters )
{
  subscription_t sensorToSubscribe;
  sensorToSubscribe.queue = q_sensorsTapisEntree;

  uint8_t t_cpt_type_manquante[3];
  uint8_t cpt_pieces_prises = 0;

  sensor_t firstSensorType;
  sensor_t secondSensorType;
  sensor_t dummy;

  uint8_t typePiece = 0;

  while( 1 )
  {
      uint8_t type;
      for ( type = 0 ; type < 3 ; type++ )
        t_cpt_type_manquante[ type ] = 3;

      cpt_pieces_prises = 0;

      while( cpt_pieces_prises != 9 )
      {
    	  LCD_DisplayStringLine(24, (uint8_t*) "Avance tapis");
          DeplacerTapisEntree( 1 );

          sensorToSubscribe.sensor = SENSOR_TYPE_S0;
          sensorToSubscribe.value = 1;

          xQueueSendToFront( q_newSubscriptions , ( void * const )&sensorToSubscribe , portMAX_DELAY );
          sensorToSubscribe.sensor = SENSOR_TYPE_S1;
          xQueueSendToFront( q_newSubscriptions , ( void * const )&sensorToSubscribe , portMAX_DELAY );

          firstSensorType.value = 0;
          secondSensorType.value = 0;

          xQueueReceive( q_sensorsTapisEntree , &firstSensorType , portMAX_DELAY );
          xQueueReceive( q_sensorsTapisEntree , &secondSensorType , 0 );

          typePiece = ( ( firstSensorType.value << firstSensorType.sensor ) | ( secondSensorType.value << secondSensorType.sensor )) - 1;

          char label[ 15 ];
          sprintf(label, "Piece type : %d", typePiece);
    	  LCD_DisplayStringLine( 24 , (uint8_t*) label );

          if( t_cpt_type_manquante[ typePiece ] != 0)
          {
              t_cpt_type_manquante[ typePiece ]--;

              sensorToSubscribe.sensor = SENSOR_BOUT_TAPIS_ENTREE;
              sensorToSubscribe.value = 1;

              LCD_DisplayStringLine( 60 , "Subscription sen2" );
              xQueueSendToFront( q_newSubscriptions , ( void * const )&sensorToSubscribe , portMAX_DELAY );
              LCD_DisplayStringLine( 60 , "Subscription done" );
              xQueueReceive( q_sensorsTapisEntree , ( void * const )&dummy , portMAX_DELAY );
              LCD_DisplayStringLine( 60 , "Receive sen2 done" );

              DeplacerTapisEntree( 0 );

              xQueueSendToFront( q_pieceDispo , ( void * const )&typePiece , portMAX_DELAY );
              xQueueReceive( q_piecePrise , ( void * const )&dummy , portMAX_DELAY );
              // xQueueReceive( q_piecePrise , ( void * const )&dummy , 0 );

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
