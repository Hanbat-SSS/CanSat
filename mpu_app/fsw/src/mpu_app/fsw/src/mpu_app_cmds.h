/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as “core Flight System: Bootes”
 *
 * Copyright (c) 2020 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ************************************************************************/

/**
 * @file
 *   This file contains the prototypes for the Mpu App Ground Command-handling functions
 */

#ifndef MPU_APP_CMDS_H
#define MPU_APP_CMDS_H

/*
** Required header files.
*/
#include "cfe_error.h"
#include "mpu_app_msg.h"

CFE_Status_t MPU_APP_SendHkCmd(const MPU_APP_SendHkCmd_t *Msg);
CFE_Status_t MPU_APP_ResetCountersCmd(const MPU_APP_ResetCountersCmd_t *Msg);
CFE_Status_t MPU_APP_ProcessCmd(const MPU_APP_ProcessCmd_t *Msg);
CFE_Status_t MPU_APP_NoopCmd(const MPU_APP_NoopCmd_t *Msg);
CFE_Status_t MPU_APP_DisplayParamCmd(const MPU_APP_DisplayParamCmd_t *Msg);
CFE_Status_t MPU_APP_ReadingCmd(const MPU_APP_ReadingCmd_t *Msg);
CFE_Status_t MPU_APP_Stop_ReadingCmd(const MPU_APP_Stop_ReadingCmd_t *Msg);
CFE_Status_t MPU_APP_Currant_DataCmd(const MPU_APP_Currant_DataCmd_t *Msg);
CFE_Status_t MPU_APP_Rewheel_OnCmd(const MPU_APP_Rewheel_OnCmd_t *Msg);
CFE_Status_t MPU_APP_Rewheel_OffCmd(const MPU_APP_Rewheel_OffCmd_t *Msg);

#endif /* MPU_APP_CMDS_H */
