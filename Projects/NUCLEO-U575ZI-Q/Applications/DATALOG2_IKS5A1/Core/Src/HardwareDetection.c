/**
  ******************************************************************************
  * @file    HardwareDetection.c
  * @author  SRA
  * @brief
  *
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file in
  * the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  *
  ******************************************************************************
  */

#include "mx.h"
#include "spi.h"
#include "services/systypes.h"
#include "HardwareDetection.h"
#include "iis3dwb10is_reg.h"

extern SPI_HandleTypeDef hspi1;
extern I2C_HandleTypeDef hi2c2;
extern I2C_HandleTypeDef hi2c3;

static int32_t ext_sensor_spi_read(void *handle, uint8_t reg, uint8_t *p_data, uint16_t size);
static int32_t ext_sensor_spi_write(void *handle, uint8_t reg, uint8_t *p_data, uint16_t size);

/* Public functions declaration */
/*********************************/
/**
  * Detect an external IIS3DWB10IS sensor
  *
  * @return 0 if no sensor is detected, 1 if a DIL24 is detected, 2 if a FLEX is detected
  */
uint8_t HardwareDetection_Check_Ext_IIS3DWB10IS(void)
{
  uint8_t whoami_val = 0U;
  uint8_t found = NO_IIS3DWB10IS;
  stmdev_ctx_t ctx;

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_PinState flex = (GPIO_PinState)0;
  GPIO_PinState dil = (GPIO_PinState)0;

  ctx.read_reg = ext_sensor_spi_read;
  ctx.write_reg = ext_sensor_spi_write;

  MX_SPI1_Init();

  iis3dwb10is_device_id_get(&ctx, (uint8_t *) &whoami_val);

  if (whoami_val == IIS3DWB10IS_ID)
  {
    NVIC_DisableIRQ(DIL_INT1_EXTI_IRQn);
    NVIC_ClearPendingIRQ(DIL_INT1_EXTI_IRQn);
    NVIC_DisableIRQ(USER_INT_EXTI_IRQn);
    NVIC_ClearPendingIRQ(USER_INT_EXTI_IRQn);

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOE_CLK_ENABLE();
    /*Configure GPIO pins : PEPin PEPin */
    GPIO_InitStruct.Pin = DIL_INT1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(DIL_INT1_GPIO_Port, &GPIO_InitStruct);

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOF_CLK_ENABLE();
    /*Configure GPIO pins : PEPin PEPin */
    GPIO_InitStruct.Pin = USER_INT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(USER_INT_GPIO_Port, &GPIO_InitStruct);

    uint8_t data = 0x00;
    iis3dwb10is_write_reg(&ctx, IIS3DWB10IS_IF_CFG, &data, 1);
    data = 0x0C;
    iis3dwb10is_write_reg(&ctx, IIS3DWB10IS_PIN_CTRL, &data, 1);
    data = 0x10;
    iis3dwb10is_write_reg(&ctx, IIS3DWB10IS_IF_CFG, &data, 1);

    flex = HAL_GPIO_ReadPin(USER_INT_GPIO_Port, USER_INT_Pin);
    dil = HAL_GPIO_ReadPin(DIL_INT1_GPIO_Port, DIL_INT1_Pin);

    data = 0x00;
    iis3dwb10is_write_reg(&ctx, IIS3DWB10IS_IF_CFG, &data, 1);

    if (HAL_GPIO_ReadPin(USER_INT_GPIO_Port, USER_INT_Pin) != flex)
    {
      found = IIS3DWB10IS_FLEX;
    }
    else if (HAL_GPIO_ReadPin(DIL_INT1_GPIO_Port, DIL_INT1_Pin) != dil)
    {
      found = IIS3DWB10IS_DIL24;
    }
    else
    {
      /* No sensor detected */
      found = NO_IIS3DWB10IS;
    }
  }

  HAL_SPI_DeInit(&hspi1);
  return found;
}


/* Private function definition */
/*******************************/

static int32_t ext_sensor_spi_write(void *handle, uint8_t reg, uint8_t *p_data, uint16_t size)
{
  /* CS Enable */
  HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET);

  HAL_SPI_Transmit(&hspi1, &reg, 1, 1000);
  HAL_SPI_Transmit(&hspi1, p_data, size, 1000);

  /* CS Disable */
  HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET);

  return 0;
}

static int32_t ext_sensor_spi_read(void *handle, uint8_t reg, uint8_t *p_data, uint16_t size)
{
  /* CS Enable */
  HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET);

  reg |= 0x80;

  HAL_SPI_Transmit(&hspi1, &reg, 1, 1000);
  HAL_SPI_Receive(&hspi1, p_data, size, 1000);

  /* CS Disable */
  HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET);

  return 0;
}

