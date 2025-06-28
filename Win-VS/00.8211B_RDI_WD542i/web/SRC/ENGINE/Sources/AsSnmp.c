/*
 *	File:		AsSnmp.c
 *
 *	Contains:	RomPager routines for SNMP variable access
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
 *		02/16/01	rhb		rename file and functions from Rp* to As*
 *		02/09/00	bva		use AsEngine.h
 *		01/17/00	bva		theServerDataPtr -> theTaskDataPtr
 * * * * Release 3.06  * * *
 * * * * Release 3.0 * * * *
 *		01/07/99	rhb		use RP_ATOL() instead of atol() 
 *		01/06/99	rhb		use consistent return types and compatible 
 *								initializations
 *		12/15/98	bva		created from SNMP Research RpHtml.c
 * * * * Release 2.2 * * * *
 * * * * Release 2.0 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

#include "AsEngine.h"

#if AsSnmpAccess && AsVariableAccess

#if (RpTargetSnmp == eRpSnmpResearch) 
#define SR_SNMPv1
#define SR_ANSI_PROTOTYPES
#define SR_IP

#include "sr_snmp.h"
#include "sr_trans.h"
#include "v2clssc.h"
#include "comunity.h"
#include "context.h"
#include "objectdb.h"
#include "frmt_lib.h"
#include "prnt_lib.h"

char	gOID_ValueString[2048 + 512];
OID		gTempOID;
#endif

#if (RpTargetSnmp == eRpSnmpResearch)
static OID *	ConvertToSnmpResearchOID(asSnmpAccessItemPtr theAccessPtr);
static void		SaveFromSnmpResearchOID(OID *theRetrievedOIDPtr, 
							asSnmpAccessItemPtr theSaveAccessPtr);
#endif

#if RomPagerServer && RomPagerSoftPages
void RpSoftSnmpAccess(void *theTaskDataPtr,
						Signed16Ptr theIndexValuesPtr,
						char *theOIDStringPtr) {
	Unsigned32Ptr		theArrayptr;
	rpDataPtr			theDataPtr;
	rpHttpRequestPtr 	theRequestPtr;
	asSnmpAccessItem	theSnmpRequest;
	
	theDataPtr = (rpDataPtr) theTaskDataPtr;
	theRequestPtr = theDataPtr->fCurrentHttpRequestPtr;
	/*
		Set the request type to Get.
	*/
	theSnmpRequest.fGetNextIndex = -1; 
	/*
		Convert the OID string to internal format.
	*/
	theSnmpRequest.fCount = 0;
	theArrayptr = theSnmpRequest.fArray;
	do {
		*theArrayptr++ = RP_ATOL(theOIDStringPtr);
		while (*theOIDStringPtr >= kAscii_0 && *theOIDStringPtr <= kAscii_9) {
			theOIDStringPtr += 1;
		}
		theOIDStringPtr += 1;
		theSnmpRequest.fCount += 1;
	} while (*theOIDStringPtr != '\0');

	/*
		Call the access function to send out the text string of the value.
	*/
	RpSendOutSnmpDisplay(theRequestPtr, &theSnmpRequest);
	return;				
}
#endif	/* RomPager Server && RomPagerSoftPages */

/*
	This function is called by the various access routines in AsVarAcc.c.
	The routine is passed in an OID structure and various engine information.
	It returns a pointer to the MIB value in native format for conversion to
	string by the RpVarAcc.c routines.
*/ 

