#include "main.h"

#define max_step 9

typedef struct {
	uint16_t ID			: 11;
	uint8_t RTR			: 1;
	uint8_t dataLength	: 4;
	uint8_t data[ 8u ];
	uint16_t crc        : 15;
	uint8_t isValid		: 1;
}tCAN_msg;

static inline void initialize( tCAN_msg* msg )
{
	msg->ID = 0u;
	msg->RTR = 0u;
	msg->dataLength = 0u;

	uint8_t i = 0u;
	for( i = 0u; i < 8u; msg->data[ i++ ] = 0u );

	msg->crc = 0u;
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

void bitToMsg( uint8_t octet[ 17u ], uint8_t size, tCAN_msg* msg )
{
	uint8_t octetIndex = 0u;
	uint8_t bitIndex = 7u;
	uint8_t previousBit = 0u;
	uint8_t dataIndex = 0u;
	uint8_t crcIndex = 15u;
	uint8_t consecutiveBitCount = 0u;
	uint8_t eotBitCount = 0u;
	uint8_t intertrameBitCount = 0u;
	int8_t step = 0;
	int8_t idIndex = 10;
	int8_t ctrlIndex = 5;
	int8_t ack = -1;

	initialize( msg );

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

				if ( 5u == consecutiveBitCount )
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
				msg->ID += ( bit << idIndex );

				if ( 0u == idIndex )
				{
					step++;
				}

				idIndex--;

				break;
			}
			case 2: // RTR (données ou requete)
			{
				msg->RTR = bit;

				step++;

				break;
			}
			case 3: // Zone de contrôle
			{
				if ( ctrlIndex < 4u )
				{
					msg->dataLength += ( bit << ctrlIndex );

					if ( 0u == ctrlIndex )
					{
						dataIndex = msg->dataLength;

						step++;
					}
				}

				ctrlIndex--;

				break;
			}
			case 4: // Champ de bits
			{
				if ( dataIndex > 0u )
				{
					msg->data[ dataIndex ] += ( bit << bitIndex );

					if ( 0u == bitIndex )
					{
						if ( 0u == dataIndex )
						{
							step++;
						}
						else
						{
							dataIndex--;
						}
					}
				}

				break;
			}
			case 5: // Champ de CRC
			{
				msg->crc += ( bit << crcIndex );

				if ( 0u == crcIndex )
				{
					step++;
				}

				crcIndex--;

				break;
			}
			case 6: // delimiteur
			{
				if (bit == 0) {
					step++;
				}
				else {
					step = -1;
				}

				break;
			}
			case 7: // Champ ack + délimiteur
			{
				if (ack == -1)
				{
					ack = bit;
				}
				else
				{
					if (bit == 0)
					{
						step++;
					}
					else
					{
						step = -1;
					}
				}

				break;
			}
			case 8: // EOT
			{
				if (bit == 0)
				{
					eotBitCount++;

					if (eotBitCount == 7)
					{
						step++;
					}
				}
				else
				{
					step = -1;
				}

				break;
			}
			case 9: // Intertrame
			{
				if ( 0u == bit )
				{
					intertrameBitCount++;

					if ( 3u == intertrameBitCount )
					{
						step++;
					}
				}
				else
				{
					step = -1;
				}

				break;
			}

		}

		previousBit = bit;
	}

	if ( step == ( max_step + 1 ) )
	{
		msg->isValid = 1u;
	}
}
