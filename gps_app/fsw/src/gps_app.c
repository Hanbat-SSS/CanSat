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
 *   This file contains the source code for the Gps App.
 */

/*
** Include Files:
*/
#include "gps_app.h"
#include "gps_app_cmds.h"
#include "gps_app_utils.h"
#include "../inc/gps_app_eventids.h"
#include "gps_app_dispatch.h"
#include "gps_app_tbl.h"
#include "gps_app_version.h"

/*
** global data
*/
GPS_APP_Data_t GPS_APP_Data;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
/*                                                                            */
/* Application entry point and main process loop                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void GPS_APP_Main(void)
{
    CFE_Status_t     status;
    CFE_SB_Buffer_t *SBBufPtr;

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(GPS_APP_PERF_ID);

    /*
    ** Perform application-specific initialization
    ** If the Initialization fails, set the RunStatus to
    ** CFE_ES_RunStatus_APP_ERROR and the App will not enter the RunLoop
    */
    status = GPS_APP_Init();
    if (status != CFE_SUCCESS)
    {
        GPS_APP_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /*
    ** Gps App Runloop
    */
    while (CFE_ES_RunLoop(&GPS_APP_Data.RunStatus) == true)
    {
        /*
        ** Performance Log Exit Stamp
        */
        CFE_ES_PerfLogExit(GPS_APP_PERF_ID);

        /* Pend on receipt of command packet */
        status = CFE_SB_ReceiveBuffer(&SBBufPtr, GPS_APP_Data.CommandPipe, CFE_SB_PEND_FOREVER);

        /*
        ** Performance Log Entry Stamp
        */
        CFE_ES_PerfLogEntry(GPS_APP_PERF_ID);

        if (status == CFE_SUCCESS)
        {
            GPS_APP_TaskPipe(SBBufPtr);
        }
        else
        {
            CFE_EVS_SendEvent(GPS_APP_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "GPS APP: SB Pipe Read Error, App Will Exit");

            GPS_APP_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    /*
    ** Performance Log Exit Stamp
    */
    CFE_ES_PerfLogExit(GPS_APP_PERF_ID);

    CFE_ES_ExitApp(GPS_APP_Data.RunStatus);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* Initialization                                                             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t GPS_APP_Init(void)
{
    CFE_Status_t status;
    char         VersionString[GPS_APP_CFG_MAX_VERSION_STR_LEN];

    /* Zero out the global data structure */
    memset(&GPS_APP_Data, 0, sizeof(GPS_APP_Data));

    GPS_APP_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;

    /*
    ** Initialize app configuration data
    */
    GPS_APP_Data.PipeDepth = GPS_APP_PIPE_DEPTH;

    strncpy(GPS_APP_Data.PipeName, "GPS_APP_CMD_PIPE", sizeof(GPS_APP_Data.PipeName));
    GPS_APP_Data.PipeName[sizeof(GPS_APP_Data.PipeName) - 1] = 0;

    /*
    ** Register the events
    */
    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Gps App: Error Registering Events, RC = 0x%08lX\n", (unsigned long)status);
    }
    else
    {
        /*
         ** Initialize housekeeping packet (clear user data area).
         */
        CFE_MSG_Init(CFE_MSG_PTR(GPS_APP_Data.HkTlm.TelemetryHeader), CFE_SB_ValueToMsgId(GPS_APP_HK_TLM_MID),
                     sizeof(GPS_APP_Data.HkTlm));

        /*
         ** Create Software Bus message pipe.
         */
        status = CFE_SB_CreatePipe(&GPS_APP_Data.CommandPipe, GPS_APP_Data.PipeDepth, GPS_APP_Data.PipeName);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(GPS_APP_CR_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Gps App: Error creating SB Command Pipe, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to Housekeeping request commands
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(GPS_APP_SEND_HK_MID), GPS_APP_Data.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(GPS_APP_SUB_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Gps App: Error Subscribing to HK request, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to ground command packets
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(GPS_APP_CMD_MID), GPS_APP_Data.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(GPS_APP_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Gps App: Error Subscribing to Commands, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Register Example Table(s)
        */
        status = CFE_TBL_Register(&GPS_APP_Data.TblHandles[0], "ExampleTable", sizeof(GPS_APP_ExampleTable_t),
                                  CFE_TBL_OPT_DEFAULT, GPS_APP_TblValidationFunc);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(GPS_APP_TABLE_REG_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Gps App: Error Registering Example Table, RC = 0x%08lX", (unsigned long)status);
        }
        else
        {
            status = CFE_TBL_Load(GPS_APP_Data.TblHandles[0], CFE_TBL_SRC_FILE, GPS_APP_TABLE_FILE);
        }

        CFE_Config_GetVersionString(VersionString, GPS_APP_CFG_MAX_VERSION_STR_LEN, "Gps App", GPS_APP_VERSION,
                                    GPS_APP_BUILD_CODENAME, GPS_APP_LAST_OFFICIAL);

        CFE_EVS_SendEvent(GPS_APP_INIT_INF_EID, CFE_EVS_EventType_INFORMATION, "Gps App Initialized.%s",
                          VersionString);
    }

    return status;
}
