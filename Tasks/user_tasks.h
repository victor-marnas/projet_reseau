#pragma once

//#define SNIFFER

#ifdef SNIFFER
void vTaskSniffer( void *pvParameters );
void vTaskTransceiverRX( void *pvParameters );
#else
void vTaskTransceiverTX( void *pvParameters );
void vTaskInjection( void *pvParameters );
#endif

