#pragma once

#define max_step 9
#define RECESSIVE	( (uint8_t) 1u )
#define DOMINANT	( (uint8_t) 0u )

typedef struct {
	uint16_t ID			: 11;
	uint8_t RTR			: 1;
	uint8_t dataLength	: 4;
	uint8_t data[ 8u ];
	uint16_t crc        : 15;
	uint8_t isValid		: 1;
}tCAN_msg;


void bitToMsg( uint8_t octet[ 17u ], uint8_t size, tCAN_msg* msg );
void msgToBit( tCAN_msg* msg, uint8_t octet[ 17u ], uint8_t* size );