void *AsRetrieveSnmpItem(void *theTaskDataPtr, 
							char *theHtmlNamePtr, 
							Signed16Ptr theIndexValuesPtr,
							asTextType theItemDataType,
							asSnmpAccessItemPtr theSnmpAccessPtr) {

#if RomPagerServer
	asSnmpAccessItemPtr	theAccessPtr;
	rpDataPtr			theDataPtr;
	Boolean				theGetNextFlag;
	Signed8				theGetNextIndex;
	Signed16			theIndexI;						
	Signed16			theIndexJ;						
	Signed16			theIndexK;						
	Signed16			theIndexL;						
	Signed16			theIndexM;
	rpHttpRequestPtr 	theRequestPtr;
	asSnmpAccessItemPtr	theSaveAccessPtr;
	void				*theValuePtr;						
#if (RpTargetSnmp == eRpSnmpResearch)
	VarBind				*theVarBindPtr;
	unsigned char		theViewName[] = "All";
	OctetString			theViewNameOctetString;
	OID					*theAccessOIDPtr;
	OID					*theRetrievedOIDPtr;
	ObjectSyntax		*theMIBValuePtr;

	theViewNameOctetString.octet_ptr = theViewName;
	theViewNameOctetString.length = 3;
#endif
	
	theDataPtr = (rpDataPtr) theTaskDataPtr;
	theRequestPtr = theDataPtr->fCurrentHttpRequestPtr;

	if (!theRequestPtr->fSnmpInitialized) {
		theRequestPtr->fSnmpInitialized = True;
		/*
			Do any setup needed for the first 
			SNMP access on a page.
		*/
	}

	/*
		Set up the index values
	*/	
	theIndexI = *(theIndexValuesPtr++);
	theIndexJ = *(theIndexValuesPtr++);
	theIndexK = *(theIndexValuesPtr++);
	theIndexL = *(theIndexValuesPtr++);
	theIndexM = *(theIndexValuesPtr++);

	theGetNextIndex = theSnmpAccessPtr->fGetNextIndex;
	if (theGetNextIndex == -1) {
		/*
			This is a Get request, so use the passed in OID.
		*/
		theGetNextFlag = False;
		theAccessPtr = theSnmpAccessPtr;
	}
	else {
		/*
			This is a GetNext request, so the first time use
			the passed in OID.  On subsequent passes, use the
			OID retrieved from the GetNext operation.
		*/
		theGetNextFlag = True;
		theSaveAccessPtr = &theRequestPtr->fSnmpAccess[theGetNextIndex];
		if (theRequestPtr->fSnmpAccess[theGetNextIndex].fCount == 0) {
			theAccessPtr = theSnmpAccessPtr;
		}
		else {
			theAccessPtr = &theRequestPtr->fSnmpAccess[theGetNextIndex];
		}
	}
	/*
		theAccessPtr now points to the RomPager SNMP access structure to
		use to get the MIB object.
		
		typedef struct {
			Signed32		fArray[32];		OID array 
			Signed32		fCount;			Item count in OID array 
			Signed8			fGetNextIndex;	Index for GetNext storage
		} asSnmpAccessItem, *asSnmpAccessItemPtr;
	*/

#if (RpTargetSnmp == eRpSnmpResearch)

	theAccessOIDPtr = ConvertToSnmpResearchOID(theAccessPtr);
 	if (theGetNextFlag) {
		theVarBindPtr = GetNextObjectInstance(theAccessOIDPtr,
      			&theViewNameOctetString, NULL, 
      			theRequestPtr->fSerial, SR_SNMPv1_PDU_PAYLOAD);
	}			
	else {
		theVarBindPtr = GetExactObjectInstance(theAccessOIDPtr,
      			&theViewNameOctetString, NULL, 
      			theRequestPtr->fSerial, SR_SNMPv1_PDU_PAYLOAD);
	}			
    if (theVarBindPtr != (VarBind *) 0) {
		/*
			If this is a GetNext request, save away the 
			retrieved OID to use for the next retrieval
		*/
	 	if (theGetNextFlag) {
	        theRetrievedOIDPtr = theVarBindPtr->name;
	        SaveFromSnmpResearchOID(theRetrievedOIDPtr, theSaveAccessPtr);
	    }

		/*
			Get the value from this instance.  We could look at
			the data type that was passed in to see if it matches,
			but we assume the user is perfect, for now.
		*/
		theMIBValuePtr = &theVarBindPtr->value;		
		switch (theMIBValuePtr->type) {
			case COUNTER_TYPE:
			case GAUGE_TYPE:
			case TIME_TICKS_TYPE:
				theValuePtr = &theMIBValuePtr->ul_value;
				break;

			case OCTET_PRIM_TYPE:
			case IP_ADDR_PRIM_TYPE:
			case OPAQUE_PRIM_TYPE:
				theValuePtr = theMIBValuePtr->os_value;
				break;

			case OBJECT_ID_TYPE:
				theValuePtr = theMIBValuePtr->oid_value;
				break;

			case COUNTER_64_TYPE:
				theValuePtr = theMIBValuePtr->uint64_value;
				break;

			case INTEGER_TYPE:

			default:
				theValuePtr = &theMIBValuePtr->sl_value;
				break;
		}
	}
#endif
	return theValuePtr;				
#else
	return (void *) 0;				
#endif	/* RomPagerServer */
}

/*
	This function is called by the various storage routines in AsVarAcc.c.
	The routine is passed in a SNMP access structure, various engine 
	information, and a pointer to the value to be stored.
*/ 

void AsStoreSnmpItem(void *theTaskDataPtr, 
							char *theHtmlNamePtr, 
							Signed16Ptr theIndexValuesPtr,
							asTextType theItemDataType,
							asSnmpAccessItemPtr theSnmpAccessPtr,
							void *theValuePtr) {

#if RomPagerServer
	rpDataPtr			theDataPtr;
	Signed16			theIndexI;						
	Signed16			theIndexJ;						
	Signed16			theIndexK;						
	Signed16			theIndexL;						
	Signed16			theIndexM;						
	rpHttpRequestPtr 	theRequestPtr;
	
	theDataPtr = (rpDataPtr) theTaskDataPtr;
	theRequestPtr = theDataPtr->fCurrentHttpRequestPtr;

	if (!theRequestPtr->fSnmpInitialized) {
		theRequestPtr->fSnmpInitialized = True;
		/*
			Do any setup needed for the first 
			SNMP access in a form.
		*/
	}

	/*
		Set up the index values
	*/	
	theIndexI = *(theIndexValuesPtr++);
	theIndexJ = *(theIndexValuesPtr++);
	theIndexK = *(theIndexValuesPtr++);
	theIndexL = *(theIndexValuesPtr++);
	theIndexM = *(theIndexValuesPtr++);
#endif	/* RomPagerServer */

	return;				
}


