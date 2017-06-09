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

  resetRobot();

  while( 1 )
  {
	  uint8_t type;

	  deplacerRobot( 0 , 0 );

	  xQueueReceive( q_pieceDispo , (uint8_t * const)type , portMAX_DELAY );

	  prendrePiece( 1 );

	  xQueueSendToFront( q_piecePrise , (void * const)&piecePrise , portMAX_DELAY);

	  deplacerRobot( t_cpt_type_a_placer[type] , type );
	  t_cpt_type_a_placer[type]--;

	  prendrePiece( 0 );

	  if ( !(t_cpt_type_a_placer[0] + t_cpt_type_a_placer[1] + t_cpt_type_a_placer[2]) )
	  {
		  uint8_t type;
		  for ( type = 0 ; type < 3 ; type++ )
			t_cpt_type_a_placer[ type ] = 3;

		  xQueueSendToFront(q_caissePleine , (void * const)&caissePleine , portMAX_DELAY );
	  }
  }
}

void deplacerRobot( uint8_t x , uint8_t y )
{
  static uint8_t actualX = 0;
  static uint8_t actualY = 0;

  sensor_t sensorDeplacement;
  sensorDeplacement.sensor = SENSOR_DEPLACEMENT_ROBOT;
  sensorDeplacement.value = 0;

  if(( x != actualX ) || ( y != actualY ))
  {
	  uint8_t xTimesToMove = 0;

	  actuators_t robotX;
	  if (x < actualX)
	  {
		  xTimesToMove = -(x - actualX);
		  robotX.actuator = ACTUATOR_ROBOT_X_NEG;
	  }
	  else
	  {
		  xTimesToMove = x - actualX;
		  robotX.actuator = ACTUATOR_ROBOT_X_POS;
	  }

	  uint8_t yTimesToMove = 0;

	  actuators_t robotY;
	  if (y < actualY)
	  {
		  yTimesToMove = -(y - actualY);
	  	  robotY.actuator = ACTUATOR_ROBOT_Y_NEG;
	  }
	  else
	  {
		  yTimesToMove = y - actualY;
	      robotY.actuator = ACTUATOR_ROBOT_Y_POS;
	  }

	  int i;
	  for( i = 0 ; i < xTimesToMove ; i++ )
	  {
		  robotX.value = 1;
		  xQueueSendToFront( q_actuators , (void * const)&robotX , portMAX_DELAY );
		  xQueueSendToFront( q_newSubscriptions , (void * const)&sensorDeplacement , portMAX_DELAY );
		  xQueueReceive( q_sensorsRobot , (void * const)NULL , portMAX_DELAY );
		  robotX.value = 0;
		  xQueueSendToFront( q_actuators , (void * const)&robotX , portMAX_DELAY );
	  }

	  for( i = 0 ; i < yTimesToMove ; i++ )
	  {
	  	  robotY.value = 1;
	  	  xQueueSendToFront( q_actuators , (void * const)&robotY , portMAX_DELAY );
	  	  xQueueSendToFront( q_newSubscriptions , (void * const)&sensorDeplacement , portMAX_DELAY );
	  	  xQueueReceive( q_sensorsRobot , (void * const)NULL , portMAX_DELAY );
	  	  robotY.value = 0;
	  	  xQueueSendToFront( q_actuators , (void * const)&robotY , portMAX_DELAY );
	  }
  }
}

void resetRobot( void )
{
	actuators_t robotX;
	robotX.actuator = ACTUATOR_ROBOT_X_NEG;

	actuators_t robotY;
	robotY.actuator = ACTUATOR_ROBOT_Y_NEG;

	sensor_t sensorDeplacement;
	sensorDeplacement.sensor = SENSOR_DEPLACEMENT_ROBOT;
	sensorDeplacement.value = 0;

	int i;

	for( i = 0 ; i < 4 ; i++ )
	{
		robotX.value = 1;
		xQueueSendToFront( q_actuators , (void * const)&robotX , portMAX_DELAY );
		xQueueSendToFront( q_newSubscriptions , (void * const)&sensorDeplacement , portMAX_DELAY );
        xQueueReceive( q_sensorsRobot , (void * const)NULL , portMAX_DELAY );
		robotX.value = 0;
		xQueueSendToFront( q_actuators , (void * const)&robotX , portMAX_DELAY );
	}

	for( i = 0 ; i < 3 ; i++ )
	{
		robotY.value = 1;
		xQueueSendToFront( q_actuators , (void * const)&robotY , portMAX_DELAY );
		xQueueSendToFront( q_newSubscriptions , (void * const)&sensorDeplacement , portMAX_DELAY );
		xQueueReceive( q_sensorsRobot , (void * const)NULL , portMAX_DELAY );
		robotY.value = 0;
		xQueueSendToFront( q_actuators , (void * const)&robotY , portMAX_DELAY );
	}
}

void prendrePiece( uint8_t prendre )
{
	actuators_t descendre;
	descendre.actuator = ACTUATOR_ROBOT_DESCENDRE;
	descendre.value = 1;

	actuators_t tenir;
	tenir.actuator = ACTUATOR_ROBOT_PRENDRE;
	tenir.value = prendre;

	sensor_t sensorBas;
	sensorBas.sensor = SENSOR_ROBOT_BAS;
	sensorBas.value = 1;

	sensor_t sensorPris;
	sensorPris.sensor = SENSOR_ROBOT_PIECE_PRISE;

	if( prendre )
		sensorPris.value = 1;
	else
		sensorPris.value = 0;

	xQueueSendToFront( q_actuators , (void * const)&descendre , portMAX_DELAY );
	xQueueSendToFront( q_newSubscriptions , (void * const)&sensorBas , portMAX_DELAY );
	xQueueReceive( q_sensorsRobot , (void * const)NULL , portMAX_DELAY );

	xQueueSendToFront( q_actuators , (void * const)&tenir , portMAX_DELAY );
	xQueueSendToFront( q_newSubscriptions , (void * const)&sensorPris , portMAX_DELAY );
	xQueueReceive( q_sensorsRobot , (void * const)NULL , portMAX_DELAY );

	descendre.value = 0;
	sensorBas.value = 0;

	xQueueSendToFront( q_actuators , (void * const)&descendre , portMAX_DELAY );
	xQueueSendToFront( q_newSubscriptions , (void * const)&sensorBas , portMAX_DELAY );
	xQueueReceive( q_sensorsRobot , (void * const)NULL , portMAX_DELAY );
}
