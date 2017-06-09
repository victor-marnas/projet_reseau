
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

// Main program
int main( void )
{
    // Init board LEDS and Blue Push Button

    STM_EVAL_LEDInit(LED3);
    STM_EVAL_LEDInit(LED4);
    STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_GPIO);

    // Start IO Extenders (Touchpad & PCF IO Extenders)

    IOE_Config();

    // Launch FreeRTOS Trace recording
    vTraceInitTraceData();
    uiTraceStart();

    // Initialize resources management variable

    user_event_channel = xTraceOpenLabel("UEV");

    // Start the Scheduler

    vTaskStartScheduler();

    while(1)
    {
    	// The program should never be here...
    }
}
