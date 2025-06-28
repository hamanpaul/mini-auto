/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#include "general.h"
#include "task.h"

#include "lwip/opt.h"

#if TCP_ECHO_SERVER_SUPPORT

//#include "lwip/sys.h"
#include "lwip/lwipsys.h"
#include "lwip/api.h"
#include "lwip/tcpecho.h"
#define LISTEN_PORT 7
OS_STK echoServerTaskStack[ECHO_SERVER_TASK_STACK_SIZE];
extern void UartCmdParse(char *UartCmdString);
/*-----------------------------------------------------------------------------------*/
static void 
tcpecho_thread(void *arg)
{
  struct netconn *conn, *newconn;
  err_t err;
  //LWIP_UNUSED_ARG(arg);
printf("[[TCP echo server start]]\n");
  /* Create a new connection identifier. */
  conn = netconn_new(NETCONN_TCP);

  /* Bind connection to well known port number 7. */
  netconn_bind(conn, NULL, LISTEN_PORT);
printf("[[Bind port %d]]\n", LISTEN_PORT);
  /* Tell connection to go into listening mode. */
  netconn_listen(conn);

  while (1) {

    /* Grab new connection. */
    newconn = netconn_accept(conn);
    /*printf("accepted new connection %p\n", newconn);*/
    /* Process the new connection. */
    if (newconn != NULL) {
      struct netbuf *buf;
      void *data;
      u16_t len;
      
      while ((buf = netconn_recv(newconn)) != NULL) {
        /*printf("Recved\n");*/
        do {
             netbuf_data(buf, &data, &len);
             printf("Recved:[[%s]]\n", data);
             UartCmdParse(data);
#if 0
             err = netconn_write(newconn, data, len, NETCONN_COPY);
            if (err != ERR_OK) {
              printf("tcpecho: netconn_write: error \"%s\"\n", lwip_strerr(err));
            }
#endif
        } while (netbuf_next(buf) >= 0);
        netbuf_delete(buf);
      }
      /*printf("Got EOF, looping\n");*/ 
      /* Close connection and discard connection identifier. */
      netconn_close(newconn);
      netconn_delete(newconn);
    }
    OSTimeDly(1);
  }
}
/*-----------------------------------------------------------------------------------*/
void
tcpecho_init(void)
{
     
//  if(sys_thread_new(tcpecho_thread, NULL,  ECHO_SERVER_PRIO) == 0)
  if(OS_NO_ERR != OSTaskCreate(tcpecho_thread, NULL, ECHO_SERVER_TASK_STACK, ECHO_SERVER_PRIO))
    printf("[[TCP Echo server create failed]]\n");
}
/*-----------------------------------------------------------------------------------*/

#endif /* LWIP_NETCONN */