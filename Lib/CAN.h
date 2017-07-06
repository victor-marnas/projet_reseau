#pragma once

#define max_step 9

typedef struct {
	uint16_t ID			: 11;
	uint8_t RTR			: 1;
	uint8_t dataLength	: 4;
	uint8_t data[ 8u ];
	uint16_t crc        : 15;
	uint8_t isValid		: 1;
}tCAN_msg;
