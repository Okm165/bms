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

#include "log.h"

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
            UBaseType_t _taskPriority = osPriorityNormal);

  ~TcpServer();

private:
  static void _task_accept(void *_param) {
    static_cast<TcpServer *>(_param)->task_accept((TcpConnection *)_param);
  }

  void task_accept(TcpConnection *conn);
};

#ifdef __cplusplus
}
#endif

#endif