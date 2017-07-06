#pragma once

// Include main headers
#include "stm32f4xx.h"
#include "stm32f4xx_tim.h"

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

#include "user_tasks.h"
#include "deviceInit.h"
#include "CAN.h"


xQueueHandle q_rxBits;
