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
 *   This file contains the source code for the Cam App Ground Command-handling functions
 */

/*
** Include Files:
*/
#include "cam_app.h"
#include "cam_app_cmds.h"
#include "cam_app_msgids.h"
#include "cam_app_eventids.h"
#include "cam_app_version.h"
#include "cam_app_tbl.h"
#include "cam_app_utils.h"
#include "cam_app_msg.h"

/* The cam_lib module provides the CAM_Function() prototype */
#include "cam_lib.h"
#include "time.h"
#include "unistd.h"
#include "pthread.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a task telemetry request */
/*         from the housekeeping task. This function will gather the Apps     */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t CAM_APP_SendHkCmd(const CAM_APP_SendHkCmd_t *Msg)
{
    int i;

    /*
    ** Get command execution counters...
    */
    CAM_APP_Data.HkTlm.Payload.CommandErrorCounter = CAM_APP_Data.ErrCounter;
    CAM_APP_Data.HkTlm.Payload.CommandCounter      = CAM_APP_Data.CmdCounter;

    /*
    ** Send housekeeping telemetry packet...
    */
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(CAM_APP_Data.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(CAM_APP_Data.HkTlm.TelemetryHeader), true);

    /*
    ** Manage any pending table loads, validations, etc.
    */
    for (i = 0; i < CAM_APP_NUMBER_OF_TABLES; i++)
    {
        CFE_TBL_Manage(CAM_APP_Data.TblHandles[i]);
    }

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* CAM NOOP commands                                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t CAM_APP_NoopCmd(const CAM_APP_NoopCmd_t *Msg)
{
    CAM_APP_Data.CmdCounter++;

    CFE_EVS_SendEvent(CAM_APP_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "CAM: NOOP command %s",
                      CAM_APP_VERSION);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry.                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t CAM_APP_ResetCountersCmd(const CAM_APP_ResetCountersCmd_t *Msg)
{
    CAM_APP_Data.CmdCounter = 0;
    CAM_APP_Data.ErrCounter = 0;

    CFE_EVS_SendEvent(CAM_APP_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "CAM: RESET command");

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function Process Ground Station Command                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t CAM_APP_ProcessCmd(const CAM_APP_ProcessCmd_t *Msg)
{
    CFE_Status_t               status;
    void *                     TblAddr;
    CAM_APP_ExampleTable_t *TblPtr;
    const char *               TableName = "CAM_APP.ExampleTable";

    /* Cam Use of Example Table */

    status = CFE_TBL_GetAddress(&TblAddr, CAM_APP_Data.TblHandles[0]);

    if (status < CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Cam App: Fail to get table address: 0x%08lx", (unsigned long)status);
        return status;
    }

    TblPtr = TblAddr;
    CFE_ES_WriteToSysLog("Cam App: Example Table Value 1: %d  Value 2: %d", TblPtr->Int1, TblPtr->Int2);

    CAM_APP_GetCrc(TableName);

    status = CFE_TBL_ReleaseAddress(CAM_APP_Data.TblHandles[0]);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Cam App: Fail to release table address: 0x%08lx", (unsigned long)status);
        return status;
    }

    /* Invoke a function provided by CAM_APP_LIB */
    CAM_LIB_Function();

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* A simple example command that displays a passed-in value                   */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t CAM_APP_DisplayParamCmd(const CAM_APP_DisplayParamCmd_t *Msg)
{
    CFE_EVS_SendEvent(CAM_APP_VALUE_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "CAM_APP: ValU32=%lu, ValI16=%d, ValStr=%s", (unsigned long)Msg->Payload.ValU32,
                      (int)Msg->Payload.ValI16, Msg->Payload.ValStr);

    return CFE_SUCCESS;
}

CFE_Status_t CAM_APP_ShotPeriodCmd(const CAM_APP_ShotPeriodCmd_t *Msg)
{
    //process for Shot_Period_Cmd

    //CFE_EVS_SendEvent
    
   

    return CFE_SUCCESS; 
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* A Cam_app Start, Stop Process to using thread                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

int Stop_Command;
pthread_t thread;

void *startloop(void *arg)
{
    while (1)
    {
        if(Stop_Command == 1)
        {
            break;
        }
        else
        {
            time_t t = time(NULL);
            struct tm *now = localtime(&t);

            char timestamp[20];
            strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H:%M:%S", now);

            char filename[50];
            sprintf(filename, "photo_%s.jpg", timestamp);

            char command[200];
            sprintf(command, "libcamera-jpeg -o /home/CFS/photo/%s -t 2000 --width 640 --height 480", filename);

            printf("Executing command: %s\n", command);

            system(command);

            sleep(10);
        }
    }
    pthread_exit(NULL);
}

void stoploop(void)
{
    Stop_Command = 1;
}

CFE_Status_t CAM_APP_ShotStartCmd(const CAM_APP_ShotStartCmd_t *Msg)
{    
    

    Stop_Command = 0;

    if(pthread_create(&thread, NULL, startloop, NULL))
    {
        perror("pthread_Create");
        exit(EXIT_FAILURE);
    }
    
    CFE_EVS_SendEvent(CAM_APP_SHOT_START_INF_EID, CFE_EVS_EventType_INFORMATION, "CAM: Image Shot Start");
    return CFE_SUCCESS; 
}

CFE_Status_t CAM_APP_ShotStopCmd(const CAM_APP_ShotStopCmd_t *Msg)
{
    //process from Shot_Stop_Cmd
    //CFE_EVS_StopEvent

    stoploop();
    pthread_join(thread, NULL);
    CFE_EVS_SendEvent(CAM_APP_SHOT_STOP_INF_EID, CFE_EVS_EventType_INFORMATION, "CAM: Send Stop_Command");
    
    return CFE_SUCCESS;
}