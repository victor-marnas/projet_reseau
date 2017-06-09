// Include main headers
#include "main.h"

void vTaskRobot( void *pvParameters )
{
  subscription_t sensorToSubscribe;
  sensorToSubscribe.queue = q_sensorsTapisEntree;

  uint8_t t_cpt_type_a_placer[] = { 3 , 3 , 3 };

  uint8_t typePiece = 0;
  uint8_t caissePleine = 1;
  uint8_t piecePrise = 1;

  uint8_t dummy;
  resetRobot();

  while( 1 )
  {
	  uint8_t type = 0;

	  deplacerRobot( 0 , 0 );

	  xQueueReceive( q_pieceDispo , (uint8_t * const)&type , portMAX_DELAY );
	  char label[ 11 ];
	  sprintf( label , "Type : %03d" , type );
	  LCD_DisplayStringLine( 72 , (uint8_t*) label );
	  prendrePiece( 1 );

	  xQueueSendToFront( q_piecePrise , (void * const)&piecePrise , portMAX_DELAY);

	  deplacerRobot( type , t_cpt_type_a_placer[type] );
	  t_cpt_type_a_placer[type]--;

	  xQueuePeek( q_caisseArrivee , (void * const)&dummy , portMAX_DELAY );

	  prendrePiece( 0 );

	  if ( !(t_cpt_type_a_placer[0] + t_cpt_type_a_placer[1] + t_cpt_type_a_placer[2]) )
	  {
		  uint8_t type;
		  for ( type = 0 ; type < 3 ; type++ )
			t_cpt_type_a_placer[ type ] = 3;

		  xQueueReceive( q_caisseArrivee , (void * const)&dummy , portMAX_DELAY );
		  xQueueSendToFront(q_caissePleine , (void * const)&caissePleine , portMAX_DELAY );
	  }
  }
}

void deplacerRobot( uint8_t x , uint8_t y )
{
  char label[ 16 ];
  sprintf( label , "R x: %03d y: %03d" , x , y );
  LCD_DisplayStringLine( 84 , (uint8_t*) label );


  static uint8_t actualX = 0;
  static uint8_t actualY = 0;

  sensor_t dummy;

  subscription_t sensorDeplacement;
  sensorDeplacement.sensor = SENSOR_DEPLACEMENT_ROBOT;
  sensorDeplacement.value = 0;
  sensorDeplacement.queue = q_sensorsRobot;

  if( ( x != actualX ) || ( y != actualY ) )
  {
	  uint8_t xTimesToMove = 0;

	  actuators_t robotX;
	  if ( x < actualX )
	  {
		  xTimesToMove = actualX - x;
		  robotX.actuator = ACTUATOR_ROBOT_X_NEG;
	  }
	  else
	  {
		  xTimesToMove = x - actualX;
		  robotX.actuator = ACTUATOR_ROBOT_X_POS;
	  }

	  uint8_t yTimesToMove = 0;

	  actuators_t robotY;
	  if ( y < actualY )
	  {
		  yTimesToMove = actualY - y;
	  	  robotY.actuator = ACTUATOR_ROBOT_Y_NEG;
	  }
	  else
	  {
		  yTimesToMove = y - actualY;
	      robotY.actuator = ACTUATOR_ROBOT_Y_POS;
	  }


	  char label[ 22 ];
	  sprintf( label , "Move X: %03d Y: %03d" , xTimesToMove , yTimesToMove );
	  LCD_DisplayStringLine( 96 , (uint8_t*) label );

	  int i;
	  for( i = 0 ; i < xTimesToMove ; i++ )
	  {
		  robotX.value = 1;
		  xQueueSendToFront( q_actuators , (void * const)&robotX , portMAX_DELAY );
		  sensorDeplacement.value = 1;
		  xQueueSendToFront( q_newSubscriptions , (void * const)&sensorDeplacement , portMAX_DELAY );
		  xQueueReceive( q_sensorsRobot , (void * const)&dummy , portMAX_DELAY );
		  sensorDeplacement.value = 0;
		  xQueueSendToFront( q_newSubscriptions , (void * const)&sensorDeplacement , portMAX_DELAY );
		  xQueueReceive( q_sensorsRobot , (void * const)&dummy , portMAX_DELAY );
		  robotX.value = 0;
		  xQueueSendToFront( q_actuators , (void * const)&robotX , portMAX_DELAY );

		  vTaskDelay( 100 / portTICK_RATE_MS );
	  }

	  for( i = 0 ; i < yTimesToMove ; i++ )
	  {
	  	  robotY.value = 1;
	  	  xQueueSendToFront( q_actuators , (void * const)&robotY , portMAX_DELAY );
		  sensorDeplacement.value = 1;
		  xQueueSendToFront( q_newSubscriptions , (void * const)&sensorDeplacement , portMAX_DELAY );
		  xQueueReceive( q_sensorsRobot , (void * const)&dummy , portMAX_DELAY );
		  sensorDeplacement.value = 0;
	  	  xQueueSendToFront( q_newSubscriptions , (void * const)&sensorDeplacement , portMAX_DELAY );
	  	  xQueueReceive( q_sensorsRobot , (void * const)&dummy , portMAX_DELAY );
	  	  robotY.value = 0;
	  	  xQueueSendToFront( q_actuators , (void * const)&robotY , portMAX_DELAY );

		  vTaskDelay( 100 / portTICK_RATE_MS );
	  }

	  actualX = x;
	  actualY = y;
  }
}

