#include "config.h"
#include "rfa_global.h"
#include "Containers/Containers.h"

typedef enum AppState_t
{
	APP_STATE_INITIAL,
	APP_STATE_IDLE,
	APP_STATE_WAIT_MSG_SEND
} AppState_t;

#pragma pack(push)
#pragma pack(1)

typedef struct UartMsgHeader_t
{
	uint8_t		size;		//Payload size. Max value = 255 - sizeof(UartMsgHeader_t)
	uint16_t	srcAddr;	//SRC address
	uint16_t	dstAddr;	//DST address
} UartMsgHeader_t;

typedef struct AppMessage_t
{
	UartMsgHeader_t header;
	uint8_t			data[255];
} AppMessage_t;

#pragma pack(pop)

static AppState_t		appState		= APP_STATE_INITIAL;

#if defined(MODE_TX)
	static NWK_DataReq_t	appNwkDataReq;
	static AppMessage_t		appMessage;
#endif

#define USART_BAUDRATE 57600
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

#if defined(MODE_TX)
	FIFO uart_rx_fifo;
#elif defined(MODE_RX)
	FIFO uart_tx_fifo;
#else
#	error Unknown MODE. Set MODE_RX or MODE_TX
#endif

void USART_Init()
{
	UBRR0H = (uint8_t)(BAUD_PRESCALE>>8);
	UBRR0L = (uint8_t) BAUD_PRESCALE;

	// Double async UART speed
	//UCSR0A = (1<<U2X0);

	// Enable receiver and transmitter. Disable iterrupt by RX
	UCSR0B =
#if defined(MODE_TX)
			(0 << TXEN0) |
			(1 << RXEN0) |
#elif defined(MODE_RX)
			(1 << TXEN0) |
			(0 << RXEN0) |
#else
#	error Unknown MODE. Set MODE_RX or MODE_TX
#endif
			(0 << RXCIE0);

	// Set frame format: 1 stop bit(USBS0), 8data(UCSZ00)
	//UCSR0C = (0<<USBS0)|(3<<UCSZ00);
	UCSR0C = (0 << UMSEL01) | (0 << UMSEL00)//Async UART
			|(0 << UPM01) |	(0 << UPM00)	//
			|(0 << USBS0)					//0 - one stop bit, 1 - two stop bits
			|(0 << UCSZ02) | (1 << UCSZ01) | (1 << UCSZ00);	//8 bit

#if defined(MODE_TX)
	FIFO_CLEAR(uart_rx_fifo);
#elif defined(MODE_RX)
	FIFO_CLEAR(uart_tx_fifo);
#else
#	error Unknown MODE. Set MODE_RX or MODE_TX
#endif
}

#if defined(MODE_RX)
//Process uart_tx_fifo
bool USART0_TX (void)
{
	//uart_tx_fifo is empty
	if (FIFO_IS_EMPTY(uart_tx_fifo))
	{
		return false;
	}

	//UART not ready to TX
	if (!(UCSR0A & (1 << UDRE0)))
	{
		return false;
	}

	UDR0 = FIFO_GET(uart_tx_fifo);
	FIFO_POP(uart_tx_fifo);

	return true;
}

static void WriteToUartTx(uint8_t* aPtr, uint8_t aSize)
{
	for (uint8_t id = 0; id < aSize; ++id)
	{

		uint8_t val = *aPtr++;

		if ((val == 0xFF) || (val == 0xFE))//Escape with 0xFE
		{
			// if(!FIFO_IS_FULL( uart_tx_fifo ))
				FIFO_PUSH(uart_tx_fifo, 0xFE);
		}

		// if(!FIFO_IS_FULL( uart_tx_fifo ))
			FIFO_PUSH(uart_tx_fifo, val);
	}
}

//static uint8_t testValue = 0;

static bool WirelessRX (NWK_DataInd_t* ind)
{
	uint8_t dataSize = ind->size + sizeof(UartMsgHeader_t);

	if (FIFO_FREE_SPACE(uart_tx_fifo) < (dataSize + 1))
		return true;

	//Fill header
	UartMsgHeader_t header;
	header.size		= ind->size;
	header.dstAddr	= ind->dstAddr;
	header.srcAddr	= ind->srcAddr;

	//Write start byte (0xFF)
	FIFO_PUSH(uart_tx_fifo, 0xFF);

	//Write header to uart_tx_fifo
	WriteToUartTx((uint8_t*)&header, sizeof(UartMsgHeader_t));
//

	// FIFO_PUSH(uart_tx_fifo, 0xEE);
	//Write payload to uart_tx_fifo
	WriteToUartTx(ind->data, ind->size);

	PORTB ^= 0b01000000;

	return true;
}
#endif//#if defined(MODE_RX)

#if defined(MODE_TX)
bool USART0_RX (void)
{
	//uart_rx_fifo is full
	if (FIFO_IS_FULL(uart_rx_fifo))
	{
		return false;
	}

	//UART not ready to RX
	if (!(UCSR0A & (1 << RXC0)))
	{
	   return false;
	}

	FIFO_PUSH(uart_rx_fifo, UDR0);

	return true;
}

static void OnMsgSend (NWK_DataReq_t* req)
{
	appState = APP_STATE_IDLE;
	(void)req;
}

