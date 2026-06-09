/**
  ******************************************************************************
  * @file    HardwareDetection.h
  * @author  SRA
  * @brief
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

#ifndef HARDWAREDETECTION_H_
#define HARDWAREDETECTION_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

#define NO_IIS3DWB10IS 0U
#define IIS3DWB10IS_DIL24 1U
#define IIS3DWB10IS_FLEX 2U

uint8_t HardwareDetection_Check_Ext_IIS3DWB10IS(void);


#ifdef __cplusplus
}
#endif

#endif /* HARDWAREDETECTION_H_ */
