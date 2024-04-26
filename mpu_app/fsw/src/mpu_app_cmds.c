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
 * \file
 *   This file contains the source code for the Mpu App Ground Command-handling functions
 */

/*
** Include Files:
*/
#include "mpu_app.h"
#include "mpu_app_cmds.h"
#include "mpu_app_msgids.h"
#include "../inc/mpu_app_eventids.h"
#include "mpu_app_version.h"
#include "mpu_app_tbl.h"
#include "mpu_app_utils.h"
#include "mpu_app_msg.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a task telemetry request */
/*         from the housekeeping task. This function will gather the Apps     */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t MPU_APP_SendHkCmd(const MPU_APP_SendHkCmd_t *Msg)
{
    int i;

    /*
    ** Get command execution counters...
    */
    MPU_APP_Data.HkTlm.Payload.CommandErrorCounter = MPU_APP_Data.ErrCounter;
    MPU_APP_Data.HkTlm.Payload.CommandCounter      = MPU_APP_Data.CmdCounter;

    /*
    ** Send housekeeping telemetry packet...
    */
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(MPU_APP_Data.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(MPU_APP_Data.HkTlm.TelemetryHeader), true);

    /*
    ** Manage any pending table loads, validations, etc.
    */
    for (i = 0; i < MPU_APP_NUMBER_OF_TABLES; i++)
    {
        CFE_TBL_Manage(MPU_APP_Data.TblHandles[i]);
    }

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* MPU NOOP commands                                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t MPU_APP_NoopCmd(const MPU_APP_NoopCmd_t *Msg)
{
    MPU_APP_Data.CmdCounter++;

    CFE_EVS_SendEvent(MPU_APP_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "MPU: NOOP command %s",
                      MPU_APP_VERSION);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry.                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t MPU_APP_ResetCountersCmd(const MPU_APP_ResetCountersCmd_t *Msg)
{
    MPU_APP_Data.CmdCounter = 0;
    MPU_APP_Data.ErrCounter = 0;

    CFE_EVS_SendEvent(MPU_APP_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "MPU: RESET command");

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function Process Ground Station Command                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t MPU_APP_ProcessCmd(const MPU_APP_ProcessCmd_t *Msg)
{
    CFE_Status_t               status;
    void *                     TblAddr;
    MPU_APP_ExampleTable_t *TblPtr;
    const char *               TableName = "MPU_APP.ExampleTable";

    /* Mpu Use of Example Table */

    status = CFE_TBL_GetAddress(&TblAddr, MPU_APP_Data.TblHandles[0]);

    if (status < CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Mpu App: Fail to get table address: 0x%08lx", (unsigned long)status);
        return status;
    }

    TblPtr = TblAddr;
    CFE_ES_WriteToSysLog("Mpu App: Example Table Value 1: %d  Value 2: %d", TblPtr->Int1, TblPtr->Int2);

    MPU_APP_GetCrc(TableName);

    status = CFE_TBL_ReleaseAddress(MPU_APP_Data.TblHandles[0]);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Mpu App: Fail to release table address: 0x%08lx", (unsigned long)status);
        return status;

    }
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* A simple example command that displays a passed-in value                   */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t MPU_APP_DisplayParamCmd(const MPU_APP_DisplayParamCmd_t *Msg)
{
    CFE_EVS_SendEvent(MPU_APP_VALUE_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "MPU_APP: ValU32=%lu, ValI16=%d, ValStr=%s", (unsigned long)Msg->Payload.ValU32,
                      (int)Msg->Payload.ValI16, Msg->Payload.ValStr);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                              */
/* A Mpu data reading command                                                   */
/*                                                                              */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

CFE_Status_t MPU_APP_ReadingCmd(const MPU_APP_ReadingCmd_t *Msg)
{
    /* MPU parsing code*/

    return CFE_SUCCESS;
}