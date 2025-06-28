/*
 *	File:		AsConfig.h
 *
 *	Contains:	Allegro Product Configuration Definitions
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *	Copyright:	© 1995-2003 by Allegro Software Development Corporation
 *  All rights reserved.
 *
 *  This module contains confidential, unpublished, proprietary
 *  source code of Allegro Software Development Corporation.
 *
 *  The copyright notice above does not evidence any actual or intended
 *  publication of such source code.
 *
 *  License is granted for specific uses only under separate
 *  written license by Allegro Software Development Corporation.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *	Change History (most recent first):
 *
 * * * * Release 4.30  * * *
 *		08/27/03	rhb		move AsUse64BitIntegers, AsUseFloatingPoint, and 
 *								AsUseEnums to VaConfig.h
 *		08/10/03	amp		add AsResourceLocks
 *		06/26/03	nam		add AsUseFloatingPoint
 *		06/25/03	bva		change AsVariableAccess definition
 * * * * Release 4.21  * * *
 *      04/29/03    amp     change UPnP toolkit name to RomPlug
 *		03/14/03	bva		add AsMultiTaskingOption
 * * * * Release 4.20  * * *
 *      12/18/02    amp     add RomCliSecure
 *		12/01/02	bva		integrate multitasking improvements
 *		11/05/02	rhb		add AsUseEnums
 * * * * Release 4.12  * * *
 *		10/02/02	bva		change date
 * * * * Release 4.11  * * *
 *		07/25/02	bva		change date
 * * * * Release 4.10  * * *
 * * * * Release 4.07  * * *
 *		03/26/02	amp		add OSE definition of Boolean
 * * * * Release 4.06  * * *
 *		02/08/02	bva		change date
 * * * * Release 4.05  * * *
 *		01/05/02	bva		documentation
 * * * * Release 4.03  * * *
 *		10/30/01	bva		add AsServerPollReduction
 *		10/25/01	bva		change date
 * * * * Release 4.02  * * *
 *		09/19/01	bva		change date
 * * * * Release 4.01  * * *
 * * * * Release 4.00  * * *
 *		06/29/01	amp		add RomUpnpControl
 *		06/01/01	pjr		add date handling settings for eZ80 and OSE
 *		05/23/01	rhb		move AsUse64BitIntegers from VaConfig.h
 *		04/19/01	bva		move Variable Access definitions to VaConfig.h
 *		04/18/01	bva		add RomUpnpAdvanced
 *		02/21/01	bva		rearrange definitions
 *		02/16/01	rhb		move kAsCompressionEscape* here from RpConfig.h
 *								(were kRpCompressionEscape*)
 *		02/15/01	rhb		move kAsIndexQueryDepth & kAsHexSeparator here from 
 *								RpConfig.h (were kRpIndexQueryDepth & 
 *								kRpHexSeparator)
 *		02/13/01	rhb		move AsCustomVariableAccess, AsUse64BitIntegers, 
 *								AsSnmpAccess, & kAsSnmpGetNextCount here from 
 *								AsConfig.h (were RpCustomVariableAccess, 
 *								RomPagerUse64BitIntegers, RomPagerSnmpAccess, &
 *								kRpSnmpGetNextCount) 
 *		02/09/01	bva/dts	RomConsole integration
 *		07/25/00	rhb		Support SSL/TLS
 *		04/21/00	bva		add RomTime flag
 *		01/18/00	bva		RomPagerLight -> RomPagerBasic,
 *							add additional package flags
 *		01/13/00	amp		add RomMailerBasic and RomPopBasic
 * * * * Release 3.10 * * * *
 * * * * Release 3.05 * * * *
 *		08/23/99	rhb		add RomXML support
 * * * * Release 3.0 * * * *
 *		03/16/99	pjr		add AsDebug
 *		02/26/99	bva		change connection setup definitions
 *		02/04/99	bva		rework documentation
 *		01/24/99	pjr		move empty definitions to AsChkDef.h
 *		01/11/99	pjr		RomPagerSingleTasking -> AsSingleTasking
 *		01/10/99	pjr		created from RpConfig.h
 *		12/29/98	bva		merge RomWebClient
 * * * * Release 2.2 * * * *
 *		11/01/98	bva		add empty definitions
 *		10/03/98	dts		add RomDns definitions
 *		08/12/98	bva		add RomPagerServer identity definition
 *		07/29/98	bva		add active connection for dictionary patching
 *		07/18/98	bva		add target support for time constants
 *		06/06/98	bva		RomPagerGroupedTasking -> RomPagerSingleTasking
 * * * * Release 2.1 * * * *
 *		05/23/98	bva		rework compile flags
 *		04/29/98	bva		add RomPagerGroupedTasking
 *		04/04/98	bva		moved some info to PrConfig.h
 *		04/02/98	bva		moved some info to RmConfig.h, RpCheck.h
 *		01/13/98	rhb		add RomPop support
 * * * * Release 2.0 * * * *
 *		10/21/97	bva		memory allocation documentation	
 *		10/07/97	pjr		add RomPagerDynamicRequestBlocks
 *		08/03/97	nam		add RomMailer support 
 * * * * Release 1.6 * * * *
 *		02/17/97	bva		add kConnectionReceiveTimeout, 
 *								kPersistentConnectionTimeout 
 *		01/26/97	rhb/bva	move Boolean to RpConfig.h (really!)
 *		01/03/97	bva		bump date, move Boolean to RpTypes.h
 *		12/13/96	bva		add RomPagerFileSystem
 * * * * Release 1.5 * * * *
 *		11/05/96	rhb		move Boolean from RpTypes.h
 *		09/24/96	rhb		add RomPagerDynamicGlobals
 * * * * Release 1.4 * * * *
 * * * * Release 1.2 * * * *
 *		05/31/96	bva		add RomPagerUseStandardTime
 * * * * Release 1.1 * * * *
 * * * * Release 1.0 * * * *
 *		01/24/96	bva		created
 *
 *	To Do:
 */

