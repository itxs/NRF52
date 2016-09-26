#include "system.h"

static volatile uint32_t DelayCounter;

void SysTick_Init(uint32_t period)
{
	SysTick_Config((SystemCoreClock / 1000000) * period);
	NVIC_EnableIRQ(SysTick_IRQn);
}

void Delay(uint32_t period)
{
	DelayCounter = period;
	while(DelayCounter);
}

void SysTick_Handler()
{
	if (DelayCounter)
		DelayCounter--;
}

void nrf_error_handler(uint32_t err_code, uint32_t line)
{
#if DEBUG
    while(1);
#else
    NVIC_SystemReset();
#endif
}
