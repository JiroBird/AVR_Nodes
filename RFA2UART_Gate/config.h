#pragma once

#define MODE_RX
// #define MODE_TX

#if defined(MODE_TX)
#	define APP_ADDR_SRC			0x0001 //Routing (network address < 0x8000); Non-routing (network address >= 0x8000)
#elif defined(MODE_RX)
#	define APP_ADDR_SRC			0x0002 //Routing (network address < 0x8000); Non-routing (network address >= 0x8000)
#else
#	error Unknown MODE. Set MODE_RX or MODE_TX
#endif

#define APP_CHANNEL							0x0F
#define APP_PANID							0x4184
#define APP_ENDPOINT						1

#define NWK_BUFFERS_AMOUNT					5
#define NWK_DUPLICATE_REJECTION_TABLE_SIZE	10
#define NWK_DUPLICATE_REJECTION_TTL			250 // ms
#define NWK_ROUTE_TABLE_SIZE				64
#define NWK_ROUTE_DEFAULT_SCORE				3
#define NWK_ACK_WAIT_TIME					250 // ms

#define NWK_ENABLE_ROUTE_DISCOVERY
#define NWK_ENABLE_ROUTING
