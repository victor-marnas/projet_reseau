#pragma once

// Include main headers
#include "stm32f4xx.h"

// Include BSP headers
#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_sdram.h"
#include "stm32f429i_discovery_ioe.h"

// Include FreeRTOS headers
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "queue.h"

// User defined headers
#include "user_tasks.h"

typedef struct{
  uint8_t       sensor  : 5;
  uint8_t       value   : 1;
  xQueueHandle  queue;
}subscription_t;

typedef struct{
  uint8_t       actuator        : 4;
  uint8_t       value           : 1;
}actuators_t;

typedef struct{
  uint8_t       sensor  : 5;
  uint8_t       value   : 1;
}sensor_t;

static const uint8_t NUMBER_OF_SENSORS = 12;
/// TODO
static const uint8_t ACTUATOR_TAPIS_ENTREE = 0;
static const uint8_t ACTUATOR_TAPIS_SORTIE = 1;
static const uint8_t ACTUATOR_ROBOT_X_POS = 2;
static const uint8_t ACTUATOR_ROBOT_X_NEG = 3;
static const uint8_t ACTUATOR_ROBOT_Y_NEG = 4;
static const uint8_t ACTUATOR_ROBOT_Y_POS = 5;
static const uint8_t ACTUATOR_ROBOT_DESCENDRE = 6;
static const uint8_t ACTUATOR_ROBOT_PRENDRE = 7;
static const uint8_t SENSOR_TYPE_S0 = 0;
static const uint8_t SENSOR_TYPE_S1 = 1;
static const uint8_t SENSOR_BOUT_TAPIS_ENTREE = 2;
static const uint8_t SENSOR_PALETTE = 3;
static const uint8_t SENSOR_ROBOT_Y0 = 4;
static const uint8_t SENSOR_DEPLACEMENT_ROBOT = 5;
static const uint8_t SENSOR_ROBOT_HAUT = 6;
static const uint8_t SENSOR_ROBOT_BAS = 7;
static const uint8_t SENSOR_ROBOT_PIECE_PRISE = 8;

// Queues
xQueueHandle q_newSubscriptions;
xQueueHandle q_actuators;
xQueueHandle q_sensorsTapisSortie;
xQueueHandle q_caisseArrivee;
xQueueHandle q_caissePleine;
xQueueHandle q_sensorsTapisEntree;
xQueueHandle q_pieceDispo;
xQueueHandle q_piecePrise;
xQueueHandle q_sensorsRobot;

// Semaphores
xSemaphoreHandle sem_I2C_BUS;
