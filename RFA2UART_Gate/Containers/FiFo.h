#pragma once

#include "../rfa_global.h"

#define FIFO struct\
{\
	uint8_t buf[256];\
	uint8_t tail;\
	uint8_t head;\
}

//количество элементов в очереди
#define FIFO_COUNT(fifo)  ((uint8_t)(fifo.head - fifo.tail))

//fifo заполнено?
#define FIFO_IS_FULL(fifo)		(FIFO_COUNT(fifo)==255)//256

//fifo пусто?
#define FIFO_IS_EMPTY(fifo)		(fifo.tail==fifo.head)

//количество свободного места в fifo
#define FIFO_FREE_SPACE(fifo)	(255-FIFO_COUNT(fifo))

//поместить элемент в fifo
#define FIFO_PUSH(fifo, byte) fifo.buf[fifo.head++]=byte

//взять первый элемент из fifo
#define FIFO_GET(fifo) fifo.buf[fifo.tail]

//уменьшить количество элементов в очереди
#define FIFO_POP(fifo) fifo.tail++;

//очистить fifo
#define FIFO_CLEAR(fifo)\
{\
	fifo.tail=0;\
	fifo.head=0;\
}
