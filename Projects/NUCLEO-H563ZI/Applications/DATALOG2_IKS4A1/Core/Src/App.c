/**
  ******************************************************************************
  * @file    App.c
  * @author  SRA
  * @brief   Define the Application main entry points
  *
  *
  * This file is the main entry point for the user code.
  *
  * The framework `weak` functions are redefined in this file and they link
  * the application specific code with the framework:
  * - SysLoadApplicationContext(): it is the first application defined function
  *   called by the framework. Here we define all managed tasks. A managed task
  *   implements one or more application specific feature.
  * - SysOnStartApplication(): this function is called by the framework
  *   when the system is initialized (all managed task objects have been
  *   initialized), and before the INIT task release the control. Here we
  *   link the application objects according to the application design.
  *
  * The execution time  between the two above functions is called
  * *system initialization*. During this period only the INIT task is running.
  *
  * Each managed task will be activated in turn to initialize its hardware
  * resources, if any - MyTask_vtblHardwareInit() - and its software
  * resources - MyTask_vtblOnCreateTask().
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

#include "services/sysdebug.h"
#include "services/ApplicationContext.h"
#include "AppPowerModeHelper.h"

#include "mx.h"

#include "stts22h_reg.h"

#include "UtilTask.h"

#include "I3CBusTask.h"
#include "LIS2DUXS12Task.h"
#include "LIS2MDLTask.h"
#include "LPS22DFTask.h"
#include "LSM6DSO16ISTask.h"
#include "LSM6DSV16XTask.h"
#include "SHT40Task.h"
#include "STTS22HTask.h"
#include "DatalogAppTask.h"
#include "App_model.h"

#include "PnPLCompManager.h"
#include "Lis2duxs12_Acc_PnPL.h"
#include "Lis2duxs12_Mlc_PnPL.h"
#include "Lps22df_Press_PnPL.h"
#include "Lsm6dsv16x_Acc_PnPL.h"
#include "Lsm6dsv16x_Gyro_PnPL.h"
#include "Lsm6dsv16x_Mlc_PnPL.h"
#include "Automode_PnPL.h"
#include "Log_Controller_PnPL.h"
#include "Tags_Info_PnPL.h"
#include "Acquisition_Info_PnPL.h"
#include "Firmware_Info_PnPL.h"
#include "Deviceinformation_PnPL.h"

static IPnPLComponent_t *pLis2duxs12_Acc_PnPLObj = NULL;
static IPnPLComponent_t *pLis2duxs12_Mlc_PnPLObj = NULL;
static IPnPLComponent_t *pLps22df_Press_PnPLObj = NULL;
static IPnPLComponent_t *pLsm6dsv16x_Acc_PnPLObj = NULL;
static IPnPLComponent_t *pLsm6dsv16x_Gyro_PnPLObj = NULL;
static IPnPLComponent_t *pLsm6dsv16x_Mlc_PnPLObj = NULL;
static IPnPLComponent_t *pAutomode_PnPLObj = NULL;
static IPnPLComponent_t *pLog_Controller_PnPLObj = NULL;
static IPnPLComponent_t *pTags_Info_PnPLObj = NULL;
static IPnPLComponent_t *pAcquisition_Info_PnPLObj = NULL;
static IPnPLComponent_t *pFirmware_Info_PnPLObj = NULL;
static IPnPLComponent_t *pDeviceinformation_PnPLObj = NULL;

/**
  * Utility task object.
  */
static AManagedTaskEx *sUtilObj = NULL;

/**
  * Bus task object.
  */
static AManagedTaskEx *sI3CBusObj = NULL;

/**
  * Sensor task object.
  */
static AManagedTaskEx *sLIS2DUXS12Obj = NULL;
static AManagedTaskEx *sLPS22DFObj = NULL;
static AManagedTaskEx *sLSM6DSO16ISObj = NULL;
static AManagedTaskEx *sLSM6DSV16XObj = NULL;
/**
  * DatalogApp
  */
static AManagedTaskEx *sDatalogAppObj = NULL;

