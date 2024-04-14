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
 *
 * Main header file for the Cam application
 */

#ifndef CAM_APP_H
#define CAM_APP_H

/*
** Required header files.
*/
#include "cfe.h"
#include "cfe_config.h"

#include "cam_app_mission_cfg.h"
#include "cam_app_platform_cfg.h"

#include "cam_app_perfids.h"
#include "cam_app_msgids.h"
#include "cam_app_msg.h"

/************************************************************************
** Type Definitions
*************************************************************************/

/*
** Global Data
*/
typedef struct
{
    /*
    ** Command interface counters...
    */
    uint8 CmdCounter;
    uint8 ErrCounter;

    /*
    ** Housekeeping telemetry packet...
    */
    CAM_APP_HkTlm_t HkTlm;

    /*
    ** Run Status variable used in the main processing loop
    */
    uint32 RunStatus;

    /*
    ** Operational data (not reported in housekeeping)...
    */
    CFE_SB_PipeId_t CommandPipe;

    /*
    ** Initialization data (not reported in housekeeping)...
    */
    char   PipeName[CFE_MISSION_MAX_API_LEN];
    uint16 PipeDepth;

    CFE_TBL_Handle_t TblHandles[CAM_APP_NUMBER_OF_TABLES];
} CAM_APP_Data_t;

/*
** Global data structure
*/
extern CAM_APP_Data_t CAM_APP_Data;

/****************************************************************************/
/*
** Local function prototypes.
**
** Note: Except for the entry point (CAM_APP_Main), these
**       functions are not called from any other source module.
*/
void         CAM_APP_Main(void);
CFE_Status_t CAM_APP_Init(void);

#endif /* CAM_APP_H */
