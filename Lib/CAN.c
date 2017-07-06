#include "main.h"
#include "CAN.h"

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

static inline uint8_t getBit( uint8_t octet, uint8_t bitIndex )
{
	uint8_t output = 0u;

	if ( bitIndex < 8u )
	{
		output = ( ( octet & ( 0x01u << bitIndex ) ) >> bitIndex );
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
	uint8_t consecutiveBitCount = 1u;
	uint8_t eotBitCount = 0u;
	uint8_t intertrameBitCount = 0u;
	int8_t step = 0;
	int8_t idIndex = 10;
	int8_t ctrlIndex = 5;
	int8_t ack = -1;

	initialize( msg );

	while ( ( step < max_step ) && ( step != -1 ) )
	{
		uint8_t bit = getBit( octet[octetIndex], bitIndex );

		if ( 0u == bitIndex )
		{
			bitIndex = 7u;
			octetIndex++;
		}
		else
		{
			bitIndex--;
		}

		if ( step <= 5u ) //Bit stuffing jusqu'au CRC
		{
			if ( previousBit == bit )
			{
				consecutiveBitCount++;

				if ( 5u == consecutiveBitCount )
				{
					consecutiveBitCount = 1u;
					previousBit = (bit == 1u) ? 0u : 1u;

					continue;
				}
			}
			else
			{
				consecutiveBitCount = 1u;
			}
		}

		switch( step )
		{
			case 0: // SOT
			{
				if ( DOMINANT == bit )
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
				if ( RECESSIVE == bit ) {
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
					if ( RECESSIVE == bit )
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
				if ( RECESSIVE == bit )
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
				if ( RECESSIVE == bit )
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

void msgToBit( tCAN_msg* msg, uint8_t octet[ 17u ], uint8_t* size )
{
	if (msg != (tCAN_msg*)0)
	{
		if (msg->isValid)
		{
			uint8_t step = 0u;
			uint8_t consecutiveBitCount = 1u;
			uint8_t octetIndex = 0u;
			uint8_t bitIndex = 7u;
			uint8_t previousBit = 0u;
			uint8_t bit = 0u;
			uint8_t lengthIndex = 10u;
			uint8_t dataLengthIndex = 3u;
			uint8_t crcIndex = 14u;
			uint8_t eofIndex = 9u;

			uint8_t dataOctetIndex = msg->dataLength;
			uint8_t dataIndex = 7u;

			while ( step <= 11 ) //maxStep)
			{
				switch (step)
				{
					case 0: // SOT
					{
						bit = DOMINANT;

						step++;

						break;
					}
					case 1: // ID
					{
						bit = getBit( msg->ID, lengthIndex );

						if (lengthIndex == 0u)
						{
							step++;
						}
						else {
							lengthIndex--;
						}

						break;
					}
					case 2: // RTR
					{
						bit = msg->RTR;

						step++;

						break;
					}
					case 3: // R0
					{
						bit = DOMINANT;

						step++;

						break;
					}
					case 4: // R1
					{
						bit = DOMINANT;

						step++;

						break;
					}
					case 5: // DLC
					{
						bit = getBit( msg->dataLength, dataLengthIndex );

						if (dataLengthIndex == 0u)
						{
							step++;
						}
						else {
							dataLengthIndex--;
						}

						break;
					}
					case 6: // Data
					{
						bit = getBit( msg->data[dataOctetIndex], dataIndex );

						if (dataIndex == 0u)
						{
							dataIndex = 7u;

							if (dataOctetIndex == 0u)
							{
								step++;
							}
							else
							{
								dataOctetIndex--;
							}
						}
						else
						{
							dataIndex--;
						}

						break;
					}
					case 7: // CRC
					{
						bit = getBit( msg->crc, crcIndex );

						if (crcIndex == 0u)
						{
							step++;
						}
						else
						{
							crcIndex--;
						}

						break;
					}
					case 8: // Délimiteur
					{
						bit = RECESSIVE;

						break;
					}
					case 9: // ACK
					{
						bit = RECESSIVE;

						break;
					}
					case 10: // Délimiteur
					{
						bit = RECESSIVE;

						break;
					}
					case 11: // EOF + Intertrame
					{
						bit = RECESSIVE;

						if (eofIndex == 0u)
						{
							step++;
						}
						else
						{
							eofIndex--;
						}
						break;
					}
				}

				if ( step <= 7 ) // Ajout bitstuffing avant CRC
				{
					if (consecutiveBitCount == 5u)
					{
						previousBit = (bit == 0u) ? 1u : 0u;
						octet[octetIndex] += previousBit;

						consecutiveBitCount = 1u;
					}

					if (previousBit == bit)
					{
						consecutiveBitCount++;
					}
					else
					{
						consecutiveBitCount = 1u;
					}
				}

				octet[octetIndex] += (bit << bitIndex);

				if (bitIndex == 0)
				{
					bitIndex = 7u;
					octetIndex--;
					octet[octetIndex] = 0;
				}
				else
				{
					bitIndex--;
				}
			}
		}
	}
}
