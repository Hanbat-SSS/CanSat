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
#include "time.h"
#include "unistd.h"
#include "pthread.h"
#include <stdbool.h>

/* Encypt Library */
#include "common_fnc.h"
#include "security.h"


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

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* A simple example command that Setting Shot Period                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

uint16 Period = 10;

CFE_Status_t CAM_APP_ShotPeriodCmd(const CAM_APP_ShotPeriodCmd_t *Msg)
{
    CFE_EVS_SendEvent(CAM_APP_SHOT_PERIOD_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "CAM_APP: Shot Period is set to %d(sec)", (int)Msg->Payload.Period);
    
    Period = (int)Msg->Payload.Period;
    return Period;

    return CFE_SUCCESS; 
}

byte Global_Security_Key[32] = {0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 };  // 전역 변수로 선언하여 키를 저장

CFE_Status_t CAM_APP_SecurityKeyCmd(const CAM_APP_SecurityKeyCmd_t *Msg)
{
    // 수신한 키를 이벤트 로그에 출력
    char received_key_string[96] = {0};
    for (int i = 0; i < 32; i++)
    {
        sprintf(&received_key_string[i * 3], "%02x ", Msg->Payload.Key[i]);
    }

    // 수신한 키를 전역 변수에 저장
    memcpy(Global_Security_Key, Msg->Payload.Key, sizeof(Global_Security_Key));

    // 저장된 키를 이벤트 로그에 출력
    char stored_key_string[96] = {0};
    for (int i = 0; i < 32; i++)
    {
        sprintf(&stored_key_string[i * 3], "%02x ", Global_Security_Key[i]);
    }

    // 수신된 키와 저장된 키를 비교
    if (memcmp(Msg->Payload.Key, Global_Security_Key, sizeof(Global_Security_Key)) == 0)
    {
        CFE_EVS_SendEvent(CAM_APP_SECURITY_KEY_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "CAM_APP: Security Key has been successfully stored and verified.");
    }
    else
    {
        CFE_EVS_SendEvent(CAM_APP_SECURITY_KEY_INF_EID, CFE_EVS_EventType_ERROR,
                          "CAM_APP: Security Key mismatch! Verification failed.");
    }

    // 최종적으로 저장된 키 값을 출력
    char final_key_string[96] = {0};
    for (int i = 0; i < 32; i++)
    {
        sprintf(&final_key_string[i * 3], "%02x ", Global_Security_Key[i]);
    }
    CFE_EVS_SendEvent(CAM_APP_SECURITY_KEY_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "CAM_APP: Final Key: [%s]", final_key_string);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* A Cam_app Start, Stop Process to using thread                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/


int Stop_Command;
int Start_Security_Command;
pthread_t thread;

#define BLOCK_SIZE 16

void *startloop(void *arg)
{
    while (1)
    {
        if (Stop_Command == 1)
        {
            break;
        }
        else
        {
            time_t t = time(NULL);
            struct tm *now = localtime(&t);

            char timestamp[20];
            strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H:%M:%S", now);

            char original_filename[100];
            sprintf(original_filename, "/home/cansat/Photo/Original_Photo/photo_%s.jpeg", timestamp);

            char command[200];
            sprintf(command, "libcamera-still -o %s -t 1000 --width 320 --height 240", original_filename);

            printf("Executing command: %s\n", command);

            system(command);

            if (Start_Security_Command == 1)
            {
                // 이미지 데이터 읽기
                byte* data = NULL;
                size_t size = 0;
                if (!read_image_data(&data, &size, original_filename))
                {
                    printf("Failed to read image data\n");
                    continue;
                }

                // 패딩 추가
                pad_data(&data, &size);
                printf("Padding added to data\n");
                CFE_EVS_SendEvent(CAM_APP_SECURITY_PROCESSING_INF_EID, CFE_EVS_EventType_INFORMATION,
                                  "CAM_APP: Padding added to data");

                // 암호화 키
                byte round_keys[(Nr+1) * Nb];

                // 암호화 진행
                encrypt_data(data, size, Global_Security_Key, round_keys);
                printf("Data encryption completed\n");
                CFE_EVS_SendEvent(CAM_APP_SECURITY_PROCESSING_INF_EID, CFE_EVS_EventType_INFORMATION,
                                  "CAM_APP: Data encryption completed");

                // 암호화된 데이터를 16진수 문자열로 변환하여 별도의 파일에 저장
                char encrypted_filename[100];
                sprintf(encrypted_filename, "/home/cansat/Photo/Encrypt_Photo/encrypted_photo_%s.enc", timestamp);

                FILE *encrypted_file = fopen(encrypted_filename, "w");
                if (encrypted_file != NULL) 
                {
                    for (size_t i = 0; i < size; i++) 
                    {
                        fprintf(encrypted_file, "%02x", data[i]);
                    }
                    fclose(encrypted_file);
                    printf("Encrypted data saved: %s\n", encrypted_filename);
                    CFE_EVS_SendEvent(CAM_APP_SECURITY_PROCESSING_INF_EID, CFE_EVS_EventType_INFORMATION,
                                      "CAM_APP: Encrypted data saved: %s", encrypted_filename);
                } 
                else 
                {
                    printf("Failed to open encrypted file: %s\n", encrypted_filename);
                    CFE_EVS_SendEvent(CAM_APP_SECURITY_PROCESSING_INF_EID, CFE_EVS_EventType_ERROR,
                                      "CAM_APP: Failed to open encrypted file: %s", encrypted_filename);
                    free(data);
                    continue;
                }

                
                // 복호화
                byte* encrypted_data = NULL;
                size_t encrypted_size = 0;
                read_encrypted_data(&encrypted_data, &encrypted_size, encrypted_filename);
                if (encrypted_data != NULL)
                {
                    printf("Encrypted data read successfully: %s\n", encrypted_filename);                   
                }
                else
                {
                    printf("Failed to read encrypted data: %s\n", encrypted_filename);
                    free(data);
                    continue;
                }

                // 복호화 진행
                decrypt_data(encrypted_data, encrypted_size, Global_Security_Key, round_keys);
                printf("Data decryption completed\n");                

                // 패딩 제거
                unpad_data(&encrypted_data, &encrypted_size);
                printf("Padding removed from data\n");

                // 복호화된 데이터를 파일로 저장
                char decrypted_filename[100];
                sprintf(decrypted_filename, "/home/cansat/Photo/Decrypt_Photo/decrypted_photo_%s.jpeg", timestamp);
                write_image_data(encrypted_data, encrypted_size, decrypted_filename);
                printf("Decrypted data saved: %s\n", decrypted_filename);
                

                // 메모리 해제
                free(data);
                //free(encrypted_data);
            }

            sleep(Period);
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
    stoploop();
    pthread_join(thread, NULL);
    CFE_EVS_SendEvent(CAM_APP_SHOT_STOP_INF_EID, CFE_EVS_EventType_INFORMATION, "CAM: Send Stop_Command");
    
    return CFE_SUCCESS;
}

CFE_Status_t CAM_APP_SecurityStartCmd(const CAM_APP_SecurityStartCmd_t *Msg)
{
    Start_Security_Command = 1;
    CFE_EVS_SendEvent(CAM_APP_SECURITY_START_INF_EID, CFE_EVS_EventType_INFORMATION, "CAM: Security_Start_Command");

    return CFE_SUCCESS;
}

CFE_Status_t CAM_APP_SecurityStopCmd(const CAM_APP_SecurityStopCmd_t *Msg)
{
    Start_Security_Command = 0;
    CFE_EVS_SendEvent(CAM_APP_SECURITY_STOP_INF_EID, CFE_EVS_EventType_INFORMATION, "CAM: Security_Stop_Command");

    return CFE_SUCCESS;
}

