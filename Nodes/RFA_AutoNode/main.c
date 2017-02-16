	// Address must be set to 0 for the first device, and to 1 for the second one.
#define APP_ADDR_SRC              0x8008 //Routing (network address < 0x8000); Non-routing (network address >= 0x8000)
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


/*- EEPROM Address Declarations --------------------------------------------*/
#define APP_ADDR_EEPROM 		  0x0F16

/*- Includes ---------------------------------------------------------------*/
#include "rfa_global.h"
#include <avr/eeprom.h>

/*- Declarations -----------------------------------------------------------*/
static void Configured();

static void Echo();
static void EchoADC();

static void SetupADC();
static void ReadADC(uint16_t* aOutValue, uint8_t aADCPort);

/*- Types ------------------------------------------------------------------*/
typedef enum AppState_t
{
	APP_STATE_INITIAL,
	APP_STATE_CONFIGURATION,
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
static uint8_t			isConfigured = 1;
static uint16_t			Address = 0x8008;

/*- Implementations --------------------------------------------------------*/
static void OnMsgSend (NWK_DataReq_t* req)
{
	PORTB ^= 0b00100000;

	if(isConfigured) {
		appState = APP_STATE_IDLE;
	} else  {
		appState = APP_STATE_CONFIGURATION;
	}

	(void)req;
}

static void SendMsg (void)
{
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
	PORTB ^= 0b01000000; 

	// const uint8_t s = ind->size;

	//update NET_ADDRESS
	if(ind->data[0]==0x17){
		// Address = (uint16_t)ind->data[1];
		Address = ind->data[2]<<8 | ind->data[1];
		eeprom_write_word((uint16_t*)APP_ADDR_EEPROM,Address);
		Configured();	
	} else if(ind->data[0] == 0x0A){
		Echo();
	} else if(ind->data[0] == 0x0B){
		EchoADC();
	} else if(ind->data[0] == 50) {
		//TODO:
	} else if(ind->data[0] == 51) {
		if(ind->data[1] == 0){
			PORTB = 0b01000000;
			PORTD = 0b00000000;
		}

		if(ind->data[1] == 1){
			PORTB = 0b00100000;
			PORTD = 0b00000000;
		}

		if(ind->data[1] == 2){
			PORTB = 0b00000000;
			PORTD = 0b00000100;
		}
	}

	return true;
}

//****************************************************************

static void APP_TaskHandler(void)
{
	switch (appState)
	{
		case APP_STATE_INITIAL:
		{
			PORTB = 0b00000000;
			PORTD = 0b00000000;

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

			Address = eeprom_read_word((uint16_t*)APP_ADDR_EEPROM);
			if (Address == 0xFFFF) {
				Address = APP_ADDR_SRC;
				isConfigured = 0;
				appState = APP_STATE_CONFIGURATION;
			} else {

				isConfigured = 1;
				appState = APP_STATE_IDLE;
			}

			NWK_SetAddr(Address);
			NWK_SetPanId(APP_PANID);
			PHY_SetChannel(APP_CHANNEL);
			PHY_SetRxState(true);
			//PHY_SetTxPower();
			NWK_OpenEndpoint(APP_ENDPOINT, OnMsgRcvd);

			sei();
		} break;

		case APP_STATE_CONFIGURATION:
		{
			PORTD = 0b00001000;
		} break;

		case APP_STATE_IDLE:
		{
			if (readyToSend) {
				SendMsg();
				readyToSend = 0;
			}
		} break;

		case APP_STATE_WAIT_MSG_SEND:
		{

		} break;
	}
}

int main(void)
{	
	DDRB  = 0b01100000;
	DDRD  = 0b00111100;

	PORTB = 0b00000000;
	PORTD = 0b00000000;
	
	SetupADC();
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
	TCNT1 = 0;

	switch (appState) {
		case APP_STATE_CONFIGURATION: {
			appMessage.dstAddr	= 0x0002;
			appMessage.size = 1;
			appMessage.data[0] = 0xEE;
			SendMsg();
		} break;

		default: {
			
		} break;
	}

	PORTB ^= 0b01000000;
}

static void Configured(){
	TCCR1B	= 0;
	TCCR3B	= 0;
	
	isConfigured = 1;

	PORTB = 0b01100000;
	PORTD = 0b00001000;

	cli();

	_delay_ms(100);

	appState = APP_STATE_INITIAL;
}

// ISR(TIMER3_COMPA_vect)
// {
// 	TCNT3 = 1;
// 	// PORTB ^= 0b00001100;
// 	// PORTD ^= 0b00001100;	
// }

static void Echo(){ 
	if(appState == APP_STATE_IDLE) {
		appMessage.dstAddr	= 0x0002;
		appMessage.size = 2;
		appMessage.data[0] = 0x0A;
		appMessage.data[1] = 0x01;
		readyToSend = 1;
	}
}

static void EchoADC(){
	if(appState == APP_STATE_IDLE) {
		uint16_t value = 0;

		ReadADC(&value, 0);

		appMessage.dstAddr	= 0x0002;
		appMessage.size = 3;
		appMessage.data[0] = 0x0B;
		appMessage.data[1] = value & 0x00ff;
		appMessage.data[2] = value >> 8;
		readyToSend = 1;
	}
}

static void SetupADC(){
    ADMUX = (0 << REFS1) | (1 << REFS0) //1.8v AVDD
            | (0 << ADLAR); //Store 8-bit in ADCH

    ADCSRA = (1 << ADEN) // ADC Enable
            | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); //Prescaler 1/128

	while (ADCSRB & (1 << REFOK)); // wait for Internal ION
}


