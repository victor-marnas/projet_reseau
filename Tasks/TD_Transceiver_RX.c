#include "main.h"

void vTaskTransceiverRX( void *pvParameters )
{

  while( 1 )
  {
	STM_EVAL_LEDOn( LED4 );

	STM_EVAL_LEDOff( LED4 );
  }
}
