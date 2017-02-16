#pragma once

#include "../rfa_global.h"

//http://chipenable.ru/index.php/programming-avr/item/138-avr-spi-module-part2.html

typedef enum
{
	SPI_MODE_SLAVE = 0x0U,
	SPI_MODE_MASTER
} SpiMode;

void	SPI_Init			(SpiMode aMode, bool aEnableInterrupt);
uint8_t	SPI_SendAndRcv		(uint8_t aSndData);
//bool	SPI_TrySendAndRcv	(uint8_t aSndData, uint8_t* aRcvData);
