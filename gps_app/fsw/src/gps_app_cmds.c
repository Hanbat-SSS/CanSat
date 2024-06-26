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
 *   This file contains the source code for the Gps App Ground Command-handling functions
 */

/*
** Include Files:
*/
#include "gps_app.h"
#include "gps_app_cmds.h"
#include "gps_app_msgids.h"
#include "../inc/gps_app_eventids.h"
#include "gps_app_version.h"
#include "gps_app_tbl.h"
#include "gps_app_utils.h"
#include "gps_app_msg.h"

#include "pthread.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a task telemetry request */
/*         from the housekeeping task. This function will gather the Apps     */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t GPS_APP_SendHkCmd(const GPS_APP_SendHkCmd_t *Msg)
{
    int i;

    /*
    ** Get command execution counters...
    */
    GPS_APP_Data.HkTlm.Payload.CommandErrorCounter = GPS_APP_Data.ErrCounter;
    GPS_APP_Data.HkTlm.Payload.CommandCounter      = GPS_APP_Data.CmdCounter;

    /*
    ** Send housekeeping telemetry packet...
    */
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(GPS_APP_Data.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(GPS_APP_Data.HkTlm.TelemetryHeader), true);

    /*
    ** Manage any pending table loads, validations, etc.
    */
    for (i = 0; i < GPS_APP_NUMBER_OF_TABLES; i++)
    {
        CFE_TBL_Manage(GPS_APP_Data.TblHandles[i]);
    }

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* GPS NOOP commands                                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t GPS_APP_NoopCmd(const GPS_APP_NoopCmd_t *Msg)
{
    GPS_APP_Data.CmdCounter++;

    CFE_EVS_SendEvent(GPS_APP_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "GPS: NOOP command %s",
                      GPS_APP_VERSION);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry.                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t GPS_APP_ResetCountersCmd(const GPS_APP_ResetCountersCmd_t *Msg)
{
    GPS_APP_Data.CmdCounter = 0;
    GPS_APP_Data.ErrCounter = 0;

    CFE_EVS_SendEvent(GPS_APP_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "GPS: RESET command");

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function Process Ground Station Command                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t GPS_APP_ProcessCmd(const GPS_APP_ProcessCmd_t *Msg)
{
    CFE_Status_t               status;
    void *                     TblAddr;
    GPS_APP_ExampleTable_t *TblPtr;
    const char *               TableName = "GPS_APP.ExampleTable";

    /* Gps Use of Example Table */

    status = CFE_TBL_GetAddress(&TblAddr, GPS_APP_Data.TblHandles[0]);

    if (status < CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Gps App: Fail to get table address: 0x%08lx", (unsigned long)status);
        return status;
    }

    TblPtr = TblAddr;
    CFE_ES_WriteToSysLog("Gps App: Example Table Value 1: %d  Value 2: %d", TblPtr->Int1, TblPtr->Int2);

    GPS_APP_GetCrc(TableName);

    status = CFE_TBL_ReleaseAddress(GPS_APP_Data.TblHandles[0]);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Gps App: Fail to release table address: 0x%08lx", (unsigned long)status);
        return status;

    }
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* A simple example command that displays a passed-in value                   */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t GPS_APP_DisplayParamCmd(const GPS_APP_DisplayParamCmd_t *Msg)
{
    CFE_EVS_SendEvent(GPS_APP_VALUE_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "GPS_APP: ValU32=%lu, ValI16=%d, ValStr=%s", (unsigned long)Msg->Payload.ValU32,
                      (int)Msg->Payload.ValI16, Msg->Payload.ValStr);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                              */
/* A Gps data parsing command                                                   */
/*                                                                              */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

int Stop_Command;
pthread_t thread;
int end_of_loop = 0;
float coordinate[2] = {0,0};

void *startloop(void *arg)
{
  int fd;
  struct termios newt;
  char *nmea_line;
  char *parser;
  double latitude;
  float longitude;

  fd = open("/dev/ttyS0", O_RDWR | O_NONBLOCK);
  if (fd >= 0)
  {
    tcgetattr(fd, &newt);
    newt.c_iflag &= ~IGNBRK;
    newt.c_iflag &= ~(IXON | IXOFF | IXANY);
    newt.c_oflag = 0;

    newt.c_cflag |= (CLOCAL | CREAD);
    newt.c_cflag |= CS8;
    newt.c_cflag &= ~(PARENB | PARODD);
    newt.c_cflag &= ~CSTOPB;

    newt.c_lflag = 0;

    newt.c_cc[VMIN]  = 0;
    newt.c_cc[VTIME] = 0;
    tcsetattr(fd, TCSANOW, &newt);

  usleep(100000);

  while(end_of_loop == 0)
  {
    char read_buffer[1000];
    read(fd, &read_buffer,1000);
    //printf("|%s|", r_buf);

    nmea_line = strtok(read_buffer, "\n");

    while (nmea_line != NULL)
    {

      parser = strstr(nmea_line, "$GPRMC");
      if (parser != NULL)
      {
        char *token = strtok(nmea_line, ",");
        int index = 0;
        while (token != NULL)
        {
          if (index == 3)
          {
            latitude = atof(token);
            printf("found latitude: %s %f\n", token, latitude);
            coordinate[0] = latitude;
          }
          if (index == 5)
          {
            longitude = atof(token);
            printf("found longitude: %s %f\n", token, longitude);
            coordinate[1] = longitude;
          }
          token = strtok(NULL, ",");
          index++;
        }
      }
      nmea_line = strtok(NULL, "\n");
    }
    usleep(500000);
  }
  close(fd);
  return 0;
  }
  else
  {
    printf("Port cannot be opened");
    return 0;
  }
  return coordinate;
}

void stoploop(void)
{
    end_of_loop = 0;
}

CFE_Status_t GPS_APP_ParsingCmd(const GPS_APP_ParsingCmd_t *Msg)
{
    if(pthread_create(&thread, NULL, startloop, NULL))
    {
        perror("pthread_Create");
        exit(EXIT_FAILURE);
    }

    CFE_EVS_SendEvent(GPS_APP_PARSING_INF_EID, CFE_EVS_EventType_INFORMATION, "GPS: GPS Parsing Start");
    return CFE_SUCCESS;
}

CFE_Status_t GPS_APP_UnparsingCmd(const GPS_APP_UnparsingCmd_t *Msg)
{
     stoploop();
    pthread_join(thread, NULL);
    CFE_EVS_SendEvent(GPS_APP_UNPARSING_INF_EID, CFE_EVS_EventType_INFORMATION, "GPS: Stop GPS Parsing");
    
    return CFE_SUCCESS;
}

CFE_Status_t GPS_APP_Currant_DataCmd(const GPS_APP_Currant_DataCmd_t *Msg)
{
    CFE_EVS_SendEvent(GPS_APP_CURRANT_DATA_INF_EID, CFE_EVS_EventType_INFORMATION, "\nLatitude : %f\nLongitude : %f", coordinate[0], coordinate[1]);

    return CFE_SUCCESS;
}