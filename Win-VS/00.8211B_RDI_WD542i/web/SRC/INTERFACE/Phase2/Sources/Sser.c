/*
 *	File:		Sser.c
 *
 *	Contains:	Simple Serial driver interface routines.
 *				Win32 version.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *	Copyright:	© 1995-2001 by Allegro Software Development Corporation
 *	All rights reserved.
 *
 *	This module contains confidential, unpublished, proprietary
 *	source code of Allegro Software Development Corporation.
 *
 *	The copyright notice above does not evidence any actual or intended
 *	publication of such source code.
 *
 *	License is granted for specific uses only under separate
 *	written license by Allegro Software Development Corporation.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *	Change History (most recent first):
 *
 *		12/05/01	pjr		rework SserSendStatus to report actual completion
 *		09/12/01	pjr		created Win32 version
 * * * * Release 3.0 * * * *
 * * * * Release 2.2 * * * *
 * * * * Release 2.0 * * * *
 * * * * Release 1.6 * * * *
 * * * * Release 1.5 * * * *
 * * * * Release 1.4 * * * *
 * * * * Release 1.3 * * * *
 * * * * Release 1.2 * * * *
 * * * * Release 1.1 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

#include "Sser.h"
#include <windows.h>


#define SSER_DEBUG		0

#if SSER_DEBUG
#include <stdio.h>		/* for printf */
#endif


/*
	The Sser routines are the interface between RomPager and the device
	serial routines.

	The SserInit and SserDeInit routines allocate/free memory for the
	connection and set up any necessary states for the interface.
*/


/*
	Local definitions.
*/

#define False	0
#define True	1


#define kSserReceiveBufferSize	(512)


/*
	The serial port connection control block.
*/

typedef struct {
	HANDLE			fHandle;		/* connection handle */
	DWORD			fError;			/* Windows error from GetLastError */
	DWORD			fBytesSent;		/* the actual number of bytes sent */
	SserLength		fSendLength;	/* the number of bytes to send */
	char			fReceiveBuffer[kSserReceiveBufferSize];
} SserConnection, *SserConnectionPtr;


typedef struct {
	char *		fName;
} SerialPortName;


/*
	Local data.
*/

/*
	The serial port name table.  This table may be modified to support
	more serial ports, or to change the order of the ports.
*/

static SerialPortName gSerialPortNames[4] = {
	{ "com1" },
	{ "com2" },
	{ "com3" },
	{ "com4" }
};


/*
	The serial port connection control blocks.
*/

static SserConnection gConnections[kSserNumberOfConnections];


/*
	The SserInit routine is called once when RomPager starts up, so that
	the Serial interface can initialize any internal variables and processes.

	The steps required to open the serial driver for Win32:

		1. Open the comm port with CreateFile.
		2. Set the baud rate, parity, etc.
		3. Set the comm timeouts.

	Inputs:
		none

	Returns:
		theResult:
			eRpNoError			- no error
			eRpOutOfMemory		- can't allocate memory
			eRpSerialInitError	- error opening the serial port(s)
*/

