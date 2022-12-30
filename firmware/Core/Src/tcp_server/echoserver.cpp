#ifdef __cplusplus
extern "C" {
#endif

#include "echoserver.h"
#include "main.h"
#include "tcp_server.hpp"

void rcv(Socket_t socket, uint8_t *data, int32_t size) {
  int32_t lSent = 0;
  int32_t lTotalSent = 0;

  /* Call send() until all the data has been sent. */
  while ((lSent >= 0) && (lTotalSent < size)) {
    lSent = FreeRTOS_send(socket, data, size - lTotalSent, 0);
    lTotalSent += lSent;
  }
}

void vStartSimpleEchoServer(void *pvParameters) {
  /* Create the TCP echo server. */
  debug("starting ServerListener\n");
  TcpServer(7, rcv);

  /* Remember the requested stack size so it can be re-used by the server
  listening task when it creates tasks to handle connections. */
  // usUsedStackSize = usStackSize;
  vTaskDelete(NULL);
}

#ifdef __cplusplus
}
#endif