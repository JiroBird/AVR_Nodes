#pragma once

#include "../LwMesh/LwMesh.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if !defined(NULL)
#	define NULL 0
#endif

typedef enum
{
	RES_OK = 0x0,
	RES_FAILED
} Result;