RpErrorCode SserInit(void) {
	COMMTIMEOUTS		theCommTimeouts;
	SserConnectionPtr	theConnectionPtr;
	DCB					theDCB;
	int					thePort;
	LPCTSTR				thePortName;
	RpErrorCode			theResult;
	BOOL				theSuccessFlag;

	theResult = eRpNoError;
	thePort = 0;

	while (thePort < kSserNumberOfConnections && theResult == eRpNoError) {
		/*
			Initialize some connection states.
		*/
		theConnectionPtr = &gConnections[thePort];
		theConnectionPtr->fHandle = INVALID_HANDLE_VALUE;
		theConnectionPtr->fError = ERROR_SUCCESS;

		/*
			Open the serial port.
		*/
		thePortName = gSerialPortNames[thePort].fName;
		theConnectionPtr->fHandle = CreateFile(thePortName,
							GENERIC_READ | GENERIC_WRITE,
							0,					/* exclusive access */
							NULL,				/* no security attrs */
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,
							NULL);

		if (theConnectionPtr->fHandle == INVALID_HANDLE_VALUE) {
			theConnectionPtr->fError = GetLastError();
			theResult = eRpSerialInitError;

#if SSER_DEBUG
			printf("SserInit, CreateFile failed!, port = %d, error = %d\n",
					thePort, theConnectionPtr->fError);
#endif
		}


		if (theResult == eRpNoError) {

			theSuccessFlag = GetCommState(theConnectionPtr->fHandle, &theDCB);

			if (!theSuccessFlag) {
				theConnectionPtr->fError = GetLastError();
				theResult = eRpSerialInitError;

#if SSER_DEBUG
				printf("SserInit, GetCommState failed!, port = %d, error = %d\n",
						thePort, theConnectionPtr->fError);
#endif
			}
		}


		if (theResult == eRpNoError) {
			/*
				Fill in the DCB: 9600 baud, 8 data bits, no parity, 1 stop bit.
			*/
			theDCB.BaudRate = 9600;
			theDCB.ByteSize = 8;
			theDCB.Parity = NOPARITY;
			theDCB.StopBits = ONESTOPBIT;

			theSuccessFlag = SetCommState(theConnectionPtr->fHandle, &theDCB);

			if (!theSuccessFlag) {
				theConnectionPtr->fError = GetLastError();
				theResult = eRpSerialInitError;

#if SSER_DEBUG
				printf("SserInit, SetCommState failed!, port = %d, error = %d\n",
						thePort, theConnectionPtr->fError);
#endif
			}
		}


		if (theResult == eRpNoError) {
			/*
				Get the current comm timeout values.
			*/
			theSuccessFlag = GetCommTimeouts(theConnectionPtr->fHandle,
												&theCommTimeouts);

			if (!theSuccessFlag) {
				theConnectionPtr->fError = GetLastError();
				theResult = eRpSerialInitError;

#if SSER_DEBUG
				printf("SserInit, GetCommTimeouts failed!, port = %d, error = %d\n",
						thePort, theConnectionPtr->fError);
#endif
			}
		}


		if (theResult == eRpNoError) {
			/*
				Set the comm timeouts so we don't wait indefinitely on a read.
			*/
			theCommTimeouts.ReadIntervalTimeout = MAXDWORD;
			theCommTimeouts.ReadTotalTimeoutMultiplier = 0;
			theCommTimeouts.ReadTotalTimeoutConstant = 0;

			theSuccessFlag = SetCommTimeouts(theConnectionPtr->fHandle,
												&theCommTimeouts);

			if (!theSuccessFlag) {
				theConnectionPtr->fError = GetLastError();
				theResult = eRpSerialInitError;

#if SSER_DEBUG
				printf("SserInit, SetCommTimeouts failed!, port = %d, error = %d\n",
						thePort, theConnectionPtr->fError);
#endif
			}
		}


		/*
			Maybe flush the receive buffer?
			Maybe call PurgeComm().
		*/

		/*
			Increment to the next port.
		*/
		thePort++;
	}


	/*
		If any of the ports failed to initialize, de-initialize them all.
	*/
	if (theResult != eRpNoError) {
		(void) SserDeInit();
	}

	return theResult;
}


/*
	The SserDeInit routine is called once at when RomPager finishes so that
	the Serial interface can deinitialize any internal variables and processes.
	Any receive buffers that are still allocated, should be deallocated here.

	Inputs:
		none

	Returns:
		theResult:
			eRpNoError				- no error
			eRpSerialDeInitError	- error closing the serial port(s)
*/

RpErrorCode SserDeInit(void) {
	SserConnectionPtr	theConnectionPtr;
	int					thePort;
	RpErrorCode			theResult;
	BOOL				theSuccessFlag;

	theResult = eRpNoError;

	/*
		Close any open serial connections.

		Do we have to do anything to terminate any pending
		I/O or flush any buffers?
	*/
	for (thePort = 0; thePort < kSserNumberOfConnections; thePort++) {

		theConnectionPtr = &gConnections[thePort];

		if (theConnectionPtr->fHandle != INVALID_HANDLE_VALUE) {
			theSuccessFlag = CloseHandle(theConnectionPtr->fHandle);
			theConnectionPtr->fHandle = INVALID_HANDLE_VALUE;

			if (!theSuccessFlag) {
				theConnectionPtr->fError = GetLastError();
				theResult = eRpSerialDeInitError;

#if SSER_DEBUG
				printf("SserDeInit, CloseHandle failed!, port = %d, error = %d\n",
						thePort, theConnectionPtr->fError);
#endif
			}
		}
	}

	return theResult;
}


/*
	The SserSend routine is called to send a buffer of information over
	the connection.  The call is an asynchronous call and completes when
	the callback routine (SserSendComplete) is called.  The SserSendStatus
	call is used to detect the completion of the send.  The last buffer
	received with the SserReceiveStatus call can be deallocated in this
	call.

	Inputs:
		thePortNumber:			- the serial port number
		theSendPtr:				- pointer to buffer to be sent
		theSendLength:			- length of data to be sent

	Returns:
		theResult:
			eRpNoError			- no error
			eRpSerialSendError	- can't send the buffer
*/

RpErrorCode SserSend(SserPort thePortNumber,
						char *theSendPtr,
						SserLength theSendLength) {
	SserConnectionPtr	theConnectionPtr;
	RpErrorCode			theResult;
	BOOL				theSuccessFlag;

	theConnectionPtr = &gConnections[thePortNumber];

	if (theConnectionPtr->fHandle == INVALID_HANDLE_VALUE ||
			theConnectionPtr->fError != ERROR_SUCCESS) {
		return eRpSerialSendError;
	}

	theResult = eRpNoError;
	theConnectionPtr->fSendLength = theSendLength;
	theConnectionPtr->fBytesSent = 0;

	theSuccessFlag = WriteFile(theConnectionPtr->fHandle,
								theSendPtr,
								theSendLength,
								&theConnectionPtr->fBytesSent,
								NULL);

	if (!theSuccessFlag) {
		theConnectionPtr->fError = GetLastError();
		theResult = eRpSerialSendError;

#if SSER_DEBUG
		printf("SserSend, WriteFile failed!, port = %d, error = %d\n",
				thePortNumber, theConnectionPtr->fError);
#endif
	}

	return theResult;
}


