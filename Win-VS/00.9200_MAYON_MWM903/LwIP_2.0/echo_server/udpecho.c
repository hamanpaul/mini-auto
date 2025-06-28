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

//*-------------------------------------??????--------------------------------------------------------------------
#include "general.h"
#include "task.h"
#if UDP_ECHO_SERVER_SUPPORT
#include "lwip/lwipsys.h"
#include "lwip/mem.h"
#include "lwip/memp.h" 
#include "lwip/pbuf.h"
#include "lwip/tcpip.h"
#include "lwip/udpecho.h"
#include <lwip/sockets.h>
#include "lwip/opt.h"
#include "lwip/api.h"
#define LISTEN_PORT 7
OS_STK echoServerTaskStack[ECHO_SERVER_TASK_STACK_SIZE];
extern void UartCmdParse(char *UartCmdString);
/*-----------------------------------------------------------------------------------*/
static void
udpecho_thread(void *arg)
{
  static struct netconn *conn;
  static struct netbuf *buf;
  static struct ip_addr *addr;
  static unsigned short port;
  char buffer[4096];
  err_t err;
//  LWIP_UNUSED_ARG(arg);
  printf("UDP Echo server task create...\n");

  conn = netconn_new(NETCONN_UDP);
  LWIP_ASSERT("con != NULL", conn != NULL);
  netconn_bind(conn, NULL, 7);

  while (1) {
    buf = netconn_recv(conn);
    if (buf != NULL) {
      addr = netbuf_fromaddr(buf);
      port = netbuf_fromport(buf);
      netconn_connect(conn, addr, port);
      netbuf_copy(buf, buffer, buf->p->tot_len);
      buffer[buf->p->tot_len] = '\0';
      printf("Recved:[[%s]]\n", buffer);
      UartCmdParse(buffer);
//      netconn_send(conn, buf);
//      LWIP_DEBUGF(LWIP_DBG_ON, ("got %s\n", buffer));
      netbuf_delete(buf);
    }
    OSTimeDly(1);
  }
}
/*-----------------------------------------------------------------------------------*/
void udpecho_init(void)
{
  printf("UDP Echo server init...\n");
//  sys_thread_new( udpecho_thread, NULL,  ECHO_SERVER_PRIO);
  if(OS_NO_ERR != OSTaskCreate(udpecho_thread, NULL, ECHO_SERVER_TASK_STACK, ECHO_SERVER_PRIO))
    printf("[[TCP Echo server create failed]]\n");
}

#endif /* LWIP_NETCONN */