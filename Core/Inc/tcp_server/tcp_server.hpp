#ifndef _ABSTRACT_TCP_SERVER_HPP_
#define _ABSTRACT_TCP_SERVER_HPP_

#ifdef __cplusplus
extern "C" {
#endif

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "portmacro.h"
#include "semphr.h"
#include "task.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"

/* The maximum time to wait for a closing socket to close. */
#define tcpechoSHUTDOWN_DELAY (pdMS_TO_TICKS(5000))

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

typedef void (*ReceiveCallback)(Socket_t socket, uint8_t *data_ptr,
                                int32_t data_size);

struct TcpConnection {
  Socket_t socket;
  ReceiveCallback onReceive;
};

class TcpServer {

private:
  Socket_t xListeningSocket, xConnectedSocket;
  struct freertos_sockaddr xBindAddress, xClient;
  const TickType_t xReceiveTimeOut = portMAX_DELAY;
  socklen_t xSize = sizeof(xClient);
  const BaseType_t xBacklog = 20; // num of available opened sockets

  uint16_t _port;
  UBaseType_t _taskPriority;
  ReceiveCallback onReceive;

public:
  TcpServer(uint16_t _port, ReceiveCallback onReceive,
            UBaseType_t _taskPriority = osPriorityNormal) {
    this->_port = _port;
    this->onReceive = onReceive;
    this->_taskPriority = _taskPriority;

#if (ipconfigUSE_TCP_WIN == 1)
    WinProperties_t xWinProps;

    /* Fill in the buffer and window sizes that will be used by the socket. */
    xWinProps.lTxBufSize = ipconfigTCP_TX_BUFFER_LENGTH;
    xWinProps.lTxWinSize = configECHO_SERVER_TX_WINDOW_SIZE;
    xWinProps.lRxBufSize = ipconfigTCP_RX_BUFFER_LENGTH;
    xWinProps.lRxWinSize = configECHO_SERVER_RX_WINDOW_SIZE;
#endif /* ipconfigUSE_TCP_WIN */

    xListeningSocket = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_STREAM,
                                       FREERTOS_IPPROTO_TCP);
    configASSERT(xListeningSocket != FREERTOS_INVALID_SOCKET);

    /* Set a time out so accept() will just wait for a connection. */
    FreeRTOS_setsockopt(xListeningSocket, 0, FREERTOS_SO_RCVTIMEO,
                        &xReceiveTimeOut, sizeof(xReceiveTimeOut));

/* Set the window and buffer sizes. */
#if (ipconfigUSE_TCP_WIN == 1)
    {
      FreeRTOS_setsockopt(xListeningSocket, 0, FREERTOS_SO_WIN_PROPERTIES,
                          (void *)&xWinProps, sizeof(xWinProps));
    }
#endif /* ipconfigUSE_TCP_WIN */

    /* Bind the socket to the port that the client task will send to, then
    listen for incoming connections. */
    xBindAddress.sin_port = FreeRTOS_htons(_port);
    FreeRTOS_bind(xListeningSocket, &xBindAddress, sizeof(xBindAddress));
    FreeRTOS_listen(xListeningSocket, xBacklog);

    for (;;) {
      /* Wait for a client to connect. */
      Socket_t socket = FreeRTOS_accept(xListeningSocket, &xClient, &xSize);
      configASSERT(xConnectedSocket != FREERTOS_INVALID_SOCKET);

      /* Spawn a task to handle the connection. */
      xTaskCreate(_task_accept, NULL, 256, socket, _taskPriority, NULL);
    }
  }

  ~TcpServer() {}

private:
  static void _task_accept(void *_param) {
    static_cast<TcpServer *>(_param)->task_accept((Socket_t)_param);
  }

  void task_accept(Socket_t socket) {
    int32_t lBytes;
    const TickType_t xReceiveTimeOut = pdMS_TO_TICKS(5000);
    const TickType_t xSendTimeOut = pdMS_TO_TICKS(5000);
    TickType_t xTimeOnShutdown;
    uint8_t *pucRxBuffer;

    /* Attempt to create the buffer used to receive the string to be echoed
    back.  This could be avoided using a zero copy interface that just returned
    the same buffer. */
    pucRxBuffer = (uint8_t *)pvPortMalloc(ipconfigTCP_MSS);

    if (pucRxBuffer != NULL) {
      FreeRTOS_setsockopt(socket, 0, FREERTOS_SO_RCVTIMEO, &xReceiveTimeOut,
                          sizeof(xReceiveTimeOut));
      FreeRTOS_setsockopt(socket, 0, FREERTOS_SO_SNDTIMEO, &xSendTimeOut,
                          sizeof(xReceiveTimeOut));

      for (;;) {
        /* Zero out the receive array so there is NULL at the end of the string
        when it is printed out. */
        memset(pucRxBuffer, 0x00, ipconfigTCP_MSS);

        /* Receive data on the socket. */
        lBytes = FreeRTOS_recv(socket, pucRxBuffer, ipconfigTCP_MSS, 0);

        /* If data was received */
        if (lBytes >= 0) {
          // conn->onReceive(socket, pucRxBuffer, lBytes);
          int32_t lSent = 0;
          int32_t lTotalSent = 0;

          /* Call send() until all the data has been sent. */
          while ((lSent >= 0) && (lTotalSent < lBytes)) {
            lSent = FreeRTOS_send(socket, pucRxBuffer, lBytes - lTotalSent, 0);
            lTotalSent += lSent;
          }
        } else {
          /* Socket closed? */
          break;
        }
      }
    }

    /* Initiate a shutdown in case it has not already been initiated. */
    FreeRTOS_shutdown(socket, FREERTOS_SHUT_RDWR);

    /* Wait for the shutdown to take effect, indicated by FreeRTOS_recv()
    returning an error. */
    xTimeOnShutdown = xTaskGetTickCount();
    do {
      if (FreeRTOS_recv(socket, pucRxBuffer, ipconfigTCP_MSS, 0) < 0) {
        break;
      }
    } while ((xTaskGetTickCount() - xTimeOnShutdown) < tcpechoSHUTDOWN_DELAY);

    /* Finished with the socket, buffer, the task. */
    vPortFree(pucRxBuffer);
    FreeRTOS_closesocket(socket);

    vTaskDelete(NULL);
  }
};

#ifdef __cplusplus
}
#endif

#endif