/*
	The SserSendStatus routine is called to determine whether the buffer has
	been sent.  When theCompletionStatusPtr is set to eSserComplete, control
	of the buffer returns to the caller.  This means that all lower layer
	serial routines need to have finished with the buffer.

	Inputs:
		thePortNumber:			- the serial port number
		theSendStatusPtr:		- pointer to the send status

	Returns:
		theSendStatusPtr:		- eSserComplete, the send has completed
								- eSserPending, the send has not completed yet
		theResult:
			eRpNoError			- no error
			eRpSerialSendError	- can't send the buffer (an I/O error occurred)
*/

RpErrorCode SserSendStatus(SserPort thePortNumber,
							SserStatus *theSendStatusPtr) {
	SserConnectionPtr	theConnectionPtr;

	theConnectionPtr = &gConnections[thePortNumber];

	if (theConnectionPtr->fHandle == INVALID_HANDLE_VALUE ||
			theConnectionPtr->fError != ERROR_SUCCESS) {
		return eRpSerialSendError;
	}

	if (theConnectionPtr->fBytesSent == theConnectionPtr->fSendLength) {
		*theSendStatusPtr = eSserComplete;
	}
	else {
		*theSendStatusPtr = eSserPending;
	}

	return eRpNoError;
}


/*
	The SserReceive routine is called to receive a buffer of information over
	the connection.  The call is an asynchronous call and completes when
	the receive has completed.  The SserReceiveStatus call is used to
	detect the completion of the receive.  Receive buffer allocation (if not
	done in the SserInit call) should be done in this routine.  The receive
	buffer can be deallocated when this routine is called to receive another
	buffer, or when the SserSend routine is called.

	Inputs:
		thePortNumber:				- the serial port number

	Returns:
		theResult:
			eRpNoError				- no error
			eRpSerialReceiveError	- can't start a receive
*/

RpErrorCode SserReceive(SserPort thePortNumber) {
	SserConnectionPtr	theConnectionPtr;

	theConnectionPtr = &gConnections[thePortNumber];

	if (theConnectionPtr->fHandle == INVALID_HANDLE_VALUE ||
			theConnectionPtr->fError != ERROR_SUCCESS) {
		return eRpSerialReceiveError;
	}

	/*
		The receiving of data is actually done in SserReceiveStatus.
	*/
	return eRpNoError;
}


/*
	The SserReceiveStatus routine is called to determine whether a buffer
	has been received on the connection.

	Inputs:
		thePortNumber:			- the serial port number
		theCompletionStatusPtr:	- pointer to the receive status
		theReceivePtrPtr:		- pointer to a buffer pointer
		theReceiveLengthPtr:	- pointer to the received length

	Returns:
		theReceiveStatusPtr:	- eSserComplete, if a buffer has been received
								- eSserPending, if no buffer has been received yet
		theReceivePtrPtr:		- a pointer to the buffer contents
		theReceiveLengthPtr:	- the length of the received buffer
		theResult:
			eRpNoError				- no error
			eRpSerialReceiveError	- an error occurred while reading data
*/

RpErrorCode SserReceiveStatus(SserPort thePortNumber,
								SserStatus *theReceiveStatusPtr,
								char **theReceivePtrPtr,
								SserLength *theReceiveLengthPtr) {
	DWORD				theBytesRead;
	SserConnectionPtr	theConnectionPtr;
	RpErrorCode			theResult;
	BOOL				theSuccessFlag;

	theResult = eRpNoError;
	*theReceiveStatusPtr = eSserPending;
	*theReceiveLengthPtr = 0;

	theConnectionPtr = &gConnections[thePortNumber];

	if (theConnectionPtr->fHandle == INVALID_HANDLE_VALUE ||
			theConnectionPtr->fError != ERROR_SUCCESS) {
		return eRpSerialReceiveError;
	}

	/*
		Attempt to read some data.
	*/
	theSuccessFlag = ReadFile(theConnectionPtr->fHandle,
								theConnectionPtr->fReceiveBuffer,
								(DWORD) kSserReceiveBufferSize,
								&theBytesRead,
								NULL);

	if (theSuccessFlag) {
		if (theBytesRead > 0) {
			*theReceivePtrPtr = theConnectionPtr->fReceiveBuffer;
			*theReceiveLengthPtr = (SserLength) theBytesRead;
			*theReceiveStatusPtr = eSserComplete;
		}
	}
	else {
		theConnectionPtr->fError = GetLastError();
		theResult = eRpSerialReceiveError;

#if SSER_DEBUG
		printf("SserReceiveStatus, ReadFile failed!, port = %d, error = %d\n",
				thePortNumber, theConnectionPtr->fError);
#endif
	}

	return theResult;
}