#ifndef	_ASCONFIG_
#define	_ASCONFIG_


/* 
	Boolean
	
	Define Boolean to match whatever type is used in the rest of 
	your environment.  If you have other files that include typedefs 
	for Boolean, it may be necessary to include the statement below 
	to prevent duplicate definitions of Boolean.

#define	_RPBOOLEAN_

*/

#ifndef _RPBOOLEAN_
#define	_RPBOOLEAN_
#if (RpTargetOS	== eRpTargetOSE)
	typedef enum { False = 0, True = 1 } Boolean;
#else
	typedef unsigned char	Boolean;
#endif
#endif 


/* 
	Memory Allocation

	RomPagerDynamicGlobals

	Embedded systems memory allocation strategies are a tradeoff between 
	static allocation which minimizes memory fragmentation and dynamic 
	allocation which minimize wasted memory.  The RomPager operating 
	environment allows memory to be allocated statically or dynamically.

	Static allocation identifies all memory needs at compile/link time and 
	allocates the appropriate structures from the static memory pool. To 
	use this strategy, set the RomPagerDynamicGlobals variable to 0. 
	
	Dynamic memory allocation identifies all memory usage at compile/link 
	time, and allocates the memory when the RomPager engine is initialized, 
	and deallocates the memory when the engine is terminated. To use this 
	strategy, set the variable RomPagerDynamicGlobals to 1.

	If a RomPager Web Server is used, a third strategy can be used which 
	dynamically allocates global engine memory and a starting pool of HTTP 
	request control blocks at engine initialization.  Allocation for 
	additional HTTP request control blocks are made dynamically from the pool 
	of available memory. Additional request control blocks will be allocated 
	when a TCP/HTTP request has a receive buffer to process and will be freed 
	when there is no more response data to send and the connection is about to
	be closed. This third strategy allows the number of simultaneous HTTP 
	requests to be limited by available memory, but increases the possibility 
	of memory fragmentation.  The control of the Web Server HTTP request 
	block size and allocation strategy is determined by compilation flags 
	in RpConfig.h.
*/

#define RomPagerDynamicGlobals		0