/**
  * Pnpl mutex definition for thread safe purpose
  */
static TX_MUTEX pnpl_mutex;

/**
  * Private function declaration
  */
static void PnPL_lock_fp(void);
static void PnPL_unlock_fp(void);

/* eLooM framework entry points definition */
/*******************************************/

sys_error_code_t SysLoadApplicationContext(ApplicationContext *pAppContext)
{
  assert_param(pAppContext);
  sys_error_code_t res = SYS_NO_ERROR_CODE;

  /* PnPL thread safe mutex creation */
  tx_mutex_create(&pnpl_mutex, "PnPL Mutex", TX_INHERIT);

  /* PnPL thread safe function registration */
  PnPL_SetLockUnlockCallbacks(PnPL_lock_fp, PnPL_unlock_fp);

  PnPLSetAllocationFunctions(SysAlloc, SysFree);

  /************ Allocate task objects ************/
  sUtilObj = UtilTaskAlloc(&MX_GPIO_LEDYellowInitParams, NULL);
  sDatalogAppObj = DatalogAppTaskAlloc();
  sI3CBusObj = I3CBusTaskAlloc(&MX_I3C1InitParams);
  sLIS2DUXS12Obj = LIS2DUXS12TaskAlloc(NULL, NULL, NULL, true);
  sLPS22DFObj = LPS22DFTaskAlloc(NULL, NULL, true);
  sLSM6DSV16XObj = LSM6DSV16XTaskAlloc(&MX_GPIO_LSM6DSV16X_INT1InitParams, NULL, NULL, true);


  /************ Add the task object to the context ************/
  res = ACAddTask(pAppContext, (AManagedTask *) sUtilObj);
  res = ACAddTask(pAppContext, (AManagedTask *) sDatalogAppObj);
  res = ACAddTask(pAppContext, (AManagedTask *) sI3CBusObj);
  res = ACAddTask(pAppContext, (AManagedTask *) sLIS2DUXS12Obj);
  res = ACAddTask(pAppContext, (AManagedTask *) sLPS22DFObj);
  res = ACAddTask(pAppContext, (AManagedTask *) sLSM6DSV16XObj);

  pLis2duxs12_Acc_PnPLObj = Lis2duxs12_Acc_PnPLAlloc();
  pLis2duxs12_Mlc_PnPLObj = Lis2duxs12_Mlc_PnPLAlloc();
  pLps22df_Press_PnPLObj = Lps22df_Press_PnPLAlloc();
  pLsm6dsv16x_Acc_PnPLObj = Lsm6dsv16x_Acc_PnPLAlloc();
  pLsm6dsv16x_Gyro_PnPLObj = Lsm6dsv16x_Gyro_PnPLAlloc();
  pLsm6dsv16x_Mlc_PnPLObj = Lsm6dsv16x_Mlc_PnPLAlloc();
  pAutomode_PnPLObj = Automode_PnPLAlloc();
  pLog_Controller_PnPLObj = Log_Controller_PnPLAlloc();
  pTags_Info_PnPLObj = Tags_Info_PnPLAlloc();
  pAcquisition_Info_PnPLObj = Acquisition_Info_PnPLAlloc();
  pFirmware_Info_PnPLObj = Firmware_Info_PnPLAlloc();
  pDeviceinformation_PnPLObj = Deviceinformation_PnPLAlloc();

  return res;
}