static void SendMsg (void)
{
	appNwkDataReq.dstAddr		= appMessage.header.dstAddr;
	appNwkDataReq.dstEndpoint	= APP_ENDPOINT;
	appNwkDataReq.srcEndpoint	= APP_ENDPOINT;
	appNwkDataReq.options		= 0;//NWK_OPT_ACK_REQUEST;
	appNwkDataReq.data			= (uint8_t*)appMessage.data;
	appNwkDataReq.size			= appMessage.header.size;
	appNwkDataReq.confirm		= OnMsgSend;

	appState = APP_STATE_WAIT_MSG_SEND;

	NWK_DataReq(&appNwkDataReq);
}

#define RX_STAGE_WAIT_FOR_BEGIN		0
#define RX_STAGE_READ_HEADER		1
#define RX_STAGE_READ_BODY			2

uint8_t sRX_headerData[sizeof(UartMsgHeader_t)];
uint8_t sRX_headerDataSize	= 0;
uint8_t sRX_bodyData[255];
uint8_t sRX_bodyDataSize	= 0;
uint8_t sRX_Stage			= RX_STAGE_WAIT_FOR_BEGIN;
uint8_t sRX_isEscapeMode	= 0;

static void WirelessTX ()
{
	if (FIFO_COUNT(uart_rx_fifo) == 0)
	{
		return;
	}

	while (FIFO_COUNT(uart_rx_fifo) > 0)
	{
		uint8_t val = FIFO_GET(uart_rx_fifo);
		FIFO_POP(uart_rx_fifo);

		switch (sRX_Stage)
		{
			case RX_STAGE_WAIT_FOR_BEGIN:
			{
				if (val == 0xFF)
				{
					sRX_Stage			= RX_STAGE_READ_HEADER;
					sRX_headerDataSize	= 0;
					sRX_bodyDataSize	= 0;
					sRX_isEscapeMode	= 0;
				}
			} break;
			case RX_STAGE_READ_HEADER:
			{
				if (sRX_isEscapeMode)
				{
					sRX_isEscapeMode = 0;
				} else
				{
					if (val == 0xFE)//Escape byte
					{
						sRX_isEscapeMode = 1;
						break;
					} else if (val == 0xFF)//New message
					{
						sRX_Stage			= RX_STAGE_READ_HEADER;
						sRX_headerDataSize	= 0;
						sRX_bodyDataSize	= 0;
						sRX_isEscapeMode	= 0;
						break;
					}
				}

				sRX_headerData[sRX_headerDataSize++] = val;

				if (sRX_headerDataSize >= sizeof(UartMsgHeader_t))
				{
					UartMsgHeader_t* header = (UartMsgHeader_t*)sRX_headerData;

					if (header->size > 0)
					{
						sRX_Stage = RX_STAGE_READ_BODY;
						sRX_bodyDataSize = 0;
					} else
					{
						sRX_Stage = RX_STAGE_WAIT_FOR_BEGIN;
					}
				}
			} break;
			case RX_STAGE_READ_BODY:
			{
				if (sRX_isEscapeMode)
				{
					sRX_isEscapeMode = 0;
				} else
				{
					if (val == 0xFE)//Escape byte
					{
						sRX_isEscapeMode = 1;
						break;
					} else if (val == 0xFF)//New message
					{
						sRX_Stage			= RX_STAGE_READ_HEADER;
						sRX_headerDataSize	= 0;
						sRX_bodyDataSize	= 0;
						sRX_isEscapeMode	= 0;
						break;
					}					
				}

				sRX_bodyData[sRX_bodyDataSize] = val;
				sRX_bodyDataSize++;

				UartMsgHeader_t* header = (UartMsgHeader_t*)sRX_headerData;

				if (sRX_bodyDataSize >= header->size)
				{
					if (appState == APP_STATE_IDLE)
					{
						appMessage.header.dstAddr	= header->dstAddr;
						appMessage.header.srcAddr	= APP_ADDR_SRC;
						appMessage.header.size		= header->size;

						//Get data
						uint8_t* ptr = (uint8_t*)appMessage.data;
						uint8_t payloadSize = header->size;
						for (uint8_t id = 0; id < payloadSize; ++id)
						{
							*ptr++ = sRX_bodyData[id];
						}

						SendMsg();
						PORTB ^= 0b01000000;
					}

					sRX_Stage			= RX_STAGE_WAIT_FOR_BEGIN;
					sRX_headerDataSize	= 0;
					sRX_bodyDataSize	= 0;
					sRX_isEscapeMode	= 0;

					//FIFO_CLEAR(uart_rx_fifo);
				}
			} break;
		}
	}//while
}
#endif//#if defined(MODE_TX)

//****************************************************************

static void APP_TaskHandler(void)
{
	switch (appState)
	{
		case APP_STATE_INITIAL:
		{
			USART_Init();

			sei();

			// Network initialization
			NWK_SetAddr(APP_ADDR_SRC);
			NWK_SetPanId(APP_PANID);
			PHY_SetChannel(APP_CHANNEL);
			PHY_SetRxState(true);
			//PHY_SetTxPower();

#if defined(MODE_RX)
			NWK_OpenEndpoint(APP_ENDPOINT, WirelessRX);
#endif//#if defined(MODE_RX)

			appState = APP_STATE_IDLE;
		} break;
		case APP_STATE_IDLE:
		case APP_STATE_WAIT_MSG_SEND:
		{
#if defined(MODE_RX)
			USART0_TX();
#endif//#if defined(MODE_RX)

#if defined(MODE_TX)
			USART0_RX();
			WirelessTX();
#endif//#if defined(MODE_TX)
		} break;
	}
}

int main(void)
{
	SYS_Init();

	//LED
	DDRB  = 0b01000000;
	PORTB = 0b00000000;

	while (1)
	{
		SYS_TaskHandler();
		APP_TaskHandler();
	}
}
