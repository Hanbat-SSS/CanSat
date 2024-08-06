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
 *   Specification for the CAM_APP command and telemetry
 *   message constant definitions.
 *
 *  For CAM_APP this is only the function/command code definitions
 */
#ifndef CAM_APP_MSGDEFS_H
#define CAM_APP_MSGDEFS_H

#include "common_types.h"
#include "cam_app_fcncodes.h"

typedef struct CAM_APP_DisplayParam_Payload
{
    uint32 ValU32;                            /**< 32 bit unsigned integer value */
    int16  ValI16;                            /**< 16 bit signed integer value */
    char   ValStr[CAM_APP_STRING_VAL_LEN]; /**< An example string */
} CAM_APP_DisplayParam_Payload_t;


typedef struct CAM_APP_ShotPeriod_Payload
{
    uint16 Period;
} CAM_APP_ShotPeriod_Payload_t;

typedef struct CAM_APP_SeucirtyKey_Payload
{
    char Key[32];
} CAM_APP_SecurityKey_Payload_t;

/*************************************************************************/
/*
** Type definition (Cam App housekeeping)
*/

typedef struct CAM_APP_HkTlm_Payload
{
    uint8 CommandErrorCounter;
    uint8 CommandCounter;
    uint16 spare[2];
} CAM_APP_HkTlm_Payload_t;

#endif