#if RomPagerServer
void RpSendOutSnmpDisplay(rpHttpRequestPtr theRequestPtr,
										asSnmpAccessItemPtr theSnmpAccessPtr) {
	char *					theBytesPtr;
	asSnmpAccessItemPtr		theSaveAccessPtr;
	asSnmpAccessItemPtr		theAccessPtr;
	Boolean					theGetNextFlag;
	Signed8					theGetNextIndex;
			
#if (RpTargetSnmp == eRpSnmpResearch)
	VarBind					*theVarBindPtr;
	unsigned char			theViewName[] = "All";
	OctetString				theViewNameOctetString;
	OID						*theAccessOIDPtr;
	OID						*theRetrievedOIDPtr;

	theViewNameOctetString.octet_ptr = theViewName;
	theViewNameOctetString.length = 3;
#endif

	theGetNextIndex = theSnmpAccessPtr->fGetNextIndex;
	if (theGetNextIndex == -1) {
		/*
			This is a Get request, so use the passed in OID.
		*/
		theGetNextFlag = False;
		theAccessPtr = theSnmpAccessPtr;
	}
	else {
		/*
			This is a GetNext request, so the first time use
			the passed in OID.  On subsequent passes, use the
			OID retrieved from the GetNext operation.
		*/
		theGetNextFlag = True;
		theSaveAccessPtr = &theRequestPtr->fSnmpAccess[theGetNextIndex];
		if (theRequestPtr->fSnmpAccess[theGetNextIndex].fCount == 0) {
			theAccessPtr = theSnmpAccessPtr;
		}
		else {
			theAccessPtr = &theRequestPtr->fSnmpAccess[theGetNextIndex];
		}
	}
	
	/*
		theAccessPtr now points to the RomPager SNMP access structure to
		use to get the MIB object.
		
		typedef struct {
			Signed32		fArray[32];		OID array 
			Signed32		fCount;			Item count in OID array 
			Signed8			fGetNextIndex;	Index for GetNext storage
		} asSnmpAccessItem, *asSnmpAccessItemPtr;
	*/

	
#if (RpTargetSnmp == eRpSnmpGeneric)
		theBytesPtr = "SNMP Data";
#endif
	
#if (RpTargetSnmp == eRpSnmpResearch)

	theAccessOIDPtr = ConvertToSnmpResearchOID(theAccessPtr);
 	if (theGetNextFlag) {
		theVarBindPtr = GetNextObjectInstance(theAccessOIDPtr,
      			&theViewNameOctetString, NULL, 
      			theRequestPtr->fSerial, SR_SNMPv1_PDU_PAYLOAD);
	}			
	else {
		theVarBindPtr = GetExactObjectInstance(theAccessOIDPtr,
      			&theViewNameOctetString, NULL, 
      			theRequestPtr->fSerial, SR_SNMPv1_PDU_PAYLOAD);
	}			
    if (theVarBindPtr == (VarBind *) 0) {
		theBytesPtr = kBadSnmpGetRequest;
    }
	else {
		/*
			If this is a GetNext request, save away the 
			retrieved OID to use for the next retrieval
		*/
	 	if (theGetNextFlag) {
	        theRetrievedOIDPtr = theVarBindPtr->name;
	        SaveFromSnmpResearchOID(theRetrievedOIDPtr, theSaveAccessPtr);
	    }

		/*
			turn the VarBind into a string of the form
			'MIB_Name = MIB_Value'
		*/
		SPrintVarBind(theVarBindPtr, gOID_ValueString);
		FreeVarBind(theVarBindPtr);
		theBytesPtr = gOID_ValueString;

		/*
			we don't want the whole string, so search for the boundary
		*/
		while (*theBytesPtr != ' ') {
			theBytesPtr += 1;			
		}
		/*
			we want the value, so skip past the ' = '
		*/
		theBytesPtr += 3;			
	}
#endif
	RpSendDataOutZeroTerminated(theRequestPtr, theBytesPtr);
	return;
}
#endif	/* RomPagerServer */


#if (RpTargetSnmp == eRpSnmpResearch)
static OID *ConvertToSnmpResearchOID(asSnmpAccessItemPtr theAccessPtr) {
	
	gTempOID.length = theAccessPtr->fCount;
	gTempOID.oid_ptr = theAccessPtr->fArray;
	return &gTempOID;
}

static void SaveFromSnmpResearchOID(OID *theRetrievedOIDPtr, 
								asSnmpAccessItemPtr theSaveAccessPtr) {
	Unsigned32			*theFromPtr;
	Unsigned32			*theToPtr;

    theFromPtr = theRetrievedOIDPtr->oid_ptr;
    theToPtr = theSaveAccessPtr->fArray;
	RP_MEMCPY(theToPtr, theFromPtr, 
			theRetrievedOIDPtr->length * sizeof(Unsigned32));
	theSaveAccessPtr->fCount = theRetrievedOIDPtr->length;
	return;
}

#endif


#endif	/* AsSnmpAccess */
