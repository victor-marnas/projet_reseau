#include "main.h"

void vTaskSniffer( void *pvParameters )
{
	portTickType xLastWakeTime = xTaskGetTickCount();
	while( 1 )
	{
		vTaskDelayUntil( &xLastWakeTime , 1000 / portTICK_RATE_MS );
	}
}
