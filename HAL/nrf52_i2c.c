#include "i2c_init.h"
#include "err.h"
#include "system.h"
#include "nrf52_bitfields.h"

void I2C_Init(I2C_HandleTypeDef* hi2c)
{
	if (hi2c->EasyDMA_Ctrl == EasyDMA_On)
	{
		if (hi2c->Mode == I2C_MODE_SLAVE)
		{
			LOG_ERROR("I2C Init Error: EasyDMA is not available in the current driver release!");
			return;
		}
		if (hi2c->Mode == I2C_MODE_MASTER)
		{
			LOG_ERROR("I2C Init Error: EasyDMA is not available in the current driver release!");
			return;
		}
	}
	else
	{
		if (hi2c->Mode == I2C_MODE_SLAVE)
		{
			LOG_ERROR("I2C Init Warning: slave mode supported only with EasyDMA, mode switched to master");
			hi2c->Mode = I2C_MODE_MASTER;
		}
		if (hi2c->Mode == I2C_MODE_MASTER)
		{
			if (!hi2c->Instance)
			{
				LOG_ERROR("I2C Init Warning: empty instance ptr, TWI0 used by default");
				hi2c->Instance = NRF_TWI0;
			}
			if (hi2c->Instance->ENABLE != 0)
			{
				LOG_ERROR("I2C Init Error: resource conflict");
				return;
			}
		}
	}

	if (hi2c->Mode == I2C_MODE_SLAVE)
	{
		hi2c->Instance->ADDRESS = hi2c->MainAddress;
	}
	
	hi2c->Instance->FREQUENCY = hi2c->Speed;
	hi2c->Instance->PSELSCL = hi2c->SCL_Pin;
	hi2c->Instance->PSELSDA = hi2c->SDA_Pin;
	hi2c->Instance->INTENSET = (uint32_t)hi2c->Interrupts;
	
	hi2c->State = I2C_STATE_DISABLED;
}

void I2C_DeInit(I2C_HandleTypeDef* hi2c)
{
	hi2c->Instance->ENABLE = 0;
}

void I2C_InitDefault(I2C_HandleTypeDef* hi2c)
{
	I2C_Disable(hi2c);
	hi2c->Mode = I2C_MODE_MASTER;
	hi2c->Speed = I2C_SPEED_400K;
	hi2c->SCL_Pin = 13;
	hi2c->SDA_Pin = 12;
	hi2c->Interrupts = I2C_IT_STOPPED | I2C_IT_RXREADY | I2C_IT_TXDSENT | I2C_IT_ERR | I2C_IT_BB | I2C_IT_SUSPEND;
	hi2c->EasyDMA_Ctrl = EasyDMA_Off;
	hi2c->Callback = NULL;
	hi2c->Instance = NRF_TWI0;
	
	I2C_Init(hi2c);
	NVIC_EnableIRQ(SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQn);
}

void I2C_Enable(I2C_HandleTypeDef* hi2c)
{
	if ((hi2c->Instance->ENABLE == 0) && (hi2c->State == I2C_STATE_DISABLED))
	{
		hi2c->Instance->ENABLE = 0x05;
		hi2c->State = I2C_STATE_STOPPED;
	}
}

void I2C_Disable(I2C_HandleTypeDef* hi2c)
{
	if ((hi2c->Instance->ENABLE != 0) && (hi2c->State == I2C_STATE_SUSPENDED || hi2c->State == I2C_STATE_STOPPED))
	{
		hi2c->Instance->ENABLE = 0x00;
		hi2c->State = I2C_STATE_DISABLED;
	}
}

void I2C_StartTransfer(I2C_HandleTypeDef* hi2c, uint8_t address, uint8_t xfer_size, I2C_Xfer_t xfer_type)
{
	hi2c->Instance->ADDRESS = (uint8_t)(address);
	if (xfer_type == I2C_TX)
	{
		hi2c->Instance->SHORTS = 0;
		hi2c->Instance->TASKS_STARTTX = 1;
		hi2c->Instance->TASKS_SUSPEND = 1;
		while(hi2c->State != I2C_STATE_SUSPENDED);
		hi2c->Instance->TASKS_RESUME = 1;
	}
	else
	{
		hi2c->Instance->ADDRESS |= 0x80;
		hi2c->Instance->SHORTS = (xfer_size == 1) ? TWI_SHORTS_BB_STOP_Msk : TWI_SHORTS_BB_SUSPEND_Msk;
		hi2c->Instance->TASKS_SUSPEND = 1;
		while(hi2c->State != I2C_STATE_SUSPENDED);
		hi2c->Instance->TASKS_RESUME = 1;
		hi2c->Instance->TASKS_STARTRX = 1;
	}

}