/* 
	Scheduling Algorithms
	
	The Allegro scheduler uses a single task from the host OS environment and
	provides its own internal scheduler for handling simultaneous TCP and 
	HTTP/SMTP/POP3 connections. Each potential separate TCP connection 
	maintains a state and other parameters in a connection control block. The 
	number of connections to be managed is determined by kStcpNumberOfConnections 
	which is defined in Stcp.h. The connection control block structure is
	the rpConnection structure which is defined in RomPager.h.
	
	The normal entry point to the Allegro scheduler is the AllegroMainTask
	routine.  When this routine is called, each connection will be checked 
	to see if there is something to do. If there is, the internal task will 
	be run to the next I/O breaking point (a packet read or write call, 
	for example).  After all the connections have been checked, the internal 
	timer task will be run to check for various timeout conditions.  The
	AllegroMainTask routine should be called at least once per second for
	timing functions and more often when TCP activity is occuring.  A simple
	way to run the server without limiting performance on the rest of the
	system is to call AllegroMainTask once every 10th of a second.

	If the AllegroMultiTasking variable is set to 1, then the
	AllegroConnectionTask and AllegroTimerTask entry points are enabled.
	These additional routines allow finer resolution of the scheduling
	process.  The AllegroTimerTask will run all of the internal RomPager 
	timer routines and should be run at least once per second for correct 
	timeout behavior.  The AllegroConnectionTask routine will run a single 
	internal task to the next I/O breaking point.  The AllegroConnectionTask
	should be scheduled for each individual connection when the caller has 
	determined that there is activity for a particular connection.  The 
	connection number used in the AllegroConnectionTask call is the same 
	as the connection number used by the TCP interface calls (defined in 
	Stcp.h).  This makes it possible for the calls at the TCP interface 
	layer to signal the Allegro scheduler task with the information about 
	which connection needs to be run.
	
	Allegro provides an implementation of multitasking support for some 
	environments. If you are using an Allegro provided implementation, then
	the AsMultiTaskingOption variable should be set to 1.

	In a non-multitasking environment, some operating system environments
	that use socket-based TCP can use a lot of system resources to check 
	whether a connection has come in on a server port. Performance can be 
	boosted by setting the AsServerPollReduction variable to 1 which 
	reduces the number of system calls used to check on idle connections.
	This capability only works in socket-based TCP environments. In non-socket
	environments, enabling this flag can cause missed connections.
*/

#define AllegroMultiTasking			1
#if AllegroMultiTasking
	#define AsMultiTaskingOption	0
#endif

#define AsServerPollReduction		0


/*
 	Date Handling
 	
 	kHttpRomMonth, kHttpRomDay, kHttpRomYear
 	RomPagerCalendarTime, RomPagerUseStandardTime
 	
 	The kHttpRomMonth, kHttpRomDay and kHttpRomYear variables are used to 
 	set up an internal rom date.  It is recommended for best browser caching 
 	behaviour that this date be the release date of the device.  Static 
 	objects, such as graphics, are served using the internal rom date.  
 	
	If RomPagerUseStandardTime is defined as 1, the date code will use the 
	standard C library time calls. If your device is already using these 
	libraries, you should set this flag. Otherwise, additional RomPager code 
	(using about 500 bytes) equivalent to these libraries will be generated.

 	If RomPagerCalendarTime is defined as 1, dynamic pages will be served 
 	using the device date.  If a calendar date is not available, a caching 
 	date will be created for dynamic pages using the internal rom date plus 
 	number of seconds since system boot.  This date creation is needed to 
 	force browsers to update their page displays for dynamic pages.
 	 	
 	The RomPagerUseStandardTime and RomPagerCalendarTime flags are used to
 	control the code generation of the date routines in AsDate.c.  For a 
 	number of operating system environments, the settings and optional code
 	in AsDate.c is already provided. If your operating system environment
 	is one that is identified in AsTarget.h, you don't need to set these
 	flags.  Otherwise, set the flags for the unknown target environment. 	
 	
	
*/

#define kHttpRomMonth			(1)		/* month -- 1 for January   	*/
#define kHttpRomDay				(15)	/* day   -- 1 for 1st of month	*/
#define kHttpRomYear			(2003)	/* year  -- years A.D.   		*/

#if (RpTargetOS	== eRpTargetWin32)
	#define RomPagerUseStandardTime	1
	#define RomPagerCalendarTime	1
#elif (RpTargetOS == eRpTargetUnix) 
	#define RomPagerUseStandardTime	1
	#define RomPagerCalendarTime	1
#elif (RpTargetOS == eRpTargetLinux) 
	#define RomPagerUseStandardTime	1
	#define RomPagerCalendarTime	1
