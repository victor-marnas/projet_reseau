#include "main.h"

#define max_step 8

typedef struct {
	uint16_t ID			: 11;
	uint8_t RTR			: 1;
	uint8_t dataLength	: 4;
	uint8_t data[ 8u ];
	uint8_t isValid		: 1;
	uint16_t			: 15;
}tCAN_msg;

static inline void initialize( tCAN_msg* msg )
{
	msg->ID = 0u;
	msg->RTR = 0u;
	msg->dataLength = 0u;
	msg->isValid = 0u;
}

static inline uint8_t getBit( uint8_t* octet, uint8_t* bitIndex )
{
	uint8_t output = 0u;

	if ( *bitIndex < 8u )
	{
		output = ( ( *octet & ( 0x01u << *bitIndex ) ) >> *bitIndex );
	}
	return( output );
}

tCAN_msg bitToMsg(uint8_t octet[ 17u ], uint8_t size)
{
	uint8_t octetIndex = 0u;
	uint8_t bitIndex = 7u;
	uint8_t previousBit = 0u;
	uint8_t consecutiveBitCount = 0u;
	int8_t step = 0;
	int8_t idIndex = 10;
	int8_t ctrlIndex = 5;

	tCAN_msg msg;

	initialize( &msg );

	while ( ( step < max_step ) && ( step != -1 ) )
	{
		uint8_t bit = getBit( &( octet[octetIndex] ), &bitIndex );

		bitIndex--;

		if ( 0u == bitIndex )
		{
			bitIndex = 7u;
			octetIndex++;
		}

		if ( step <= 5u ) //Bit stuffing uniquement avant CRC
		{
			if ( previousBit == bit )
			{
				consecutiveBitCount++;

				if ( consecutiveBitCount == 5u )
				{
					consecutiveBitCount = 0u;
					continue;
				}
			}
			else
			{
				consecutiveBitCount = 0u;
			}
		}

		switch( step )
		{
			case 0: // SOT
			{
				if ( 1u == bit )
				{
					step++;
				}
				else
				{
					step = -1;
				}

				break;
			}
			case 1: // ID
			{
				msg.ID += ( bit << idIndex );

				if (idIndex == 0u)
				{
					step++;
				}

				idIndex--;

				break;
			}
			case 2: // RTR (données ou requete)
			{
				msg.RTR = bit;

				step++;

				break;
			}
			case 3: // Zone de contrôle
			{
				if ( ctrlIndex < 4u )
				{
					msg.dataLength += ( bit << ctrlIndex );

					if ( ctrlIndex == 0u )
					{
						step++;
					}
				}

				ctrlIndex--;
				break;
			}
			case 4: // Champ de bits
			{
				break;
			}
			case 5: // Champ de CRC + 1 delimiteurs
			{
				break;
			}
			case 6: // Champ ack
			{
				break;
			}
			case 7: // EOT
			{
				break;
			}
			case 8: // Intertrame
			{
				break;
			}

		}

		previousBit = bit;
	}

	return( msg );
}