sys_error_code_t SysOnStartApplication(ApplicationContext *pAppContext)
{
  UNUSED(pAppContext);

  /************ Connect the sensor task to the bus ************/
  I3CBusTaskConnectDevice((I3CBusTask *) sI3CBusObj,
                          (I3CBusIF *)LIS2DUXS12TaskGetSensorIF((LIS2DUXS12Task *) sLIS2DUXS12Obj));
  I3CBusTaskConnectDevice((I3CBusTask *) sI3CBusObj, (I3CBusIF *)LPS22DFTaskGetSensorIF((LPS22DFTask *) sLPS22DFObj));
  I3CBusTaskConnectDevice((I3CBusTask *) sI3CBusObj,
                          (I3CBusIF *)LSM6DSV16XTaskGetSensorIF((LSM6DSV16XTask *) sLSM6DSV16XObj));

  /************ Connect the Sensor events to the DatalogAppTask ************/
  IEventListener *DatalogAppListener = DatalogAppTask_GetEventListenerIF((DatalogAppTask *) sDatalogAppObj);
  IEventSrcAddEventListener(LIS2DUXS12TaskGetEventSrcIF((LIS2DUXS12Task *) sLIS2DUXS12Obj), DatalogAppListener);
  IEventSrcAddEventListener(LIS2DUXS12TaskGetMlcEventSrcIF((LIS2DUXS12Task *) sLIS2DUXS12Obj), DatalogAppListener);
  IEventSrcAddEventListener(LPS22DFTaskGetPressEventSrcIF((LPS22DFTask *) sLPS22DFObj), DatalogAppListener);
  IEventSrcAddEventListener(LSM6DSV16XTaskGetAccEventSrcIF((LSM6DSV16XTask *) sLSM6DSV16XObj), DatalogAppListener);
  IEventSrcAddEventListener(LSM6DSV16XTaskGetGyroEventSrcIF((LSM6DSV16XTask *) sLSM6DSV16XObj), DatalogAppListener);
  IEventSrcAddEventListener(LSM6DSV16XTaskGetMlcEventSrcIF((LSM6DSV16XTask *) sLSM6DSV16XObj), DatalogAppListener);

  /************ Connect Sensor LL to be used for ucf management to the DatalogAppTask ************/
  if (sLSM6DSV16XObj)
  {
    DatalogAppTask_Set_LSMDSV16XMLC_IF((AManagedTask *) sLSM6DSV16XObj);
  }
  if (sLIS2DUXS12Obj)
  {
    DatalogAppTask_Set_LIS2DUXS12MLC_IF((AManagedTask *) sLIS2DUXS12Obj);
  }
  if (sLSM6DSO16ISObj)
  {
    DatalogAppTask_Set_LSM6DSO16ISMLC_IF((AManagedTask *) sLSM6DSO16ISObj);
  }

  /************ Other PnPL Components ************/
  Automode_PnPLInit(pAutomode_PnPLObj);
  Log_Controller_PnPLInit(pLog_Controller_PnPLObj);
  Tags_Info_PnPLInit(pTags_Info_PnPLObj);
  Acquisition_Info_PnPLInit(pAcquisition_Info_PnPLObj);
  Firmware_Info_PnPLInit(pFirmware_Info_PnPLObj);
  Deviceinformation_PnPLInit(pDeviceinformation_PnPLObj);

  /************ Sensor PnPL Components ************/
  Lis2duxs12_Acc_PnPLInit(pLis2duxs12_Acc_PnPLObj);
  Lis2duxs12_Mlc_PnPLInit(pLis2duxs12_Mlc_PnPLObj);
  Lps22df_Press_PnPLInit(pLps22df_Press_PnPLObj);
  Lsm6dsv16x_Acc_PnPLInit(pLsm6dsv16x_Acc_PnPLObj);
  Lsm6dsv16x_Gyro_PnPLInit(pLsm6dsv16x_Gyro_PnPLObj);
  Lsm6dsv16x_Mlc_PnPLInit(pLsm6dsv16x_Mlc_PnPLObj);

  return SYS_NO_ERROR_CODE;
}

IAppPowerModeHelper *SysGetPowerModeHelper(void)
{
  /* Install the application power mode helper. */
  static IAppPowerModeHelper *s_pxPowerModeHelper = NULL;
  if (s_pxPowerModeHelper == NULL)
  {
    s_pxPowerModeHelper = AppPowerModeHelperAlloc();
  }

  return s_pxPowerModeHelper;
}


static void PnPL_lock_fp(void)
{
  tx_mutex_get(&pnpl_mutex, TX_NO_WAIT);
}

static void PnPL_unlock_fp(void)
{
  tx_mutex_put(&pnpl_mutex);
}
