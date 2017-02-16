#include "Queue.h"
/*
Result	Queue_Init (Queue* aQueue, uint8_t aMaxCount, uint8_t aElementSize)
{
	aQueue->maxCount		= aMaxCount;
	aQueue->maxCountM1		= aMaxCount - 1;
	aQueue->elementsCount	= 0;
	aQueue->elementSize		= aElementSize;

	//Allocate memory
	uint16_t dataSize = aMaxCount*aElementSize;
	aQueue->elements = (uint8_t*)malloc(dataSize);
	if (aQueue->elements == NULL) return RES_FAILED;

	memset(aQueue->elements, 0, dataSize);

	aQueue->beginId		= 0;
	aQueue->endId		= 0;

	return RES_OK;
}

void	Queue_Destroy (Queue* aQueue)
{
	if (aQueue->elements != NULL)
	{
		free(aQueue->elements);
		aQueue->elements = NULL;
	}

	aQueue->maxCount		= 0;
	aQueue->maxCountM1		= 0;
	aQueue->elementsCount	= 0;
	aQueue->elementSize		= 0;
	aQueue->beginId			= 0;
	aQueue->endId			= 0;
}

Result	Queue_PushBack (Queue* aQueue, const void* aData, uint8_t aSize)
{
	if (aQueue->elementsCount >= aQueue->maxCount) return RES_FAILED;
	if ((aData != NULL) && (aSize > aQueue->elementSize)) return RES_FAILED;

	if (aQueue->elementsCount == 0)
	{
		aQueue->beginId	= 0;
		aQueue->endId	= 0;
	} else if (aQueue->endId < aQueue->maxCountM1)
	{
		aQueue->endId++;
	} else
	{
		aQueue->endId = 0;
	}

	aQueue->elementsCount++;

	if (aData != NULL)
	{
		memcpy(Queue_GetBack(aQueue), aData, aSize);
	}

	return RES_OK;
}

void Queue_PopFront (Queue* aQueue)
{
	if (aQueue->elementsCount == 0) return;

	if (aQueue->beginId < aQueue->maxCountM1)
	{
		aQueue->beginId++;
	} else
	{
		aQueue->beginId = 0;
	}

	aQueue->elementsCount--;
}

void*	Queue_GetFront (Queue* aQueue)
{
	if (aQueue->elementsCount == 0) return NULL;
	return aQueue->elements + aQueue->beginId*aQueue->elementSize;
}

void*	Queue_GetBack (Queue* aQueue)
{
	if (aQueue->elementsCount == 0) return NULL;
	return aQueue->elements + aQueue->endId*aQueue->elementSize;
}
*/
