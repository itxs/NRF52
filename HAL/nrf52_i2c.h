#ifndef __I2C_INIT_H
#define __I2C_INIT_H

#include "stdint.h"
#include "nrf52.h"

typedef enum
{
	I2C_IT_STOPPED = (1 << 1),
	I2C_IT_RXREADY = (1 << 2),
	I2C_IT_TXDSENT = (1 << 7),
	I2C_IT_ERR = (1 << 9),
	I2C_IT_BB = (1 << 14),
	I2C_IT_SUSPEND = (1 << 18)
} I2C_IT_t;

typedef enum
{
	I2C_SPEED_100K = 0x1980000,
	I2C_SPEED_250K = 0x4000000,
	I2C_SPEED_400K = 0x6680000
} I2C_Speed_t;

typedef enum
{
	I2C_RX,
	I2C_TX
} I2C_Xfer_t;

typedef enum
{
	I2C_MODE_MASTER,
	I2C_MODE_SLAVE
} I2C_Mode_t;

typedef enum
{
	I2C_STATE_DISABLED,
	I2C_STATE_STOPPED,
	I2C_STATE_SUSPENDED,
	I2C_STATE_TX,
	I2C_STATE_RX,
	I2C_STATE_TXSENT,
	I2C_STATE_RXRDY
} I2C_State_t;

typedef enum
{
	EasyDMA_Off,
	EasyDMA_On
} I2C_EasyDMA_Ctrl_t;

typedef struct
{
	NRF_TWI_Type*		Instance;
	I2C_Mode_t			Mode;
	I2C_EasyDMA_Ctrl_t	EasyDMA_Ctrl;
	I2C_Speed_t			Speed;
	uint8_t				SDA_Pin;
	uint8_t				SCL_Pin;
	uint8_t				MainAddress;		//Main address in slave mode
	I2C_State_t			State;
	uint32_t			Errors;
	uint32_t			Interrupts;
	void				(*Callback)();
} I2C_HandleTypeDef;

extern I2C_HandleTypeDef hI2C;

/// Function prototypes
void I2C_Init(I2C_HandleTypeDef* hi2c);
void I2C_InitDefault(I2C_HandleTypeDef* hi2c);
void I2C_Enable(I2C_HandleTypeDef* hi2c);
void I2C_Disable(I2C_HandleTypeDef* hi2c);
void I2C_Write(I2C_HandleTypeDef* hi2c, uint8_t address, uint8_t* data, uint8_t size);
void I2C_Read(I2C_HandleTypeDef* hi2c, uint8_t address, uint8_t* data, uint8_t size);
void I2C_WriteMemoryByte(I2C_HandleTypeDef* hi2c, uint8_t address, uint8_t memory_address, uint8_t data);
uint8_t I2C_ReadMemoryByte(I2C_HandleTypeDef* hi2c, uint8_t address, uint8_t memory_address);
void I2C_WriteMemory(I2C_HandleTypeDef* hi2c, uint8_t address, uint8_t memory_address, uint8_t* data, uint32_t size);
void I2C_ReadMemory(I2C_HandleTypeDef* hi2c, uint8_t address, uint8_t memory_address, uint8_t* data, uint32_t size);
void I2C_WriteMemory16(I2C_HandleTypeDef* hi2c, uint8_t address, uint16_t memory_address, uint8_t* data, uint32_t size);
void I2C_ReadMemory16(I2C_HandleTypeDef* hi2c, uint8_t address, uint16_t memory_address, uint8_t* data, uint32_t size);
void I2C_IRQHandler(I2C_HandleTypeDef* hi2c);

#endif
