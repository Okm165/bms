#ifdef __cplusplus
extern "C" {
#endif

#include "tcp_server.hpp"

TcpServer::TcpServer(uint16_t _port, ReceiveCallback onReceive,
                     UBaseType_t _taskPriority) {
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
  LogDebug("socket initialized");
  FreeRTOS_bind(xListeningSocket, &xBindAddress, sizeof(xBindAddress));
  LogDebug("socket bind");
  FreeRTOS_listen(xListeningSocket, xBacklog);
  LogDebug("socket listening");

  for (;;) {
    /* Wait for a client to connect. */
    TcpConnection *new_conn =
        (TcpConnection *)pvPortMalloc(sizeof(TcpConnection));
    new_conn->socket = FreeRTOS_accept(xListeningSocket, &xClient, &xSize);
    configASSERT(xConnectedSocket != FREERTOS_INVALID_SOCKET);
    new_conn->onReceive = this->onReceive;
    LogDebug("new connection");

    /* Spawn a task to handle the connection. */
    xTaskCreate(TcpServer::_task_accept, "tcp_server_accept_task", 256,
                new_conn, _taskPriority, NULL);
  }
}

TcpServer::~TcpServer() {}

void TcpServer::task_accept(TcpConnection *conn) {
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
    FreeRTOS_setsockopt(conn->socket, 0, FREERTOS_SO_RCVTIMEO, &xReceiveTimeOut,
                        sizeof(xReceiveTimeOut));
    FreeRTOS_setsockopt(conn->socket, 0, FREERTOS_SO_SNDTIMEO, &xSendTimeOut,
                        sizeof(xReceiveTimeOut));

    for (;;) {
      /* Zero out the receive array so there is NULL at the end of the string
      when it is printed out. */
      memset(pucRxBuffer, 0x00, ipconfigTCP_MSS);

      /* Receive data on the socket. */
      lBytes = FreeRTOS_recv(conn->socket, pucRxBuffer, ipconfigTCP_MSS, 0);

      /* If data was received */
      if (lBytes >= 0) {
        LogDebug("received data (len:%d)", lBytes);
        conn->onReceive(conn->socket, pucRxBuffer, lBytes);
      } else {
        /* Socket closed? */
        break;
      }
    }
  }
  LogDebug("socket shutting down");
  /* Initiate a shutdown in case it has not already been initiated. */
  FreeRTOS_shutdown(conn->socket, FREERTOS_SHUT_RDWR);

  /* Wait for the shutdown to take effect, indicated by FreeRTOS_recv()
  returning an error. */
  xTimeOnShutdown = xTaskGetTickCount();
  do {
    if (FreeRTOS_recv(conn->socket, pucRxBuffer, ipconfigTCP_MSS, 0) < 0) {
      break;
    }
  } while ((xTaskGetTickCount() - xTimeOnShutdown) < tcpechoSHUTDOWN_DELAY);

  /* Finished with the socket, buffer, the task. */
  vPortFree(pucRxBuffer);
  vPortFree(conn);
  FreeRTOS_closesocket(conn->socket);
  LogDebug("closing task");
  vTaskDelete(NULL);
}

#ifdef __cplusplus
}
#endif