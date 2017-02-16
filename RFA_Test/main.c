	// Address must be set to 0 for the first device, and to 1 for the second one.
#define APP_ADDR_SRC              0x0003 //Routing (network address < 0x8000); Non-routing (network address >= 0x8000)
#define APP_CHANNEL               0x0f//
#define APP_PANID                 0x4184
#define APP_ENDPOINT              1

#define NWK_BUFFERS_AMOUNT                  5
#define NWK_DUPLICATE_REJECTION_TABLE_SIZE  10
#define NWK_DUPLICATE_REJECTION_TTL         500 // ms
#define NWK_ROUTE_TABLE_SIZE                20
#define NWK_ROUTE_DEFAULT_SCORE             3
#define NWK_ACK_WAIT_TIME                   500 // ms

#define NWK_ENABLE_ROUTING

#include "rfa_global.h"
/*- Declarations -----------------------------------------------------------*/

static void ProcessIncomingData (uint8_t * iData, uint8_t size);

/*- Types ------------------------------------------------------------------*/
typedef enum AppState_t
{
	APP_STATE_INITIAL,
	APP_STATE_IDLE,
	APP_STATE_WAIT_MSG_SEND
} AppState_t;

typedef struct AppMessage_t
{
	uint16_t	dstAddr;
	uint8_t		size;
	uint8_t		data[255];
} AppMessage_t;

/*- Variables --------------------------------------------------------------*/
static AppState_t		appState		= APP_STATE_INITIAL;
static NWK_DataReq_t	appNwkDataReq;
static AppMessage_t		appMessage;

static uint8_t			readyToSend = 0;

/*- Implementations --------------------------------------------------------*/
static void OnMsgSend (NWK_DataReq_t* req)
{
	appState = APP_STATE_IDLE;
	(void)req;
}

static void SendMsg (void)
{
	//Turn ON LED_TX
	PORTB ^= 0b01000000;

	appNwkDataReq.dstAddr		= appMessage.dstAddr;
	appNwkDataReq.dstEndpoint	= APP_ENDPOINT;
	appNwkDataReq.srcEndpoint	= APP_ENDPOINT;
	appNwkDataReq.options		= NWK_OPT_ACK_REQUEST;
	appNwkDataReq.data			= (uint8_t*)&appMessage;
	appNwkDataReq.size			= appMessage.size + 1;
	appNwkDataReq.confirm		= OnMsgSend;

	appState = APP_STATE_WAIT_MSG_SEND;

	NWK_DataReq(&appNwkDataReq);
}

static bool OnMsgRcvd (NWK_DataInd_t* ind)
{
	//Turn ON LED_RX
	PORTB ^= 0b00100000;

	appMessage.dstAddr	= 0x0002;
	appMessage.size		= ind->size;

	const uint8_t s = appMessage.size;

	for (uint8_t id = 0; id < s; ++id)
	{
		appMessage.data[id] = ind->data[id];
	}

	readyToSend = 1;

	return true;
}

//****************************************************************

static void APP_TaskHandler(void)
{
	switch (appState)
	{
		case APP_STATE_INITIAL:
		{
			//Send msg timer
			TIMSK1	|= (1 << OCIE1A);	//Enable timer OCIE1A interrupt
			TCNT1	= 0;				//Set timer counter initial value to 0
			TCCR1A	= 0;
			TCCR1B	= (1 << CS12) | (0 << CS11) | (1 << CS10);	// 1/1024 prescaler
			OCR1A	= 15625/10;		//OCR1A value (1024*(15625))/16000000 = 1.0000 sec

			//LED turn off timer
			TIMSK3	|= (1 << OCIE3A);	//Enable timer OCIE3A interrupt
			TCNT3	= 0;				//Set timer counter initial value to 0
			TCCR3A	= 0;
			TCCR3B	= (1 << CS32) | (0 << CS31) | (1 << CS30);	// 1/1024 prescaler
			OCR3A	= 15600/7;		//OCR1A value (1024*(15600/7))/16000000 = 0.1430 sec

			sei();

			// Network initialization
			NWK_SetAddr(APP_ADDR_SRC);
			NWK_SetPanId(APP_PANID);
			PHY_SetChannel(APP_CHANNEL);
			PHY_SetRxState(true);
			//PHY_SetTxPower();
			NWK_OpenEndpoint(APP_ENDPOINT, OnMsgRcvd);

			appState = APP_STATE_IDLE;
		} break;
		case APP_STATE_IDLE:
		{
			if (readyToSend)
			{
				SendMsg();
				readyToSend = 0;
			}
			//
		} break;
		case APP_STATE_WAIT_MSG_SEND:
		{
			//
		} break;
	}
}

int main(void)
{	
	DDRB  = 0b01100000;
	PORTB = 0b00000000;

	SYS_Init();

	while (1)
	{
		SYS_TaskHandler();
		APP_TaskHandler();
	}/**/
}

ISR(TIMER1_COMPA_vect)
{
	/*//Reset timer counter
	TCNT1 = 0;

	if (appState != APP_STATE_IDLE) return;

	appMessage.size = 3;
	appMessage.data[0] = 127;
	appMessage.data[1] = 128;
	appMessage.data[2] = 2;

	SendMsg();*/
}

ISR(TIMER3_COMPA_vect)
{
	//TCNT3 = 0;
	//PORTB = 0b00000000;
}