void I2C_StopTransfer(I2C_HandleTypeDef* hi2c)
{
	hi2c->Instance->SHORTS = 0;
	hi2c->Instance->TASKS_STOP = 1;
	while(hi2c->State != I2C_STATE_STOPPED);
}

void I2C_Transmit(I2C_HandleTypeDef* hi2c, uint8_t* data, uint8_t size)
{
	while(size--)
	{
		hi2c->State = I2C_STATE_TX;
		hi2c->Instance->TXD = *data++;
		while(hi2c->State != I2C_STATE_TXSENT);
	}
}

void I2C_Receive(I2C_HandleTypeDef* hi2c, uint8_t* data, uint8_t size)
{
	while(size-- )
	{
		hi2c->State = I2C_STATE_RX;
		while(hi2c->State != I2C_STATE_RXRDY);
		*data++ = hi2c->Instance->RXD;
	}
}

void I2C_Write(I2C_HandleTypeDef* hi2c, uint8_t address, uint8_t* data, uint8_t size)
{
	I2C_StartTransfer(hi2c, address, size, I2C_TX);
	I2C_Transmit(hi2c, data, size);
}

void I2C_Read(I2C_HandleTypeDef* hi2c, uint8_t address, uint8_t* data, uint8_t size)
{
	I2C_StartTransfer(hi2c, address, size, I2C_RX);
	I2C_Receive(hi2c, data, size);
}

void I2C_WriteMemoryByte(I2C_HandleTypeDef* hi2c, uint8_t address, uint8_t memory_address, uint8_t data)
{
	I2C_Write(hi2c, address, &memory_address, 1);
	I2C_Transmit(hi2c, &data, 1);
	I2C_StopTransfer(hi2c);
}

uint8_t I2C_ReadMemoryByte(I2C_HandleTypeDef* hi2c, uint8_t address, uint8_t memory_address)
{
	uint8_t rx_data;
	I2C_Write(hi2c, address, &memory_address, 1);
	I2C_Read(hi2c, address, &rx_data, 1);
	I2C_StopTransfer(hi2c);
	return rx_data;
}

void I2C_WriteMemory(I2C_HandleTypeDef* hi2c, uint8_t address, uint8_t memory_address, uint8_t* data, uint32_t size)
{
	I2C_Write(hi2c, address, &memory_address, 1);
	I2C_Transmit(hi2c, data, size);
	I2C_StopTransfer(hi2c);
}

void I2C_ReadMemory(I2C_HandleTypeDef* hi2c, uint8_t address, uint8_t memory_address, uint8_t* data, uint32_t size)
{
	I2C_Write(hi2c, address, &memory_address, 1);
	I2C_Read(hi2c, address, data, size);
	I2C_StopTransfer(hi2c);
}

void I2C_WriteMemory16(I2C_HandleTypeDef* hi2c, uint8_t address, uint16_t memory_address, uint8_t* data, uint32_t size)
{
	I2C_Write(hi2c, address, (uint8_t*)&memory_address, 2);
	I2C_Transmit(hi2c, data, size);
	I2C_StopTransfer(hi2c);
}

void I2C_ReadMemory16(I2C_HandleTypeDef* hi2c, uint8_t address, uint16_t memory_address, uint8_t* data, uint32_t size)
{
	I2C_Write(hi2c, address, (uint8_t*)&memory_address, 2);
	I2C_Receive(hi2c, data, size);
	I2C_StopTransfer(hi2c);
}

void I2C_IRQHandler(I2C_HandleTypeDef* hi2c)
{
	if (hi2c->Instance->EVENTS_ERROR)
	{
		LOG_ERROR("I2C Error: %d", k);
		hi2c->Instance->EVENTS_ERROR = 0;
	}
	if (hi2c->Instance->EVENTS_STOPPED)
	{
		hi2c->Instance->EVENTS_STOPPED = 0;
		hi2c->State = I2C_STATE_STOPPED;
	}
	if (hi2c->Instance->EVENTS_RXDREADY)
	{
		hi2c->Instance->EVENTS_RXDREADY = 0;
		hi2c->State = I2C_STATE_RXRDY;
		hi2c->Instance->TASKS_RESUME = 1;
	}
	if (hi2c->Instance->EVENTS_TXDSENT)
	{
		hi2c->Instance->EVENTS_TXDSENT = 0;
		hi2c->State = I2C_STATE_TXSENT;
	}
	if (hi2c->Instance->EVENTS_BB)
	{
		hi2c->Instance->EVENTS_BB = 0;
	}
	if (hi2c->Instance->EVENTS_SUSPENDED)
	{
		hi2c->Instance->EVENTS_SUSPENDED = 0;
		hi2c->State = I2C_STATE_SUSPENDED;
	}
	if (hi2c->Callback) hi2c->Callback();
}
