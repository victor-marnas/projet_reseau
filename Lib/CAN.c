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
	uint8_t dataBitIndex = 7u;
	uint8_t crcIndex = 14u;
	uint8_t consecutiveBitCount = 0u;
	uint8_t eotBitCount = 0u;
	int8_t step = 0;
	int8_t idIndex = 10;
	int8_t ctrlIndex = 5;
	int8_t ack = -1;

	initialize( msg );

	while ( ( step < (max_step + 1) ) && ( step != -1 ) )
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
			}
			else if ( 5u == consecutiveBitCount )
			{
				consecutiveBitCount = 0u;

				previousBit = (bit == 1u) ? 0u : 1u;

				continue;
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
				msg->ID |= ( bit << idIndex );

				if ( 0u == idIndex )
				{
					step++;
				}

				idIndex--;

				break;
			}
			case 2: // RTR (donn�es ou requete)
			{
				msg->RTR = bit;

				step++;

				break;
			}
			case 3: // Zone de contr�le
			{
				if ( ctrlIndex < 4u )
				{
					msg->dataLength |= ( bit << ctrlIndex );

					if ( 0u == ctrlIndex )
					{
						dataIndex = msg->dataLength - 1u;

						step++;
					}
				}

				ctrlIndex--;

				break;
			}
			case 4: // Champ de bits
			{
				msg->data[ dataIndex ] |= ( bit << dataBitIndex );

				if ( 0u == dataBitIndex )
				{
					if ( 0u == dataIndex )
					{
						step++;
					}
					else
					{
						dataBitIndex = 8u;
						dataIndex--;
					}
				}

				dataBitIndex--;

				break;
			}
			case 5: // Champ de CRC
			{
				msg->crc |= ( bit << crcIndex );

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
				else
				{
					step = -1;
				}

				break;
			}
			case 7: // Champ ack + d�limiteur
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
					//step = -1;
					step++;
				}

				break;
			}
		}

		previousBit = bit;
	}

	if ( step == ( max_step + 1 ) )
	{
		msg->isValid = 1u;

		uint16_t calculatedCRC = crc( msg );
		if ( calculatedCRC != msg->crc )
		{
			msg->isValid = 0u;
		}
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
					case 8: // D�limiteur
					{
						bit = RECESSIVE;

						break;
					}
					case 9: // ACK
					{
						bit = RECESSIVE;

						break;
					}
					case 10: // D�limiteur
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

void displayData( uint8_t octet[ 17u ] )
{
	uint8_t msg[135];
	uint8_t index = 0;

	for (uint8_t octetIndex = 0u; octetIndex < 17u; ++octetIndex)
	{
		for (uint8_t bitIndex = 7u; bitIndex > 0u; --bitIndex)
		{
			msg[index] = getBit(octet[octetIndex], bitIndex) + '0';

			index++;
		}
	}

	msg[index] = '\0';

	LCD_DisplayStringLine( 36 , (uint8_t*) msg );
}

uint16_t crc( tCAN_msg* msg )
{
	uint16_t crc = 0u;

	if ( (tCAN_msg*)0 != msg )
	{
		uint8_t i = 0u;
		uint8_t oct10_ID = 0u;
		uint8_t oct9_ID = 0u;
		uint8_t oct8_ID = 0u;
		uint8_t oct10 = 0u;
		uint8_t oct9 = 0u;
		uint8_t oct8 = 0u;
		uint8_t oct7 = 0u;
		uint8_t oct6 = 0u;
		uint8_t oct5 = 0u;
		uint8_t oct4 = 0u;
		uint8_t oct3 = 0u;
		uint8_t oct2 = 0u;
		uint8_t oct1 = 0u;
		uint8_t oct0 = 0u;

		oct0 = msg->data[0];
		oct1 = msg->data[1];
		oct2 = msg->data[2];
		oct3 = msg->data[3];
		oct4 = msg->data[4];
		oct5 = msg->data[5];
		oct6 = msg->data[6];
		oct7 = msg->data[7];
		oct8_ID = 0b01u & msg->ID;
		oct8 |= oct8_ID << 1;
		oct8 |= msg->RTR;
		oct8 = oct8 << 6u;
		oct8 |= msg->dataLength;

		oct9_ID = (0b00111111110 & msg->ID) >> 1;
		oct9 = oct9_ID;
		oct10_ID = (0b11000000000 & msg->ID) >> 9;
		oct10 = oct10_ID;

		uint8_t data[ 11u ] = { oct10, oct9, oct8, oct7, oct6, oct5, oct4, oct3, oct2, oct1, oct0 };

		for ( i = 0u; i < 11u; i++ )
		{
			crc = can_crc_next( crc, data[ i ] );
		}
	}

	return( crc );
}

uint16_t can_crc_next( uint16_t crc, uint8_t data )
{
	uint8_t i = 0u;
	uint16_t crcReturn = crc;

	if ( 0u != data )
	{
		crcReturn ^= (uint16_t)data << 7u;

		for ( i = 0u; i < 8u; i++ )
		{
			crcReturn <<= 1u;

			if ( 0u != ( crcReturn & 0x8000u ) )
			{
				crcReturn ^= 0xc599u;
			}
		}
	}

	crcReturn &= 0x7fffu;

	return ( crcReturn );
}
