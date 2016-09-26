#ifndef __SYSTEM_H
#define __SYSTEM_H

#include "nrf52.h"

typedef enum
{
	OK = 0,
	ERROR,
	PEND
} STATUS_t;

typedef enum
{
	REALTIME = 0,
	HIGHEST,
	VERYHIGH,
	HIGH,
	DEFAULT,
	LOW,
	VERYLOW,
	LOWEST,
	IDLE
} IRQPriority_t;

typedef struct
{
	uint8_t Address;
	uint8_t Value;
} Register8_t;

typedef struct
{
	uint16_t Address;
	uint8_t Value;
} Register16_t;

typedef struct
{
    uint8_t*	pBuffer;	/**< Pointer to FIFO buffer memory.                      */
    uint16_t	SizeMask;	/**< Read/write index mask. Also used for size checking. */
    uint32_t	ReadPos;	/**< Next read position in the FIFO buffer.              */
    uint32_t	WritePos;	/**< Next write position in the FIFO buffer.             */
} FIFO_t;

void SysTick_Init(uint32_t period);
void Delay(uint32_t period);
void SysTick_Handler(void);

#endif