#elif (RpTargetOS == eRpTargetMacOS) 
	#define RomPagerUseStandardTime	1
	#define RomPagerCalendarTime	1
#elif (RpTargetOS == eRpTargetTestNoStd) 
	#define RomPagerUseStandardTime	0
	#define RomPagerCalendarTime	1
#elif (RpTargetOS == eRpTargetTestNoCal) 
	#define RomPagerUseStandardTime	0
	#define RomPagerCalendarTime	0
#elif (RpTargetOS == eRpTargetLynx) 
	#define RomPagerUseStandardTime	1
	#define RomPagerCalendarTime	0
#elif (RpTargetOS == eRpTargetNucleus) 
	#define RomPagerUseStandardTime	0
	#define RomPagerCalendarTime	0
#elif (RpTargetOS == eRpTargetOS9) 
	#define RomPagerUseStandardTime	1
	#define RomPagerCalendarTime	0
#elif (RpTargetOS == eRpTargetPrecise) 
	#define RomPagerUseStandardTime	0
	#define RomPagerCalendarTime	0
#elif (RpTargetOS == eRpTargetPSOS) 
	#define RomPagerUseStandardTime	0
	#define RomPagerCalendarTime	0
#elif (RpTargetOS == eRpTargetQNX) 
	#define RomPagerUseStandardTime	1
	#define RomPagerCalendarTime	0
#elif (RpTargetOS == eRpTargetVRTX) 
	#define RomPagerUseStandardTime	1
	#define RomPagerCalendarTime	0
#elif (RpTargetOS == eRpTargetVxWorks) 
	#define RomPagerUseStandardTime	1
	#define RomPagerCalendarTime	0
#elif (RpTargetOS == eRpTargetEz80) 
	#define RomPagerUseStandardTime	0
	#define RomPagerCalendarTime	0
#elif (RpTargetOS == eRpTargetOSE) 
	#define RomPagerUseStandardTime	1
	#define RomPagerCalendarTime	0
#else
	#define RomPagerUseStandardTime	0
	#define RomPagerCalendarTime	0
#endif


/*
	File System

	RomPagerFileSystem

	If RomPagerFileSystem is defined as 1, the code to support file system
	data access functions will be generated. The file system functions can 
	be optionally used by various of the RomPager product family to store 
	and retrieve Web objects or mail messages and attachments.
*/

#define RomPagerFileSystem	1	


/*
	TCP Connections
	
	kConnectionCloseTimeout, kConnectionReceiveTimeout, 
	kConnectionMaxIdleTimeout
	
	There are a few cases where the engine may need to terminate a connection 
	to free it up so that other requests can use it.  Different implementations
	of TCP handle connection closings in different ways, and may need the
	engine to perform connection cleanup. The first case is that is that when 
	a connection closes, it sometimes is left in the TCP TIME_WAIT state 
	which depending on implementation can be a long time (4 minutes) after 
	the connection has been closed. In the Web server enviroment each request 
	is typically a different connection, so a lot of connections are opened 
	and closed. If each connection has to wait 4 minutes to free up its 
	resources, a lot of connections and internal resources will be used. Some 
	browsers just abort the connection to avoid this problem.  Persistent 
	connection approaches such as "Keep-Alive" and HTTP 1.1 reduce this 
	problem, but using connection timeouts reduces the problem for all 
	browsers.  By setting the kConnectionCloseTimeout variable to a low 
	value, the connection will be aborted after spending a small amount of 
	time in the TIME_WAIT state, so that less total connections are required.
	
	In Web server environments, a second case is where an impatient user hits 
	the reload button in a browser, while a request is in process. The browser 
	will close the connection that the receive has started on, but not all TCP 
	implementations are able to signal the remote side close.  This can leave 
	a connection tied up waiting indefinitely for a receive that was reissued 
	on another connection. The kConnectionReceiveTimeout value is used abort 
	connections where a receive was started and no data has been received.
	
	The third case is that of a Web server persistent connection being used by 
	"Keep-Alive" or HTTP 1.1.  A persistent connection can reduce the 
	overhead for a single user, but can leave a connection tied up that 
	another user might need.  The kConnectionMaxIdleTimeout value is used 
	to set the maximum idle time period.  If another request is not made 
	in this period, then the connection will be closed.
*/

