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
 *   Specification for the MPU_APP command and telemetry
 *   message data types.
 *
 * @note
 *   Constants and enumerated types related to these message structures
 *   are defined in mpu_app_msgdefs.h.
 */
#ifndef MPU_APP_MSGSTRUCT_H
#define MPU_APP_MSGSTRUCT_H

/************************************************************************
 * Includes
 ************************************************************************/

#include "mpu_app_mission_cfg.h"
#include "mpu_app_msgdefs.h"
#include "cfe_msg_hdr.h"

/*************************************************************************/

/*
** The following commands all share the "NoArgs" format
**
** They are each given their own type name matching the command name, which
** allows them to change independently in the future without changing the prototype
** of the handler function
*/
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} MPU_APP_NoopCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} MPU_APP_ResetCountersCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} MPU_APP_ProcessCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t           CommandHeader; /**< \brief Command header */
    MPU_APP_DisplayParam_Payload_t Payload;
} MPU_APP_DisplayParamCmd_t;

typedef struct 
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command Header */
    MPU_APP_Reading_Payload_t Payload;     /**< \brief Telemetry payload */
} MPU_APP_ReadingCmd_t;

typedef struct 
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command Header */
} MPU_APP_Stop_ReadingCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;/**< \brief Command Header */
} MPU_APP_Currant_DataCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;/**< \brief Command Header */
} MPU_APP_Rewheel_OnCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;/**< \brief Command Header */
} MPU_APP_Rewheel_OffCmd_t;

/*************************************************************************/
/*
** Type definition (Mpu App housekeeping)
*/
/*************************************************************************/

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} MPU_APP_SendHkCmd_t;

typedef struct
{
    CFE_MSG_TelemetryHeader_t  TelemetryHeader; /**< \brief Telemetry header */
    MPU_APP_HkTlm_Payload_t Payload;         /**< \brief Telemetry payload */
} MPU_APP_HkTlm_t;

#endif /* MPU_APP_MSGSTRUCT_H */
