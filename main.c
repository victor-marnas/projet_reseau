
/*************************************************************************************************
 * 					ITS-PLC Startup Project
 *
 * 					(c) Polytech Montpellier - MEA
 *
 * 					20/09/2015 - Laurent Latorre
 *
 * 					FreeRTOS getting started project to work with ITS-PLC Simulator
 *
 * 					Creates 3 asynchronous tasks :
 *
 * 					- Graphical User Interface (TouchScreen and I/O extender interface)
 * 					- Task1 : Actuator A00 toggle
 * 					- Task2 : Actuator A01 copies the state of Sensor S00
 *
 *************************************************************************************************/


// Local defines

#define	PAD_WIDTH	56
#define PAD_HEIGHT	36
#define PAD_MARGIN	3

#define DBG_OFFLINE	0
#define	DBG_ONLINE	1

#include "main.h"

// Trace Labels
traceLabel user_event_channel;


// Global variables
static	uint16_t	   its_sensors;			// 16 bits representing sensors states from ITS-PLC
static	uint16_t	   forced_sensors;		// 16 bits representing sensors states from User Interface
static	uint16_t	   sensors;				// Bitwise OR between the two above
static	uint16_t	   actuators;			// 16 bits representing actuators states

static  uint8_t		   dbg_mode;			// ONLINE/OFFLINE upon detection of the I/O Extender on boot


// Main program
int main( void )
{
    // Init board LEDS and Blue Push Button

    STM_EVAL_LEDInit(LED3);
    STM_EVAL_LEDInit(LED4);
    STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_GPIO);

    // Start IO Extenders (Touchpad & PCF IO Extenders)

    IOE_Config();

    // Test if ITS-PLC I/O Extender is connected

    if (I2C_WritePCFRegister(PCF_B_WRITE_ADDR, (int8_t)0x00))
    {
      // I/O Extender found, ONLINE mode -> Green LED
      dbg_mode = DBG_ONLINE;
      STM_EVAL_LEDOn(LED3);
      STM_EVAL_LEDOff(LED4);
    }
    else
    {
      // I/O Extender not found, OFFLINE mode -> Red LED
      dbg_mode = DBG_OFFLINE;
      STM_EVAL_LEDOff(LED3);
      STM_EVAL_LEDOn(LED4);
    }

    // Init sensors & actuators variables
    actuators = 0x0000;
    forced_sensors = 0x000;

    // Launch FreeRTOS Trace recording
    vTraceInitTraceData();
    uiTraceStart();

    // Initialize resources management variable
    q_newSubscriptions = xQueueCreate( SENSORS_QUEUE_LENGTH , sizeof( subscription_t ) );
    if ( q_newSubscriptions == NULL )
    {
        STM_EVAL_LEDOn(LED4);
    }

    q_actuators = xQueueCreate( ACTUATORS_QUEUE_LENGTH , sizeof( actuators_t ) );
    if ( q_actuators == NULL )
      {
          STM_EVAL_LEDOn(LED4);
      }

    q_sensorsTapisSortie = xQueueCreate( 1 , sizeof( sensor_t ) );
    if ( q_sensorsTapisSortie == NULL )
      {
          STM_EVAL_LEDOn(LED4);
      }

    q_caisseArrivee = xQueueCreate( 1 , sizeof( uint8_t ) );
    if ( q_caisseArrivee == NULL )
      {
          STM_EVAL_LEDOn(LED4);
      }

    q_caissePleine = xQueueCreate( 1 , sizeof( uint8_t ) );
    if ( q_caissePleine == NULL )
      {
          STM_EVAL_LEDOn(LED4);
      }

    q_sensorsTapisEntree = xQueueCreate( 2 , sizeof( sensor_t ) );
    if ( q_sensorsTapisEntree == NULL )
      {
          STM_EVAL_LEDOn(LED4);
      }

    q_pieceDispo = xQueueCreate( 1 , sizeof( uint8_t ) );
    if ( q_pieceDispo == NULL )
      {
          STM_EVAL_LEDOn(LED4);
      }

    q_piecePrise = xQueueCreate( 1 , sizeof( uint8_t ) );
   if ( q_piecePrise == NULL )
     {
         STM_EVAL_LEDOn(LED4);
     }

   q_sensorsRobot = xQueueCreate( 1 , sizeof( sensor_t ) );
   if ( q_sensorsRobot == NULL )
     {
         STM_EVAL_LEDOn(LED4);
     }

    sem_I2C_BUS = xSemaphoreCreateMutex();
    if ( sem_I2C_BUS == NULL )
      {
          STM_EVAL_LEDOn(LED4);
      }

    // Create Tasks

    xTaskCreate( vTaskAcquisitionCapteur , "Task_Capteurs" , 128 , NULL , TASK_SENSORS_PRIORITY , NULL );        // 128 bytes stack, priority 1
    xTaskCreate( vTaskEcritureActionneur , "Task_Actionneurs" , 128 , NULL , TASK_ACTUATORS_PRIORITY , NULL );        // 128 bytes stack, priority 1
    xTaskCreate( vTaskTapisSortie , "Task_TapisSortie" , 128 , NULL , TASK_TAPIS_SORTIE_PRIORITY , NULL );        // 128 bytes stack, priority 1
    xTaskCreate( vTaskTapisEntree , "Task_TapisEntree" , 128 , NULL , TASK_TAPIS_ENTREE_PRIORITY , NULL );        // 128 bytes stack, priority 1
    xTaskCreate( vTaskRobot , "Task_Robot" , 128 , NULL , TASK_ROBOT_PRIORITY , NULL );        // 128 bytes stack, priority 1


    user_event_channel = xTraceOpenLabel("UEV");

    // Start the Scheduler

    vTaskStartScheduler();

    while(1)
    {
    	// The program should never be here...
    }
}