#define kConnectionCloseTimeout			2		/*  abort timeout in secs 		*/
#define kConnectionReceiveTimeout		3		/*  abort timeout in secs 		*/
#define kConnectionMaxIdleTimeout		60		/*  persistent timeout in secs 	*/
#define kTelnetReceiveTimeout			60*15	/*  Telnet timeout in secs 		*/
#define kConsoleReceiveTimeout			60*15	/*  Console timeout in secs 	*/


/*
	Debug Options
 
	AsDebug

	If AsDebug is set, various support routines and error checking code for
	tuning and debugging will be generated.  In production versions, this
	flag should be turned off.
*/

#define AsDebug		0


/* 
	RomPager Product Family

	The RomPager product family consists of a number of different products 
	that provide various Internet application services. There are two
	different Web server products, a Web client product, four Internet email 
	clients (SMTP and POP3), an XML translator, a Telnet server, SSL/TLS services
	and a Domain Name Service (DNS) client. These products share a common scheduler 
	engine, TCP and file system services, and various routines common to Internet 
	application protocols.  The compile flags below are used for controlling the 
	build of the overall engine environment. The appropriate flag should be set 
	to 1 for each product being used in the project.
	
	**********
	
	Web (HTTP) Servers
	
	RomPager Basic is a basic HTTP server with CGI-like user exit support and
	optional file system support.
	
	If RomPagerBasic is defined as 1, the code to support the RomPager Basic
	Web server will be compiled. The configuration options for RomPager Basic
	are defined in RsConfig.h.

	The RomPager Advanced Server is a full-featured embedded Web server that 
	includes the RomPager Basic capabilities, advanced HTTP support, and a Web 
	application toolkit for compiling HTML pages and forms into highly 
	compressed Web applications with integrated dynamic variables.
	
	If RomPagerServer is defined as 1, the code to support the RomPager 
	Advanced server will be compiled. The configuration options for RomPager
	Advanced are defined in RpConfig.h.

	**********
	
	Simple Mail Transfer Protocol (SMTP)
	
	The RomMailer Basic SMTP toolkit provides the ability to send mail messages 
	to an SMTP server.  The messages can be plain text or HTML.  The message body 
	is passed in a buffer.

	If RomMailerBasic is defined as 1, the code to support the basic SMTP client 
	interface will be compiled. The configuration options for RomMailer Basic
	are defined in ScConfig.h.

	The RomMailer standard SMTP toolkit provides the ability to send mail messages 
	to an SMTP server.  The messages can be plain text, HTML, or HTML with 
	embedded images.  The message body can be sent from a previously prepared 
	file stored in the file system, or as a ROM-based object that can use all 
	the techniques of the RomPager Server Web Application Toolkit to insert 
	dynamic information into the message body.  When RomMailer is using a 
	ROM-based object it shares the code in RpHtml.c to create the message body.  
		
	If RomMailer is defined as 1, the code to support the advanced SMTP client 
	interface will be compiled. The configuration options for RomMailer
	are defined in RmConfig.h.

	**********
	
	Post Office Protocol, version 3 (POP3)

	The RomPOP Basic POP3 toolkit provides the ability to retrieve simple
	text messages from a POP3 mail server.
	
	If RomPopBasic is defined as 1, the code to support the POP3 client 
	interface will be compiled.  The configuration options for RomPOP Basic
	are defined in PcConfig.h.

	The RomPOP Standard POP3 toolkit provides the ability to retrieve mail 
	messages from a POP3 server. RomPOP can also parse messages for 
	attachments. These messages and attachments are stored using the 
	Allegro File System interface.
	
	If RomPop is defined as 1, the code to support the standard POP3 client 
	interface will be compiled.  The configuration options for RomPOP
	are defined in PrConfig.h.

	**********
	
	Web (HTTP) Client

	The RomWebClient toolkit provides the ability to retrieve HTTP objects 
	from HTTP Servers. 
	
	If RomWebClient is defined as 1, the code to support the Web Client 
	interface will be compiled.  The configuration options for RomWebClient
	are defined in WcConfig.h.

	**********
	
	Domain Name Service (DNS)

	The RomDNS toolkit provides the ability to look up IP addresses and 
	other information based on fully qualified domain names.
		
	If RomDns is defined as 1, the code to support the DNS client 
	interface will be compiled. The configuration options for RomDns
	are defined in RdConfig.h.

	**********
	
	XML Parser-Framer

	The RomXML toolkit provides the ability to convert C structures to 
	and from XML datastreams.
		
	If RomXml is defined as 1, the code to support the XML toolkit 
	interface will be compiled. The configuration options for RomXML
	are defined in RxConfig.h.

	**********
	
	SSL/TLS support

	If RomPagerSecure is defined as 1, the code to support the SSL version 3 
	and TLS security layer support for the RomPager Advanced Server is compiled.

	If RomWebClientSecure is defined as 1, the code to support the SSL version 3 
	and TLS security layer support for the RomWebClient is compiled.

	**********
	
	Telnet Server

	The RomTelnet toolkit provides the ability to support Telnet terminal
	sessions and is used by the RomCLI product.
		
	If RomTelnet is defined as 1, the code to support the Telnet toolkit 
	interface will be compiled. The configuration options for RomTelnet
	are defined in TnConfig.h.
	
	**********
	
	Console Server

	The RomConsole toolkit provides the ability to support serial port
	terminal sessions and is used by the RomCLI product.
		
	If RomConsole is defined as 1, the code to support the console toolkit 
	interface will be compiled. The configuration options for RomConsole
	are defined in CsConfig.h.
	
	**********

	CLI Server

	The RomCLI toolkit provides Command Line Interface support for 
	controlling devices.
		
	If RomCli is defined as 1, the code to support the CLI toolkit 
	interface will be compiled. The configuration options for RomCLI
	are defined in RcConfig.h.
	
	**********
	
	Time Services Client (NTP)

	The RomTime toolkit provides the ability to add Network Time Protocol
	support to embedded devices.
		
	If RomTime is defined as 1, the code to support the Time services client 
	will be compiled. The configuration options for Romtime are defined in 
	TmConfig.h.

	**********
	
	Universal Plug and Play (UPnP)

	There are 3 RomPlug toolkits for providing Universal Plug and Play 
	capabilities in embedded devices.

	RomPlug Basic provides Device side Discovery and Description services.
	If RomPlug is defined as 1, the code to support RomPlug Basic will be
	compiled.

	RomPlug Advanced provides Device side Control and Eventing services as
	well as the RomPlug Basic services. The RomPlugAdvanced flag is used to 
	enable this product.
 
	The configuration options for RomPlug Basic and Advanced are defined in
	RuConfig.h.

	RomPlug Control provides Control Point services for Discovery,
	Description, Control and Eventing.  The RomPlugControl flag is used
	to enable this product.  The configuration options for RomPlug Control
	are defined in CpConfig.h.

    **********

    RomCliSecure

    The RomCliSecure toolkit provides the ability to support SSH
    terminal sessions and is used by the RomCLI product.

    If RomCliSecure is defined as 1, the code to support the SSH toolkit
    interface will be compiled. The configuration options for RomCliSecure
    are defined in ShConfig.h.
*/

#define RomPagerBasic		0
#define RomPagerServer		1
#define RomPagerSecure		0	/* ssl */
#define	RomMailerBasic		0		
#define	RomMailer			0
#define RomPopBasic			0
#define RomPop				0
#define RomWebClient		0		
#define RomWebClientSecure	0
#define	RomDns				0		
#define RomXml				0
#define RomTelnet			0
#define RomConsole			0
#define RomCli				0
#define RomCliSecure		0
#define RomTime				0
#define RomPlug				0
#define RomPlugAdvanced		0
#define RomPlugControl		0

#if RomCli || RomPlugAdvanced || RomPlugControl
	#define AsVariableAccess	1
#else
	#define AsVariableAccess	0
#endif

/*
	AsResourceLocks provides thread safety using target-specific locking
	functions that are accessed using the API of AsLock.c.
*/
#if RomPlugAdvanced
	#define AsResourceLocks		1
#else
	#define AsResourceLocks		0
#endif	/* RomPlugAdvanced */

// +++ _Alphanetworks_Patch_, 02/12/2004, jacob_shih
#define _Alpha_RpWebID_
// --- _Alphanetworks_Patch_, 02/12/2004, jacob_shih

#endif	/* _ASCONFIG_ */
