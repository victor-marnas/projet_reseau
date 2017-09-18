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
	msg->error = NO_ERR;
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
				if (previousBit == bit) {
					msg->error = BIT_STUFFING_ERR;
				}
				else {
					consecutiveBitCount = 0u;

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
				msg->ID |= ( bit << idIndex );

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
					msg->dataLength |= ( bit << ctrlIndex );

					if ( 0u == ctrlIndex )
					{
						dataIndex = msg->dataLength - 1u;

						if (msg->dataLength == 0u) {
							step += 2;
						}
						else {
							step++;
						}
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
		uint16_t calculatedCRC = crc( msg );

		if ( calculatedCRC != msg->crc )
		{
			msg->error = CRC_ERR;
		}
		else {
			msg->isValid = 1u;
		}
	}
}

void msgToBit( tCAN_msg* msg, uint8_t octet[ 17u ], uint8_t* size )
{
	if (msg != (tCAN_msg*)0)
	{
		if (msg->isValid == 1u)
		{
			uint8_t step = 0u;
			uint8_t consecutiveBitCount = 0u;
			uint8_t octetIndex = 0u;
			uint8_t bitIndex = 7u;
			uint8_t previousBit = 0u;
			uint8_t bit = 0u;
			uint8_t lengthIndex = 10u;
			uint8_t dataLengthIndex = 3u;
			uint8_t crcIndex = 14u;
			uint8_t eofIndex = 9u;

			uint8_t dataOctetIndex = 0u;
			if ( msg->dataLength > 0u )
			{
				dataOctetIndex = msg->dataLength - 1u;
			}
			uint8_t dataIndex = 7u;

			*size = 0u;

			while ( step <= 11 ) //maxStep)
			{
				( *size ) = ( *size ) + 1u;
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
						else
						{
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

						step++;

						break;
					}
					case 9: // ACK
					{
						bit = RECESSIVE;

						step++;

						break;
					}
					case 10: // Délimiteur
					{
						bit = RECESSIVE;

						step++;

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

				if ( step <= 7u ) // Ajout bitstuffing avant CRC
				{
					/*
					if (consecutiveBitCount == 5u)
					{

						( *size ) = ( *size ) + 1u;
						//previousBit = (bit == 0u) ? 1u : 0u;
						//octet[octetIndex] |= previousBit;
						previousBit = (previousBit == 0u) ? 1u : 0u;
						octet[octetIndex] |= (previousBit << bitIndex);

						if (bitIndex == 0u) {
							bitIndex = 7u;
							octetIndex++;
							octet[octetIndex] = 0;
						}
						else
						{
							bitIndex--;
						}

						consecutiveBitCount = 1u;
					}
					else
					{
						if (previousBit == bit)
						{
							consecutiveBitCount++;
						}
						else
						{
							consecutiveBitCount = 1u;
						}

						previousBit = bit;
					}
*/
					if (consecutiveBitCount == 5u)
					{
						( *size ) = ( *size ) + 1u;


						//previousBit = (bit == 0u) ? 1u : 0u;
						//octet[octetIndex] |= previousBit;

						previousBit = (previousBit == 0u) ? 1u : 0u;
						octet[octetIndex] |= (previousBit << bitIndex);

						if (bitIndex == 0u) {
							bitIndex = 7u;
							octetIndex++;
							octet[octetIndex] = 0;
						}
						else
						{
							bitIndex--;
						}

						consecutiveBitCount = 1u;
					}
					else
					{
						if (previousBit == bit)
						{
							consecutiveBitCount++;
						}
						else
						{
							consecutiveBitCount = 1u;
						}

						previousBit = bit;
					}

				}

				octet[octetIndex] |= (bit << bitIndex);

				if (bitIndex == 0)
				{
					bitIndex = 7u;
					octetIndex++;
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

uint16_t crc( tCAN_msg* msg )
{
	uint16_t crc = 0u;

	if ( (tCAN_msg*)0 != msg )
	{
		uint8_t i = 0u;
		uint8_t oct10 = 0u;
		uint8_t oct9 = 0u;
		uint8_t oct8 = 0u;
		uint8_t oct7 = msg->data[7];
		uint8_t oct6 = msg->data[6];
		uint8_t oct5 = msg->data[5];
		uint8_t oct4 = msg->data[4];
		uint8_t oct3 = msg->data[3];
		uint8_t oct2 = msg->data[2];
		uint8_t oct1 = msg->data[1];
		uint8_t oct0 = msg->data[0];

		oct8 = 0b01u & msg->ID;
		oct8 = oct8 << 1;
		oct8 |= msg->RTR;
		oct8 = oct8 << 6u;
		oct8 |= msg->dataLength;

		oct9 = (0b00111111110u & msg->ID) >> 1;
		oct10 = (0b11000000000u & msg->ID) >> 9;

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

	crcReturn ^= (uint16_t)data << 7u;

	for ( i = 0u; i < 8u; i++ )
	{
		crcReturn <<= 1u;

		if ( 0u != ( crcReturn & 0x8000u ) )
		{
			crcReturn ^= 0x4599u;
		}
	}

	crcReturn &= 0x7fffu;

	return ( crcReturn );
}