void resetRobot( void )
{
	actuators_t robotX;
	robotX.actuator = ACTUATOR_ROBOT_X_NEG;

	actuators_t robotY;
	robotY.actuator = ACTUATOR_ROBOT_Y_NEG;

	actuators_t monter;
	monter.actuator = ACTUATOR_ROBOT_DESCENDRE;
	monter.value = 0;

	actuators_t pince;
	pince.actuator = ACTUATOR_ROBOT_PRENDRE;
	pince.value = 0;

	subscription_t sensorDeplacement;
	sensor_t dummy;

	sensorDeplacement.sensor = SENSOR_DEPLACEMENT_ROBOT;
	sensorDeplacement.value = 0;
	sensorDeplacement.queue = q_sensorsRobot;


	int i;

	for( i = 0 ; i < 3 ; i++ )
	{
		robotX.value = 1;
		xQueueSendToFront( q_actuators , (void * const)&robotX , portMAX_DELAY );
		xQueueSendToFront( q_newSubscriptions , (void * const)&sensorDeplacement , portMAX_DELAY );
        xQueueReceive( q_sensorsRobot , (void * const)&dummy , portMAX_DELAY );
		robotX.value = 0;
		xQueueSendToFront( q_actuators , (void * const)&robotX , portMAX_DELAY );

		vTaskDelay( 100 / portTICK_RATE_MS );
	}

	for( i = 0 ; i < 4 ; i++ )
	{
		robotY.value = 1;
		xQueueSendToFront( q_actuators , (void * const)&robotY , portMAX_DELAY );
		xQueueSendToFront( q_newSubscriptions , (void * const)&sensorDeplacement , portMAX_DELAY );
		xQueueReceive( q_sensorsRobot , (void * const)&dummy , portMAX_DELAY );
		robotY.value = 0;
		xQueueSendToFront( q_actuators , (void * const)&robotY , portMAX_DELAY );

		vTaskDelay( 100 / portTICK_RATE_MS );
	}

	xQueueSendToFront( q_actuators , (void * const)&monter , portMAX_DELAY );
	xQueueSendToFront( q_actuators , (void * const)&pince , portMAX_DELAY );
}

void prendrePiece( uint8_t prendre )
{
	actuators_t descendre;
	descendre.actuator = ACTUATOR_ROBOT_DESCENDRE;
	descendre.value = 1;

	actuators_t tenir;
	tenir.actuator = ACTUATOR_ROBOT_PRENDRE;
	tenir.value = prendre;

	sensor_t dummy;

	subscription_t sensorBas;
	sensorBas.sensor = SENSOR_ROBOT_BAS;
	sensorBas.value = 1;
	sensorBas.queue = q_sensorsRobot;

	subscription_t sensorHaut;
	sensorHaut.sensor = SENSOR_ROBOT_HAUT;
	sensorHaut.value = 1;
	sensorHaut.queue = q_sensorsRobot;

	subscription_t sensorPris;
	sensorPris.sensor = SENSOR_ROBOT_PIECE_PRISE;
	sensorPris.queue = q_sensorsRobot;

	if( prendre )
		sensorPris.value = 1;
	else
		sensorPris.value = 0;

	xQueueSendToFront( q_actuators , (void * const)&descendre , portMAX_DELAY );
	xQueueSendToFront( q_newSubscriptions , (void * const)&sensorBas , portMAX_DELAY );
	xQueueReceive( q_sensorsRobot , (void * const)&dummy , portMAX_DELAY );

	xQueueSendToFront( q_actuators , (void * const)&tenir , portMAX_DELAY );
	xQueueSendToFront( q_newSubscriptions , (void * const)&sensorPris , portMAX_DELAY );
	xQueueReceive( q_sensorsRobot , (void * const)&dummy , portMAX_DELAY );

	descendre.value = 0;

	xQueueSendToFront( q_actuators , (void * const)&descendre , portMAX_DELAY );
	xQueueSendToFront( q_newSubscriptions , (void * const)&sensorHaut , portMAX_DELAY );
	xQueueReceive( q_sensorsRobot , (void * const)&dummy , portMAX_DELAY );
}
