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
#include "cam_app_dispatch.h"
#include "cam_app_cmds.h"
#include "cam_app_eventids.h"
#include "cam_app_msgids.h"
#include "cam_app_msg.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Verify command packet length                                               */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
bool CAM_APP_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
{
    bool              result       = true;
    size_t            ActualLength = 0;
    CFE_SB_MsgId_t    MsgId        = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t FcnCode      = 0;

    CFE_MSG_GetSize(MsgPtr, &ActualLength);

    /*
    ** Verify the command packet length.
    */
    if (ExpectedLength != ActualLength)
    {
        CFE_MSG_GetMsgId(MsgPtr, &MsgId);
        CFE_MSG_GetFcnCode(MsgPtr, &FcnCode);

        CFE_EVS_SendEvent(CAM_APP_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid Msg length: ID = 0x%X,  CC = %u, Len = %u, Expected = %u",
                          (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode, (unsigned int)ActualLength,
                          (unsigned int)ExpectedLength);

        result = false;

        CAM_APP_Data.ErrCounter++;
    }

    return result;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* CAM ground commands                                                     */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void CAM_APP_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    /*
    ** Process CAM app ground commands
    */
    switch (CommandCode)
    {
        case CAM_APP_NOOP_CC:
            if (CAM_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(CAM_APP_NoopCmd_t)))
            {
                CAM_APP_NoopCmd((const CAM_APP_NoopCmd_t *)SBBufPtr);
            }
            break;

        case CAM_APP_RESET_COUNTERS_CC:
            if (CAM_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(CAM_APP_ResetCountersCmd_t)))
            {
                CAM_APP_ResetCountersCmd((const CAM_APP_ResetCountersCmd_t *)SBBufPtr);
            }
            break;

        case CAM_APP_PROCESS_CC:
            if (CAM_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(CAM_APP_ProcessCmd_t)))
            {
                CAM_APP_ProcessCmd((const CAM_APP_ProcessCmd_t *)SBBufPtr);
            }
            break;

        case CAM_APP_DISPLAY_PARAM_CC:
            if (CAM_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(CAM_APP_DisplayParamCmd_t)))
            {
                CAM_APP_DisplayParamCmd((const CAM_APP_DisplayParamCmd_t *)SBBufPtr);
            }
            break;
 
        case CAM_APP_SHOT_PERIOD_CC:
            if (CAM_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(CAM_APP_ShotPeriodCmd_t)))
            {
                CAM_APP_ShotPeriodCmd((const CAM_APP_ShotPeriodCmd_t *)SBBufPtr);
            }
            break;

        case CAM_APP_SHOT_START_CC:
            if (CAM_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(CAM_APP_ShotStartCmd_t)))
            {
                CAM_APP_ShotStartCmd((const CAM_APP_ShotStartCmd_t *)SBBufPtr);
            }
            break;

        case CAM_APP_SHOT_STOP_CC:
            if (CAM_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(CAM_APP_ShotStopCmd_t)))
            {
                CAM_APP_ShotStopCmd((const CAM_APP_ShotStopCmd_t *)SBBufPtr);
            }
            break;

        case CAM_APP_SECURITY_START_CC:
            if (CAM_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(CAM_APP_SecurityStartCmd_t)))
            {
                CAM_APP_SecurityStartCmd((const CAM_APP_SecurityStartCmd_t *)SBBufPtr);
            }
            break;

        case CAM_APP_SECURITY_STOP_CC:
            if (CAM_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(CAM_APP_SecurityStopCmd_t)))
            {
                CAM_APP_SecurityStopCmd((const CAM_APP_SecurityStopCmd_t *)SBBufPtr);
            }
            break;

        case CAM_APP_SECURITY_KEY_CC:
            if (CAM_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(CAM_APP_SecurityKeyCmd_t)))
            {
                CAM_APP_SecurityKeyCmd((const CAM_APP_SecurityKeyCmd_t *)SBBufPtr);
            }
            break;


        /* default case already found during FC vs length test */
        default:
            CFE_EVS_SendEvent(CAM_APP_CC_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid ground command code: CC = %d",
                              CommandCode);
            break;
        
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the CAM    */
/*     command pipe.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void CAM_APP_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case CAM_APP_CMD_MID:
            CAM_APP_ProcessGroundCommand(SBBufPtr);
            break;

        case CAM_APP_SEND_HK_MID:
            CAM_APP_SendHkCmd((const CAM_APP_SendHkCmd_t *)SBBufPtr);
            break;

        default:
            CFE_EVS_SendEvent(CAM_APP_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "CAM: invalid command packet,MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}
