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
 *   This file contains the source code for the Cam App.
 */

/*
** Include Files:
*/
#include "cam_app.h"
#include "cam_app_cmds.h"
#include "cam_app_utils.h"
#include "cam_app_eventids.h"
#include "cam_app_dispatch.h"
#include "cam_app_tbl.h"
#include "cam_app_version.h"

#include "time.h"
#include "unistd.h"

/*
** global data
*/
CAM_APP_Data_t CAM_APP_Data;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
/*                                                                            */
/* Application entry point and main process loop                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void CAM_APP_Main(void)
{
    CFE_Status_t     status;
    CFE_SB_Buffer_t *SBBufPtr;

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(CAM_APP_PERF_ID);

    /*
    ** Perform application-specific initialization
    ** If the Initialization fails, set the RunStatus to
    ** CFE_ES_RunStatus_APP_ERROR and the App will not enter the RunLoop
    */
    status = CAM_APP_Init();
    if (status != CFE_SUCCESS)
    {
        CAM_APP_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /*
    ** Cam App Runloop
    */
    while (CFE_ES_RunLoop(&CAM_APP_Data.RunStatus) == true)
    {
        /*
        ** Performance Log Exit Stamp
        */
        CFE_ES_PerfLogExit(CAM_APP_PERF_ID);

        /* Pend on receipt of command packet */
        status = CFE_SB_ReceiveBuffer(&SBBufPtr, CAM_APP_Data.CommandPipe, CFE_SB_PEND_FOREVER);

        /*
        ** Performance Log Entry Stamp
        */
        CFE_ES_PerfLogEntry(CAM_APP_PERF_ID);

        if (status == CFE_SUCCESS)
        {
            CAM_APP_TaskPipe(SBBufPtr);

            //main clode in here
            
        }
        else
        {
            CFE_EVS_SendEvent(CAM_APP_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "CAM APP: SB Pipe Read Error, App Will Exit");

            CAM_APP_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    /*
    ** Performance Log Exit Stamp
    */
    CFE_ES_PerfLogExit(CAM_APP_PERF_ID);

    CFE_ES_ExitApp(CAM_APP_Data.RunStatus);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* Initialization                                                             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t CAM_APP_Init(void)
{
    CFE_Status_t status;
    char         VersionString[CAM_APP_CFG_MAX_VERSION_STR_LEN];

    /* Zero out the global data structure */
    memset(&CAM_APP_Data, 0, sizeof(CAM_APP_Data));

    CAM_APP_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;

    /*
    ** Initialize app configuration data
    */
    CAM_APP_Data.PipeDepth = CAM_APP_PIPE_DEPTH;

    strncpy(CAM_APP_Data.PipeName, "CAM_APP_CMD_PIPE", sizeof(CAM_APP_Data.PipeName));
    CAM_APP_Data.PipeName[sizeof(CAM_APP_Data.PipeName) - 1] = 0;

    /*
    ** Register the events
    */
    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Cam App: Error Registering Events, RC = 0x%08lX\n", (unsigned long)status);
    }
    else
    {
        /*
         ** Initialize housekeeping packet (clear user data area).
         */
        CFE_MSG_Init(CFE_MSG_PTR(CAM_APP_Data.HkTlm.TelemetryHeader), CFE_SB_ValueToMsgId(CAM_APP_HK_TLM_MID),
                     sizeof(CAM_APP_Data.HkTlm));

        /*
         ** Create Software Bus message pipe.
         */
        status = CFE_SB_CreatePipe(&CAM_APP_Data.CommandPipe, CAM_APP_Data.PipeDepth, CAM_APP_Data.PipeName);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(CAM_APP_CR_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Cam App: Error creating SB Command Pipe, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to Housekeeping request commands
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(CAM_APP_SEND_HK_MID), CAM_APP_Data.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(CAM_APP_SUB_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Cam App: Error Subscribing to HK request, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to ground command packets
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(CAM_APP_CMD_MID), CAM_APP_Data.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(CAM_APP_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Cam App: Error Subscribing to Commands, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Register Example Table(s)
        */
        status = CFE_TBL_Register(&CAM_APP_Data.TblHandles[0], "ExampleTable", sizeof(CAM_APP_ExampleTable_t),
                                  CFE_TBL_OPT_DEFAULT, CAM_APP_TblValidationFunc);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(CAM_APP_TABLE_REG_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Cam App: Error Registering Example Table, RC = 0x%08lX", (unsigned long)status);
        }
        else
        {
            status = CFE_TBL_Load(CAM_APP_Data.TblHandles[0], CFE_TBL_SRC_FILE, CAM_APP_TABLE_FILE);
        }

        CFE_Config_GetVersionString(VersionString, CAM_APP_CFG_MAX_VERSION_STR_LEN, "Cam App", CAM_APP_VERSION,
                                    CAM_APP_BUILD_CODENAME, CAM_APP_LAST_OFFICIAL);

        CFE_EVS_SendEvent(CAM_APP_INIT_INF_EID, CFE_EVS_EventType_INFORMATION, "Cam App Initialized.%s",
                          VersionString);
    }

    return status;
}
