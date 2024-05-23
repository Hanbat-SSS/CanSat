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
 * Define Mpu App Events IDs
 */

#ifndef MPU_APP_EVENTS_H
#define MPU_APP_EVENTS_H

#define MPU_APP_RESERVED_EID      0
#define MPU_APP_INIT_INF_EID      1
#define MPU_APP_CC_ERR_EID        2
#define MPU_APP_NOOP_INF_EID      3
#define MPU_APP_RESET_INF_EID     4
#define MPU_APP_MID_ERR_EID       5
#define MPU_APP_CMD_LEN_ERR_EID   6
#define MPU_APP_PIPE_ERR_EID      7
#define MPU_APP_VALUE_INF_EID     8
#define MPU_APP_CR_PIPE_ERR_EID   9
#define MPU_APP_SUB_HK_ERR_EID    10
#define MPU_APP_SUB_CMD_ERR_EID   11
#define MPU_APP_TABLE_REG_ERR_EID 12
#define MPU_APP_READING_INF_EID   13
#define MPU_APP_STOP_READING_INF_EID 14
#define MPU_APP_CURRANT_DATA_INF_EID 15

#endif /* MPU_APP_EVENTS_H */
