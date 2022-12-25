#ifndef _ECHOSERVER_H_
#define _ECHOSERVER_H_

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"

/* Remove the whole file if FreeRTOSIPConfig.h is set to exclude TCP. */
#if (ipconfigUSE_TCP != 1)
#error "ipconfigUSE_TCP should be enabled"
#endif

/* The maximum time to wait for a closing socket to close. */
#define tcpechoSHUTDOWN_DELAY (pdMS_TO_TICKS(5000))

/* The standard echo port number. */
#define tcpechoPORT_NUMBER 7

/* If ipconfigUSE_TCP_WIN is 1 then the Tx sockets will use a buffer size set by
ipconfigTCP_TX_BUFFER_LENGTH, and the Tx window size will be
configECHO_SERVER_TX_WINDOW_SIZE times the buffer size.  Note
ipconfigTCP_TX_BUFFER_LENGTH is set in FreeRTOSIPConfig.h as it is a standard
TCP/IP stack constant, whereas configECHO_SERVER_TX_WINDOW_SIZE is set in
FreeRTOSConfig.h as it is a demo application constant. */
#ifndef configECHO_SERVER_TX_WINDOW_SIZE
#define configECHO_SERVER_TX_WINDOW_SIZE 2
#endif

/* If ipconfigUSE_TCP_WIN is 1 then the Rx sockets will use a buffer size set by
ipconfigTCP_RX_BUFFER_LENGTH, and the Rx window size will be
configECHO_SERVER_RX_WINDOW_SIZE times the buffer size.  Note
ipconfigTCP_RX_BUFFER_LENGTH is set in FreeRTOSIPConfig.h as it is a standard
TCP/IP stack constant, whereas configECHO_SERVER_RX_WINDOW_SIZE is set in
FreeRTOSConfig.h as it is a demo application constant. */
#ifndef configECHO_SERVER_RX_WINDOW_SIZE
#define configECHO_SERVER_RX_WINDOW_SIZE 2
#endif

/*-----------------------------------------------------------*/

/*
 * Uses FreeRTOS+TCP to listen for incoming echo connections, creating a task
 * to handle each connection.
 */
void prvConnectionListeningTask(void *pvParameters);

/*
 * Created by the connection listening task to handle a single connection.
 */
void prvServerConnectionInstance(void *pvParameters);

/*-----------------------------------------------------------*/

/* Stores the stack size passed into vStartSimpleTCPServerTasks() so it can be
reused when the server listening task creates tasks to handle connections. */
// static uint16_t usUsedStackSize = 0

void vStartSimpleTCPServerTasks(void *pvParameters);

void prvConnectionListeningTask(void *pvParameters);

void prvServerConnectionInstance(void *pvParameters);

#endif