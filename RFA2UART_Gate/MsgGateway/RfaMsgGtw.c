#include "RfaMsgGtw.h"

#define SPI_2_RFA_STREAM_STATUS_IDLE		0x01
#define SPI_2_RFA_STREAM_STATUS_READ_DATA	0x02

void	RfaMsgGtw_AddToQueue (Queue* aQueue, RfaMsgGtw_PacketHeader* aHeader, const uint8_t* aData)
{
	if (Queue_PushBack(aQueue, NULL, 0) != RES_OK)
	{
		//Drop packet
		return;
	}

	uint8_t* beginPtr = (uint8_t*)Queue_GetBack(aQueue);
	if ((beginPtr == NULL) || (aHeader->size > RFA_MSG_GTW_MAX_DATA_SIZE_BUFFER))
	{
		//Drop packet
		return;
	}

	memcpy(beginPtr, aHeader, sizeof(RfaMsgGtw_PacketHeader));
	memcpy(beginPtr + sizeof(RfaMsgGtw_PacketHeader), aData, aHeader->size);
}

Result	RfaMsgGtw_Init (RfaMsgGtwDesc* aDesc, uint8_t aPacketsQueueSize)
{
	//Init SPI queue
	Result res = Queue_Init(&aDesc->spiQueue, aPacketsQueueSize, sizeof(RfaMsgGtw_PacketHeader) + RFA_MSG_GTW_MAX_DATA_SIZE_BUFFER);
	if (res != RES_OK) return res;

	res = Queue_Init(&aDesc->rfaQueue, aPacketsQueueSize, sizeof(RfaMsgGtw_PacketHeader) + RFA_MSG_GTW_MAX_DATA_SIZE_BUFFER);
	if (res != RES_OK) return res;

	return RES_OK;
}

void	RfaMsgGtw_AddToQueueSPI (RfaMsgGtwDesc* aDesc, RfaMsgGtw_PacketHeader* aHeader, const uint8_t* aData)
{
	RfaMsgGtw_AddToQueue(&aDesc->spiQueue, aHeader, aData);
}

void	RfaMsgGtw_AddToQueueRFA (RfaMsgGtwDesc* aDesc, RfaMsgGtw_PacketHeader* aHeader, const uint8_t* aData)
{
	RfaMsgGtw_AddToQueue(&aDesc->rfaQueue, aHeader, aData);
}

uint8_t	RfaMsgGtw_ReadNextByteForSPI (RfaMsgGtwDesc* aDesc)
{
	static RfaMsgGtw_PacketHeader*	sHeader			= NULL;
	static uint8_t*					sDataPtr		= 0;
	static uint8_t					sDataSize		= 0;
	static uint8_t					sIsFirstByte	= 0;
	uint8_t							res				= 0;

	if (sHeader == NULL)
	{
		void* queueDataPtr = Queue_GetFront(&aDesc->spiQueue);

		if (queueDataPtr == NULL) return res;

		sDataPtr	= (uint8_t*)queueDataPtr;
		sHeader		= (RfaMsgGtw_PacketHeader*)queueDataPtr;
		sDataSize	= sHeader->size + sizeof(RfaMsgGtw_PacketHeader);
		sIsFirstByte= 1;
	}

	if (sIsFirstByte) res = sDataSize;
	else res = *sDataPtr;


	if (sIsFirstByte > 0)
	{
		sIsFirstByte = 0;
	} else
	{
		sDataPtr++;
		sDataSize--;

		if (sDataSize == 0)
		{
			sHeader = NULL;
			Queue_PopFront(&aDesc->spiQueue);
		}
	}

	return res;
}

void	RfaMsgGtw_WriteNextByteForRFA (RfaMsgGtwDesc* aDesc, uint8_t aData)
{
	static uint8_t sStatus = SPI_2_RFA_STREAM_STATUS_IDLE;
	static uint8_t sData[255];
	static uint8_t sDataSize = 0;
	static uint8_t sReadSize = 0;

	if (sStatus == SPI_2_RFA_STREAM_STATUS_IDLE)
	{
		if (aData > 0)
		{
			sStatus		= SPI_2_RFA_STREAM_STATUS_READ_DATA;
			sDataSize	= aData;
			sReadSize	= 0;
		} else
		{
			//Drop byte
			return;
		}
	} else if (sStatus == SPI_2_RFA_STREAM_STATUS_READ_DATA)
	{
		sData[sReadSize] = aData;
		sReadSize++;

		if (sReadSize >= sDataSize)
		{
			sStatus = SPI_2_RFA_STREAM_STATUS_IDLE;

			if (sReadSize < sizeof(RfaMsgGtw_PacketHeader))
			{
				//Drop packet
				return;
			}

			RfaMsgGtw_PacketHeader* header = (RfaMsgGtw_PacketHeader*)&sData[0];
			if (header->size != (sReadSize - sizeof(RfaMsgGtw_PacketHeader)))
			{
				//Drop packet
				return;
			}

			if (header->size == 0)
			{
				//Drop packet
				return;
			}

			//
			RfaMsgGtw_AddToQueueRFA(aDesc, header, &sData[sizeof(RfaMsgGtw_PacketHeader)]);
		}
	}
}
