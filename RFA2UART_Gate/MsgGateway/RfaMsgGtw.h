#pragma once

#include "../rfa_global.h"
#include "../Containers/Containers.h"

#define RFA_MSG_GTW_MAX_DATA_SIZE_BUFFER	32
#define RFA_MSG_GTW_PROTOCOL_VERSION		0x01

#pragma pack(push)
#pragma pack(1)

typedef struct
{
	uint16_t	dstAddr;	//DST address
	uint16_t	srcAddr;	//SRC address
	uint8_t		size;		//
	uint8_t		chSum;		//?
} RfaMsgGtw_PacketHeader;

typedef struct
{
	Queue		spiQueue;	//queue of RfaMsgGtw_PacketHeader+payload
	Queue		rfaQueue;	//queue of RfaMsgGtw_PacketHeader+payload
} RfaMsgGtwDesc;

#pragma pack(pop)

Result		RfaMsgGtw_Init						(RfaMsgGtwDesc* aDesc, uint8_t aPacketsQueueSize);
void		RfaMsgGtw_AddToQueueSPI				(RfaMsgGtwDesc* aDesc, RfaMsgGtw_PacketHeader* aHeader, const uint8_t* aData);
void		RfaMsgGtw_AddToQueueRFA				(RfaMsgGtwDesc* aDesc, RfaMsgGtw_PacketHeader* aHeader, const uint8_t* aData);

uint8_t		RfaMsgGtw_ReadNextByteForSPI		(RfaMsgGtwDesc* aDesc);
void		RfaMsgGtw_WriteNextByteForRFA		(RfaMsgGtwDesc* aDesc, uint8_t aData);
