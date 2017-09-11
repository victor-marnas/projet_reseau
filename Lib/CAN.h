#pragma once

#define max_step 8
#define RECESSIVE	( (uint8_t) 1u )
#define DOMINANT	( (uint8_t) 0u )

typedef enum { NO_ERR, BIT_STUFFING_ERR, CRC_ERR, LENGTH_ERR } eErrorCAN;

typedef struct {
	uint16_t ID			: 11u;
	uint8_t RTR			: 1u;
	uint8_t dataLength	: 4u;
	uint8_t data[ 8u ];
	uint16_t crc        : 15u;
	uint8_t isValid     : 1u;
	eErrorCAN error     : 8u;
}tCAN_msg;


void bitToMsg( uint8_t octet[ 17u ], uint8_t size, tCAN_msg* msg );
void msgToBit( tCAN_msg* msg, uint8_t octet[ 17u ], uint8_t* size );
void displayData( uint8_t octet[ 17u ] );
uint16_t crc( tCAN_msg* msg );
uint16_t can_crc_next( uint16_t crc, uint8_t data );