static void ReadADC(uint16_t* aOutValue, uint8_t aADCPort)
{
	uint8_t valADMUX = ADMUX;
	valADMUX &= ~((1 << MUX5) | (1 << MUX4) | (1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0));

    switch (aADCPort)
	{
		case 0: valADMUX |= (0 << MUX5) | (0 << MUX4) | (0 << MUX3) | (0 << MUX2) | (0 << MUX1) | (0 << MUX0); break;
		case 1: valADMUX |= (0 << MUX5) | (0 << MUX4) | (0 << MUX3) | (0 << MUX2) | (0 << MUX1) | (1 << MUX0); break;
		case 2: valADMUX |= (0 << MUX5) | (0 << MUX4) | (0 << MUX3) | (0 << MUX2) | (1 << MUX1) | (0 << MUX0); break;
		case 3: valADMUX |= (0 << MUX5) | (0 << MUX4) | (0 << MUX3) | (0 << MUX2) | (1 << MUX1) | (1 << MUX0); break;
		case 4: valADMUX |= (0 << MUX5) | (0 << MUX4) | (0 << MUX3) | (1 << MUX2) | (0 << MUX1) | (0 << MUX0); break;
		case 5: valADMUX |= (0 << MUX5) | (0 << MUX4) | (0 << MUX3) | (1 << MUX2) | (0 << MUX1) | (1 << MUX0); break;
		case 6: valADMUX |= (0 << MUX5) | (0 << MUX4) | (0 << MUX3) | (1 << MUX2) | (1 << MUX1) | (0 << MUX0); break;
		case 7: valADMUX |= (0 << MUX5) | (0 << MUX4) | (0 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0); break;
    }

    //valADMUX |= (0 << MUX5) | (0 << MUX4) | (1 << MUX3) | (0 << MUX2) | (1 << MUX1) | (0 << MUX0); //200x

	ADMUX = valADMUX;/**/

	uint16_t v = 0;
	for (uint8_t id = 0; id < (1 << 4); ++id)
	{
		ADCSRA |= (1<<ADSC); // Start conversion
		while (ADCSRA & (1<<ADSC)); // wait for conversion to complete
		v += (uint16_t)ADC;
	}

	*aOutValue = v >> 4;
}
