/*****************************************************************************************

* Copyright Information :	This software is the property of D-Link and shall not be 
							reproduced distributed and copied without the permission 
							from D-Link

* Module Name			:	Secure Socket Layer

* Description			:	This file is porting related header for SSL

* Author Name			:	M.K.Saravanan

******************************************************************************************/

#ifndef SSLPORT_H
#define SSLPORT_H


#include <SSLCfg.h>

#if SSL_CFG_NUCLEUS
#include <string.h>
#include <p2types.h>
#include <iprtypes.h>
#include <p2ibd.h>
#include <p2port.h>
#include <ipport.h>
#include <p2iptype.h>
#include <p2snadd.h>
#include "socket.h"
#include "tcp_usr.h"
#include <cert_epm.h>
#include <SSLEpm.h>
#endif

#if SSL_CFG_WINDOWS
#define SSL_DEBUG						printf
#define SSL_Memory						NULL
#define RUNTIME_VERSION_CHECK(version)	1
#define RUNTIME_REQUEST_CERT			SSL_CFG_REQUEST_CERTIFICATE
#define RUNTIME_CLIENTCERT_MANDATORY	1
#endif

#if SSL_CFG_NUCLEUS

#define SSL_DEBUG						if (WEB_Get_SSL_Debug()) xprintf
#define SSL_MEM_POOL_SIZE				100*1024//300*1024
#define SSL_MIN_ALLOC					5

#define RUNTIME_VERSION_CHECK(version)	SSLIsVersionEnabled(version)
#define RUNTIME_REQUEST_CERT			SSLEpmReqCert()
#define RUNTIME_CLIENTCERT_MANDATORY	1

#ifdef DEFINE_SSL_MEMORY
void *SSL_Memory;
#endif

#ifdef EXTERN_SSL_MEMORY
extern void *SSL_Memory;
#endif


#endif



#endif /* SSLPORT_H */