/*
 *	File:		IppShell.c
 *
 *	Contains:	Example/Test IPP parser shell
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
 * * * * Release 4.00  * * *
 *		02/17/01	bva		use AsExtern.h
 *		10/06/00	pjr		IppInit is passed theTaskDataPtr
 *		06/27/00	rhb		add support for multiple IPP response buffers
 *		06/26/00	pjr		add IppTimer
 * * * * Release 3.10  * * *
 * * * * Release 3.0 * * * *
 *		03/18/99	pjr		include RpExtern.h instead of RomPager.h
 * * * * Release 2.2 * * * *
 * * * * Release 2.1 * * * *
 *		01/05/98	rhb		fix response length
 * * * * Release 2.0 * * * *
 *		11/03/97	rhb		created
 * * * * Release 1.6 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

#include "AsExtern.h"


#if RomPagerIpp

/*
	IPP parser data structure.
*/

typedef struct ippData {
	Unsigned8			fResponseIndex;
} ippData, *ippDataPtr;

static 	ippData		gIppData;

static char *gIppResponse[] = {
	"Send this",
	"and this",
	"and some more",
	"and the last bit"
};


/*
	Initialize the IPP parser.
*/

extern void *IppInit(void *theTaskDataPtr) {

	/*
		Initialize the index used for sending the response.
	*/
	gIppData.fResponseIndex = 0;

	return (void *) &gIppData;
}


/*
	De-initialize the IPP parser.
*/

extern void IppDeInit(void *theIppDataPtr) {

	/*
		The IPP parser data structure would be freed here if it
		had been allocated dynamically.
	*/
	return;
}


/*
	Begin processing an IPP request buffer.

	Returns:

		eIppHttpResponse_Pending		- still processing, call IppCheckParser
										  later to continue processing
		eIppHttpResponse_NeedNextBuffer	- the buffer has been processed, the
										  parser is ready for another buffer
		eIppHttpResponse_TransmitOK		- the buffer has been processed and
										  a response packet is being returned
		eIppHttpResponse_TransmitAbort	- the buffer has been processed, a
										  response packet is being returned,
										  and the connection should be closed
										  after the last response buffer has
										  been sent
*/

extern ippHttpResponse IppStartParser(ippRequestPtr theIppRequestPtr) {

	if (theIppRequestPtr->fLastBufferFlag) {
		/*
			We've received the last IPP request buffer.
			For this example, just send the 4 hard-coded
			response buffers.
		*/
		strcpy(theIppRequestPtr->fResponsePtr,
				gIppResponse[gIppData.fResponseIndex]);
		theIppRequestPtr->fResponseLength =
				strlen(gIppResponse[gIppData.fResponseIndex]);

		if (gIppData.fResponseIndex < 3) {
			/*
				This is one of the first 3 response buffers.
			*/
			theIppRequestPtr->fLastBufferFlag = False;
			gIppData.fResponseIndex++;
		}
		else {
			/*
				This is the last 4th (and last) response buffer.
				Reset the response buffer index for next time.
			*/
			theIppRequestPtr->fLastBufferFlag = True;
			gIppData.fResponseIndex = 0;
		}
		return eIppHttpResponse_TransmitOK;
	}
	else {
		/*
			For testing purposes, return pending if this is not
			the last buffer, so IppCheckParser will be called.
		*/
		return eIppHttpResponse_Pending;
	}
}


/*
	Continue processing an IPP request buffer.

	Returns:

		eIppHttpResponse_Pending		- still processing, call this routine
										  again later
		eIppHttpResponse_NeedNextBuffer	- the buffer has been processed, the
										  parser is ready for another buffer
		eIppHttpResponse_TransmitOK		- the buffer has been processed and
										  a response packet is being returned
		eIppHttpResponse_TransmitAbort	- the buffer has been processed, a
										  response packet is being returned,
										  and the connection should be closed
										  after the last response buffer has
										  been sent
*/

extern ippHttpResponse IppCheckParser(ippRequestPtr theIppRequestPtr) {

	/*
		For this example, IppCheckParser is only called for intermediate
		buffers so it always returns eIppHttpResponse_NeedNextBuffer.
	*/
	return eIppHttpResponse_NeedNextBuffer;
}


/*
	This routine is called once per second.  Handle any housekeeping
	tasks here.  For example, timing out a job that has been created
	but has waited too long for a document to be attached.
*/

extern RpErrorCode IppTimer(void * theIppDataPtr,
							Unsigned32 theCurrentTime) {

	return eRpNoError;
}

#endif	/* RomPagerIpp */
