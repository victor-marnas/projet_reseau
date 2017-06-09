#pragma once

#define TASK_TAPIS_ENTREE_PRIORITY 1
#define TASK_ROBOT_PRIORITY 2
#define TASK_TAPIS_SORTIE_PRIORITY 3
#define TASK_SENSORS_PRIORITY 4
#define TASK_ACTUATORS_PRIORITY 5

static const uint8_t SENSORS_QUEUE_LENGTH = 5;
static const uint8_t ACTUATORS_QUEUE_LENGTH = 5;

static const uint16_t SENSORS_UPDATE_PERIOD = 50; // millis
static const uint16_t ACTUATORS_UPDATE_PERIOD = 100; // millis

void vTaskAcquisitionCapteur( void *pvParameters );
void ReadSensors( uint16_t * sensors );

void vTaskEcritureActionneur( void *pvParameters );
void WriteActuators( uint8_t * actuators );

void vTaskTapisSortie( void *pvParameters );
void DeplacerTapisSortie( uint8_t etat );

void vTaskTapisEntree( void *pvParameters );
void DeplacerTapisEntree( uint8_t etat );

void vTaskRobot( void *pvParameters );
void deplacerRobot( uint8_t x , uint8_t y );
void resetRobot( void );
void prendrePiece( uint8_t prendre );
