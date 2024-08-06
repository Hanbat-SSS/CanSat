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
#include <sys/time.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* defines for PID control                                                    */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
#define MPU6050_ADDR 0x68
#define RAD_TO_DEG 57.295779513082320876798154814105

// Define the PWM pins
#define PWM_PIN_X 1   // GPIO18 (wiringPi pin 1) - X축 모터
#define PWM_PIN_Y 23  // GPIO13 (wiringPi pin 23) - Y축 모터

// Define the PWM range and values
#define PWM_RANGE 2000
#define PWM_MIN 135
#define PWM_MAX 165
#define PWM_NEUTRAL 150
#define PWM_STABLE_RANGE 3.0  // +-3도 내의 각도에 대해 중립 PWM 값 사용

// PID constants
#define KP 0.825 // Proportional gain
#define KI 0.000004  // Integral gain
#define KD 0.179 // Derivative gain

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

int stop_MPU;
int stop_RW;
int fd;
pthread_t thread;
int16_t offset[3] = {-22, 15, -4};

typedef struct {
    float setpoint;
    float integral;
    float last_error;
} PIDController;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* A Fuction for the setup the Sensor(MPU6050)                                */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void *setup_mpu6050(void) 
{
    // WiringPi initialization
    wiringPiSetup();
    
    // I2C initialization
    fd = wiringPiI2CSetup(MPU6050_ADDR);
    
    // Power Management
    wiringPiI2CWriteReg8(fd, 107, 0);
    
    // Register 26(DLPF)
    for (uint8_t i = 2; i <= 7; i++) {
        wiringPiI2CWriteReg8(fd, 26, i << 3 | 0x03);
    }
    
    // Register 27(Gyro scale : -2000~+2000 scale)
    wiringPiI2CWriteReg8(fd, 27, 3 << 3);
    
    // Register 28(Accel : -2g ~ +2g --> basic scale)
    wiringPiI2CWriteReg8(fd, 28, 0);

    return 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* A Fuction that read mpu6050 Sensor's data Value                            */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void read_mpu6050_data(float* angle)
{
    // 파일 포인터 초기화
    FILE *file = fopen("/home/test2/cFS/Data/angle_data.txt", "w");
    if (file == NULL) {
        printf("Failed to open file for writing\n");
        return;  
    }

    while(stop_MPU == 0)
    {
        if(stop_MPU == 1)
        {
            break;
        }
        else{
            uint8_t i;
            static int16_t acc_raw[3] = {0}, gyro_raw[3] = {0};
            float vec;
            static unsigned long p = 0;

            // Get Accel
            for (i = 0; i < 3; i++) {
                acc_raw[i] = (wiringPiI2CReadReg8(fd, 59 + i*2) << 8) | wiringPiI2CReadReg8(fd, 60 + i*2);
            }

            // Get Gyro
            for (i = 0; i < 3; i++) {
                int16_t temp = (wiringPiI2CReadReg8(fd, 67 + i*2) << 8) | wiringPiI2CReadReg8(fd, 68 + i*2);
                gyro_raw[i] = gyro_raw[i] * 0.8 + 0.2 * (temp - offset[i]);
            }

            // Get DT
            unsigned long c = micros();
            float dt = (c - p) * 0.000001F;
            p = c;

            // Gyro Rate
            float gyro_rate[3];
            for (i = 0; i < 3; i++) gyro_rate[i] = gyro_raw[i] / 16.4 * dt;

            // Calculate
            vec = sqrt(pow(acc_raw[0], 2) + pow(acc_raw[2], 2));
            angle[0] = (angle[0] + gyro_rate[0]) * 0.98
                + atan2(acc_raw[1], vec) * RAD_TO_DEG * 0.02;
            vec = sqrt(pow(acc_raw[1], 2) + pow(acc_raw[2], 2));
            angle[1] = (angle[1] - gyro_rate[1]) * 0.98
                + atan2(acc_raw[0], vec) * RAD_TO_DEG * 0.02;

            // Update Z angle
            angle[2] += gyro_rate[2];

            printf("Angle X: %.2f\tAngle Y: %.2f", angle[0], angle[1]);

            // Debugging output
            printf("Angle X: %.2f\tAngle Y: %.2f", angle[0], angle[1]);

            // 각도 데이터를 파일에 저장
            fprintf(file, "%.2f %.2f\n", angle[0], angle[1]);
            fflush(file);  // 실시간으로 데이터를 쓰기 위해 버퍼를 비움

            delay(10);  // 10ms delay
        }
    }
    fclose(file);
}

void *read_mpu6050_data_thread(void *arg)
{
    float angle[3];
    read_mpu6050_data(angle);
    return NULL;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* A Fuction that pid compute                                                 */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
float pid_compute(PIDController* pid, float measured_value)
{
    float error = pid->setpoint - measured_value;
    pid->integral += error;
    float derivative = error - pid->last_error;
    pid->last_error = error;
    
    return KP * error + KI * pid->integral + KD * derivative;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* A Fuction for pid value initializing                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void pid_init(PIDController* pid, float setpoint) {
    pid->setpoint = setpoint;
    pid->integral = 0;
    pid->last_error = 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* signal handler function                                                    */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void signal_handler(int signum) {
    printf("Interrupt signal received. Setting PWM to 0 and exiting.\n");

    // Set PWM to 0 (or a safe value) before exiting
    pwmWrite(PWM_PIN_X, 0);
    pwmWrite(PWM_PIN_Y, 0);
    
    // Exit program
    exit(signum);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Function for control Reaction Wheel                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void *RW_Control(void *arg) {
    // Register signal and signal handler
    signal(SIGINT, signal_handler);

    setup_mpu6050();

    float angle[3] = {0};
    PIDController pid_x, pid_y;
    
    // Initialize PID controllers
    pid_init(&pid_x, 0);  // Target angle for X-axis
    pid_init(&pid_y, 0);  // Target angle for Y-axis
    
    // Setup wiringPi for PWM
    if (wiringPiSetup() == -1) {
        printf("wiringPi setup failed!\n");
        return NULL;  // 반환형에 맞게 수정
    }

    // Set PWM mode, range, and clock for both X and Y axis motors
    pinMode(PWM_PIN_X, PWM_OUTPUT);
    pinMode(PWM_PIN_Y, PWM_OUTPUT);
    pwmSetMode(PWM_MODE_MS);  // Use mark-space mode
    pwmSetRange(PWM_RANGE);
    pwmSetClock(192);  // This sets the frequency to 50Hz

    while (1) {
        if(stop_RW == 1) {
            break;
        } else {
            read_mpu6050_data(angle);
            
            // Compute PWM values using PID control
            int pwm_value_x, pwm_value_y;
            
            // Apply PID control only if the angle is outside the stable range
            if (fabs(angle[0]) > PWM_STABLE_RANGE) {
                pwm_value_x = PWM_NEUTRAL - (int)pid_compute(&pid_x, angle[0]);
            } else {
                pwm_value_x = PWM_NEUTRAL;
            }

            if (fabs(angle[1]) > PWM_STABLE_RANGE) {
                pwm_value_y = PWM_NEUTRAL - (int)pid_compute(&pid_y, angle[1]);
            } else {
                pwm_value_y = PWM_NEUTRAL;
            }

            // Clamp PWM values to valid range
            if (pwm_value_x > PWM_MAX) pwm_value_x = PWM_MAX;
            if (pwm_value_x < PWM_MIN) pwm_value_x = PWM_MIN;
            if (pwm_value_y > PWM_MAX) pwm_value_y = PWM_MAX;
            if (pwm_value_y < PWM_MIN) pwm_value_y = PWM_MIN;
            
            // Apply PWM values to motors
            pwmWrite(PWM_PIN_X, pwm_value_x);
            pwmWrite(PWM_PIN_Y, pwm_value_y);

            delay(10);  // 10ms delay
        }
    }
    return NULL;  // 반환형에 맞게 수정
}

void stopMPU(void)
{
    stop_MPU = 1;
}

void stopRW(void)
{
    stop_RW = 1;
}

CFE_Status_t MPU_APP_ReadingCmd(const MPU_APP_ReadingCmd_t *Msg)
{
    stop_MPU = 0;

    if(pthread_create(&thread, NULL, read_mpu6050_data_thread, NULL))
    {
        perror("pthread_Create");
        exit(EXIT_FAILURE);
    }
    
    CFE_EVS_SendEvent(MPU_APP_READING_INF_EID, CFE_EVS_EventType_INFORMATION, "MPU: Reading Angles");
    return CFE_SUCCESS;
}

CFE_Status_t MPU_APP_Stop_ReadingCmd(const MPU_APP_Stop_ReadingCmd_t *Msg)
{
    stopMPU();
    pthread_join(thread, NULL);
    CFE_EVS_SendEvent(MPU_APP_STOP_READING_INF_EID, CFE_EVS_EventType_INFORMATION, "MPU: Stop_Reading Accel and Gyro");
    
    return CFE_SUCCESS;
}

CFE_Status_t MPU_APP_Rewheel_OnCmd(const MPU_APP_Rewheel_OnCmd_t *Msg)
{
    stop_RW = 0;

    if(pthread_create(&thread, NULL, RW_Control, NULL))
    {
        perror("pthread_Create");
        exit(EXIT_FAILURE);
    }
    CFE_EVS_SendEvent(MPU_APP_REWHEEL_ON_INF_EID, CFE_EVS_EventType_INFORMATION, "MPU: Reaction Wheel ON");

    return CFE_SUCCESS;
}

CFE_Status_t MPU_APP_Rewheel_OffCmd(const MPU_APP_Rewheel_OffCmd_t *Msg)
{
    stopRW();
    pthread_join(thread, NULL);
    CFE_EVS_SendEvent(MPU_APP_REWHEEL_OFF_INF_EID, CFE_EVS_EventType_INFORMATION, "MPU: Reaction Wheel OFF");

    return CFE_SUCCESS;
}
