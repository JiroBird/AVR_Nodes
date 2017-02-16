#include "SPIManager.h"

void	SPI_Init (SpiMode aMode, bool aEnableInterrupt)
{
	//http://maxembedded.com/2013/11/the-spi-of-the-avr/

	uint8_t isMaster = (aMode == SPI_MODE_MASTER);

	DDRB = (1 << 3);//DDRB 3 is output (MISO)
	SPCR = ((aEnableInterrupt ? 1 : 0) << SPIE) | (1 << SPE) | (0 << DORD) | (isMaster << MSTR) | (0 << CPOL) | (0 << CPHA) | (0 << SPR1) << (1 << SPR0);

	uint8_t _s = SPSR;
	(void)_s;

	uint8_t _d = SPDR;
	SPDR = 0;
	(void)_d;
}

uint8_t SPI_SendAndRcv (uint8_t aSndData)
{
	SPDR = aSndData;			//Load data into buffer
	while(!(SPSR & (1<<SPIF)));//Wait until transmission complete
	return(SPDR);				//Return received data
}

/*bool	SPI_TrySendAndRcv (uint8_t aSndData, uint8_t* aRcvData)
{
	//SPDR = aSndData;
	if (SPSR & (1<<SPIF))
	{
		*aRcvData = SPDR;
		SPDR = aSndData;
		return true;
	}

	return false;
}*/
