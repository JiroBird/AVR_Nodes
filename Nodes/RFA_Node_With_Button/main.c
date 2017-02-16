	// Address must be set to 0 for the first device, and to 1 for the second one.
#define APP_ADDR_SRC              0x8008 //Routing (network address < 0x8000); Non-routing (network address >= 0x8000)
#define APP_CHANNEL               0x0F//
#define APP_PANID                 0x4184
#define APP_ENDPOINT              1

#define NWK_BUFFERS_AMOUNT                  5
#define NWK_DUPLICATE_REJECTION_TABLE_SIZE  10
#define NWK_DUPLICATE_REJECTION_TTL         500 // ms
#define NWK_ROUTE_TABLE_SIZE                20
#define NWK_ROUTE_DEFAULT_SCORE             3
#define NWK_ACK_WAIT_TIME                   500 // ms

#define NWK_ENABLE_ROUTING

/*- EEPROM Address Declarations --------------------------------------------*/
#define APP_ADDR_EEPROM 		  0x0F16

/*- Includes ---------------------------------------------------------------*/
#include "rfa_global.h"
#include <avr/eeprom.h>

/*- Declarations -----------------------------------------------------------*/



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

static uint8_t			readyToSend  = 0;
static uint16_t			Address = 0x8008;

/*- Implementations --------------------------------------------------------*/
static void OnMsgSend (NWK_DataReq_t* req)
{
	appState = APP_STATE_IDLE;
	(void)req;
}

static void SendMsg (void)
{
	// PORTB ^= 0b01000000;

	appNwkDataReq.dstAddr		= appMessage.dstAddr;
	appNwkDataReq.dstEndpoint	= APP_ENDPOINT;
	appNwkDataReq.srcEndpoint	= APP_ENDPOINT;
	appNwkDataReq.options		= NWK_OPT_ACK_REQUEST;
	appNwkDataReq.data			= (uint8_t*)appMessage.data;
	appNwkDataReq.size			= appMessage.size;
	appNwkDataReq.confirm		= OnMsgSend;

	appState = APP_STATE_WAIT_MSG_SEND;

	NWK_DataReq(&appNwkDataReq);
}

static bool OnMsgRcvd (NWK_DataInd_t* ind)
{
	//Turn ON LED_RX
	// PORTB ^= 0b01000000; 
	return true;
}

static bool IsResetButtonPressed () {
	if ( PINF & (1 << PF0)) 
	{
		_delay_ms(100);
		if ( PINF & (1 << PF0)) 
		{
			return true;
		}
	}
	return false;
}

//****************************************************************
static void APP_TaskHandler(void)
{
	switch (appState)
	{
		case APP_STATE_INITIAL:
		{
			PORTD = 0b00000100;
			// //LED turn off timer
			// TIMSK3	|= (1 << OCIE3A);	//Enable timer OCIE3A interrupt
			// TCNT3	= 0;				//Set timer counter initial value to 0
			// TCCR3A	= 0;
			// TCCR3B	= (1 << CS32) | (0 << CS31) | (1 << CS30);	// 1/1024 prescaler
			// OCR3A	= 15625;		//OCR1A value (1024*(15625/7))/16000000 = 0.1430 sec

			TIMSK1	|= (1 << OCIE1A);	//Enable timer OCIE1A interrupt
			TCNT1	= 0;				//Set timer counter initial value to 0
			TCCR1A	= 0;
			TCCR1B	= (1 << CS12) | (0 << CS11) | (1 << CS10);	// 1/1024 =  prescaler
			OCR1A	= 15625;		//OCR1A value (1024*(15625))/16000000 = 1.0000 sec

			Address = APP_ADDR_SRC;
			NWK_SetAddr(Address);
			NWK_SetPanId(APP_PANID);
			PHY_SetChannel(APP_CHANNEL);
			PHY_SetRxState(true);
			// PHY_SetTxPower();
			NWK_OpenEndpoint(APP_ENDPOINT, OnMsgRcvd);

			sei();
			
			appState = APP_STATE_IDLE;
		} break;

		case APP_STATE_IDLE:
		{
			if (readyToSend) {
				SendMsg();
				readyToSend = 0;
			}

			if(IsButtonPressed()) {
				// PORTD  ^= 0b00000100;
			}
		} break;

		case APP_STATE_WAIT_MSG_SEND:
		{

		} break;
	}
}

int main(void)
{	
	
	//LED OUT
	DDRD  = 0b00000100;
	PORTD = 0b00000000;

	//BUTTON IN
	DDRF  = 0b00000000;
	SYS_Init();
	while (1)
	{

		SYS_TaskHandler();
		APP_TaskHandler();

	}/**/
}

ISR(TIMER1_COMPA_vect)
{
	//Reset timer counter
	PORTD ^= 0b00000100;
	TCNT1 = 0;
}

// ISR(TIMER3_COMPA_vect)
// {
// 	TCNT3 = 1;
// 	// PORTB ^= 0b00001100;
// 	// PORTD ^= 0b00001100;	
// }