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

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <stdint.h>
#include <math.h>
#include <pthread.h>

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
/* MPU NOOP commands                                                          */
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
/* A Mpu data reading and stop command                                                   */
/*                                                                              */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
int Stop_Command;
pthread_t thread;
float mpu[6] = {0, 0, 0, 0, 0, 0};

void *startloop(void *arg)
{
    int file;
    const char *bus = "/dev/i2c-1";
    if ((file = open(bus, O_RDWR)) < 0) {
        perror("Failed to open the I2C bus");
        exit(1);
    }
    ioctl(file, I2C_SLAVE, 0x68);

    const char buf[2] = {0x6B, 0};
    if (write(file, buf, 2) != 2) {
        perror("Failed to initialize MPU6050");
        exit(1);
    }
    
    while (1) 
    {
        if (Stop_Command == 1) 
        {
        break;
        } 
        else 
        {
            char reg = 0x3B;
            if (write(file, &reg, 1) != 1) {
                perror("Failed to set register address");
                exit(1);
            }
            char data[14];
            if (read(file, data, 14) != 14) {
                perror("Failed to read from the I2C bus");
                exit(1);
            }

            // Convert accelerometer data
            int16_t raw_ax = (data[0] << 8) | data[1];
            int16_t raw_ay = (data[2] << 8) | data[3];
            int16_t raw_az = (data[4] << 8) | data[5];

            // Convert gyroscope data
            int16_t raw_gx = (data[8] << 8) | data[9];
            int16_t raw_gy = (data[10] << 8) | data[11];
            int16_t raw_gz = (data[12] << 8) | data[13];
            
            //Divide raw value by sensitivity scale factor
            float gx = raw_gx / 131.0;
            float gy = raw_gy / 131.0;
            float gz = raw_gz / 131.0;
            
            float ax = raw_ax / 16384.0;
            float ay = raw_ay / 16384.0;
            float az = raw_az / 16384.0;

            mpu[0]=ax;
            mpu[1]=ay;
            mpu[2]=az;
            mpu[3]=gx;
            mpu[4]=gy;
            mpu[5]=gz;

            printf("\n x_accel %.3fg      y_accel %.3fg      z_accel %.3fg     \r", ax, ay, az);
            printf("\n x_gyro %.2f °/s    y_gyro %.2f °/s    z_gyro %.2f °/s    \r", gx, gy, gz);

            usleep(50000);
        }
    }
    close(file);
    pthread_exit(NULL);
    return mpu;
}

void stoploop(void)
{
    Stop_Command = 1;
}

CFE_Status_t MPU_APP_ReadingCmd(const MPU_APP_ReadingCmd_t *Msg)
{
    CFE_EVS_SendEvent(MPU_APP_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "MPU: RESET command");

    Stop_Command = 0;

    if(pthread_create(&thread, NULL, startloop, NULL))
    {
        perror("pthread_Create");
        exit(EXIT_FAILURE);
    }
    
    CFE_EVS_SendEvent(MPU_APP_READING_INF_EID, CFE_EVS_EventType_INFORMATION, "MPU: Reading Accel and Gyro\nax : %f  ay : %f  az : %f\ngx : %f  gy : %f  gz : %f",mpu[0], mpu[1], mpu[2], mpu[3], mpu[4], mpu[5]);
    return CFE_SUCCESS;
}

CFE_Status_t MPU_APP_Stop_ReadingCmd(const MPU_APP_Stop_ReadingCmd_t *Msg)
{
    stoploop();
    pthread_join(thread, NULL);
    CFE_EVS_SendEvent(MPU_APP_STOP_READING_INF_EID, CFE_EVS_EventType_INFORMATION, "MPU: Stop_Reading Accel and Gyro");
    
    return CFE_SUCCESS;
}
