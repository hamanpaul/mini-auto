/*
 *	File:		Sfil.c
 *
 *	Contains:	Simple file system interface routines for standard C library
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Copyright:	© 1997-2001 by Allegro Software Development Corporation
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
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *	Change History (most recent first):
 *
 *		01/14/00	bva		ConvertFileExtensionToMimeType -> 
 *								RpConvertFileExtensionToMimeType
 *		08/12/98	rhb		created from Macintosh version
 * * * * Release 2.1 * * * *
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

#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include "AsExtern.h"

#include "Sfil.h"
#include "SfilInt.h"


static SfsLength	gFileCount;
static FileDataPtr	gFileDataPtr;



/*
 	The SfsOpenFileSystem routine is called once when RomPager starts up.  The 
	maximum number of simultaneous open files is passed in to the file system 
	so that the file system interface can dynamically initialize it's internal 
	variables and processes.  The number passed in will usually be the same as 
	the number of simultaneous HTTP requests that the RomPager engine supports. 
 	
 	Inputs:
 		theOpenFilesCount:			- number of allowed open files 		

 	Returns:
		theResult:
	 		eRpNoError				- no error
	 		eRpFileSystemNotOpen	- can't initialize the file system
*/

RpErrorCode SfsOpenFileSystem(SfsLength theOpenFilesCount) {
	RpErrorCode	theResult;

	theResult = eRpNoError;
#if 0
	gFileDataPtr = (FileDataPtr) malloc(theOpenFilesCount * sizeof(FileData));
	if (gFileDataPtr == (FileDataPtr) 0) {
		theResult = eRpFileSystemNotOpen;
	}
	else {
		memset(gFileDataPtr, 0, theOpenFilesCount * sizeof(FileData));
		gFileCount = theOpenFilesCount;
	}
#endif
	return theResult;
}


/*
 	The SfsCloseFileSystem routine is called once at when RomPager finishes so 
	that the file system can deinitialize any internal variables and processes.
 	
 	Returns:
		theResult:
	 		eRpNoError				- no error
	 		eRpFileSystemNotClosed	- can't deinitialize file system
*/

RpErrorCode SfsCloseFileSystem(void) {
	FileDataPtr		theFileDataPtr;
	SfsFileNumber	theFileIndex;
	RpErrorCode		theResult;
	theResult = eRpNoError;
#if 0
	if (gFileDataPtr == (FileDataPtr) 0) {
		theResult = eRpFileSystemNotOpen;
	}
	else {
		for (theFileIndex = 0, theFileDataPtr = gFileDataPtr; 
				theFileIndex < gFileCount; 
				theFileDataPtr += 1, theFileIndex += 1) {
			if (theFileDataPtr->fFilePtr != (FILE *) 0) {
				/*
					try to close the file
				*/
				SfsCloseFile(theFileIndex, False);
				theResult = eRpFileSystemNotClosed; /* FileNotClosed? */
			}
		}
		if (theResult == eRpNoError) {
			free(gFileDataPtr);
		}
	}
#endif
	return theResult;
}


/*
 	The SfsOpenFile routine is called to open an individual file.  The file byte 
 	position is set to 1.  The open file call is responsible for all directory 
 	positioning, since the full file name from the URL will be passed in.  An 
 	example file name is: /MyFirstDirectory/TheSecondDirectory/MyFile.  
 	
 	Inputs:
 		theFileNumber:			- file id 		
 		theFullNamePtr:			- pointer to full URL object name		

 	Returns:
		theResult:
	 		eRpNoError			- no error
	 		eRpFileNotFound		- can't find this file to open
	 		eRpFileOpenError	- can't open file
*/

RpErrorCode SfsOpenFile(SfsFileNumber theFileNumber, 
		char *theFullNamePtr) {
	FileDataPtr	theFileDataPtr;
	int			theFileNameLength;
	RpErrorCode	theResult;
	char 		*theFileNameEndPtr;
	
	theResult = eRpNoError;
#if 0
	if (gFileDataPtr == (FileDataPtr) 0) {
		theResult = eRpFatalFileSystemError;
	}
	else {
		theFileDataPtr = gFileDataPtr + theFileNumber;
		theFileNameEndPtr = theFullNamePtr + strlen(theFullNamePtr);
		/*
			Get just the name without any leading slashes
		*/
		while (theFileNameEndPtr >= theFullNamePtr) {
			if (*theFileNameEndPtr == '/') {
				/*
					exit the loop pointing at the slash
				*/
				break;
			}
			else {
				theFileNameEndPtr -= 1;
			}
		}
		theFileNameEndPtr += 1;
		theFileNameLength = strlen(theFileNameEndPtr);
		if (theFileNameLength < 1) {
			theResult = eRpFileOpenError;
		}
		else {
			theFileDataPtr->fFileType = 
					RpConvertFileExtensionToMimeType(theFileNameEndPtr);
			theFileDataPtr->fFilePtr = fopen(theFileNameEndPtr, "rb");
			if (theFileDataPtr->fFilePtr == (FILE *) 0) {
				theResult = eRpFileOpenError;
			}
		}
	}
#endif
	return theResult;
}

/*
 	The SfsOpenStatus routine is called to determine when the file has 
 	been opened.  
 	
 	Inputs:
 		theFileNumber:			- file id 		
 		theCompletionStatusPtr:	- pointer to the call status
 		theFileInfoPtr:			- pointer to an empty file info block
	
 	Returns:
 		theCompletionStatusPtr:	- eSfsComplete, if the file is open
 								- eSfsPending, if the file is not open yet 	
 		theFileInfoPtr:			- pointer to a completed file info block
		theResult:
	 		eRpNoError			- no error
	 		eRpFileNotFound		- can't find this file to open
	 		eRpFileOpenError	- can't open file (it does exist)
	 		eRpFileSystemError	- can't open file (file system corrupted)
*/

RpErrorCode SfsOpenStatus(SfsFileNumber theFileNumber, 
		SfsStatus *theCompletionStatusPtr, 
		SfsFileInfo *theFileInfoPtr) {
	FileDataPtr	theFileDataPtr;
	RpErrorCode	theResult;

	theResult = eRpNoError;
#if 0

	if (gFileDataPtr == (FileDataPtr) 0) {
		theResult = eRpFatalFileSystemError;
	}
	else {
		theFileDataPtr = gFileDataPtr + theFileNumber;
		theFileInfoPtr->fFileType   = theFileDataPtr->fFileType;
		theFileInfoPtr->fFileAccess = 0;	/* Unprotected */
		theFileInfoPtr->fFileDate   = 0;	/* Unknown */

		/*
		 *	NB: Harbbison & Steele (fourth edition) says, "ISO C does not
		 *		require implementations to 'meaningfully' support a wherefrom
		 *		value of SEEK_END for binary streams." I suppose this means
		 *		that the fseek() below may not actually position the file to 
		 *		its end and the file size would be incorrectly computed.
		 */
		fseek(theFileDataPtr->fFilePtr, 0L, SEEK_END);
		theFileInfoPtr->fFileSize = ftell(theFileDataPtr->fFilePtr);
		rewind(theFileDataPtr->fFilePtr);
		*theCompletionStatusPtr = eSfsComplete;
	}
#endif
	return theResult;
}



/*
 	The SfsCreateFile routine is called to open a new file on the file system.  
 	The full URL name is passed in the same format as the SfsOpenFile routine, 
 	and a file information block contains the rest of the information necessary 
 	to create the file. 
 	
 	Inputs:
 		theFileNumber:			- file id 		
 		theFullNamePtr:			- pointer to full URL object name				
 		theFileInfoPtr:			- pointer to a completed file info block

 	Returns:
		theResult:
	 		eRpNoError			- no error
	 		eRpFileCreateError	- can't create file
*/

RpErrorCode SfsCreateFile(SfsFileNumber theFileNumber,
		char *theFullNamePtr, SfsFileInfo *theFileInfoPtr) {
	FileDataPtr		theFileDataPtr;
	char *			theNameStringPtr;
	RpErrorCode		theResult;


	theResult = eRpNoError;
#if 0

	if (gFileDataPtr == (FileDataPtr) 0) {
		theResult = eRpFatalFileSystemError;
	}
	else {
		theFileDataPtr = gFileDataPtr + theFileNumber;
		if (theResult == eRpNoError) {
			theNameStringPtr = theFullNamePtr + strlen(theFullNamePtr);

			while (theNameStringPtr >= theFullNamePtr) {
				if (*theNameStringPtr == kAscii_Backslash ||
					*theNameStringPtr == kAscii_Colon) {
					break;
				}
				else {
					theNameStringPtr -= 1;
				}
			}

			theNameStringPtr += 1;
			if (strlen(theNameStringPtr) < 1) {
				theResult = eRpFileCreateError;
			}
			else {

				/* create the file */
				theFileDataPtr->fFilePtr = fopen(theNameStringPtr, "wb");
				if (theFileDataPtr->fFilePtr == (FILE *) 0) {
					theResult = eRpFileCreateError;
				}
			}
		}
	}
#endif
	return theResult;
}


/*
 	The SfsCreateStatus routine is called to determine whether the SfsCreateFile 
 	call has completed.  
 	
 	Inputs:
 		theFileNumber:			- file id 		
 		theCompletionStatusPtr:	- pointer to the call status			 	

 	Returns:
 		theCompletionStatusPtr:	- eSfsComplete, the file has been created
 								- eSfsPending, the file is not open yet 		
 		theResult:
	 		eRpNoError			- no error
	 		eRpFileCreateError	- can't create file
*/

RpErrorCode SfsCreateStatus(SfsFileNumber theFileNumber, 
		SfsStatus *theCompletionStatusPtr) {
	RpErrorCode		theResult;

	theResult = eRpNoError;
	if (gFileDataPtr == (FileDataPtr) 0) {
		theResult = eRpFatalFileSystemError;
	}
	else {
		*theCompletionStatusPtr = eSfsComplete;
	}
	return theResult;
}



/*
 	The SfsCloseFile routine is used to signal the end of usage for a particular 
 	file.  The file system should close the file and initialize any internal 
 	variables pointed to by the file number.  This call is asynchronous and 
 	completes when the close has been started.  The SfsCloseStatus call is 
 	used to detect the completion of the close.  theCompleteFlag is used to
 	signal whether all data was received from the application. 
 	
 	Inputs:
 		theFileNumber:			- file id 		
		theCompleteFlag			
			True (1)			- file is complete
			False (0)			- file is incomplete
 	
 	Returns:
		theResult:
	 		eRpNoError			- no error
	 		eRpFileNotOpen		- this file number not open
	 		eRpFileCloseError	- can't close file
*/

RpErrorCode SfsCloseFile(SfsFileNumber theFileNumber, int theCompleteFlag) {
	FileDataPtr		theFileDataPtr;
	RpErrorCode		theResult;

	theResult = eRpNoError;
#if 0
	if (gFileDataPtr == (FileDataPtr) 0) {
		theResult = eRpFatalFileSystemError;
	}
	else {
		theFileDataPtr = gFileDataPtr + theFileNumber;
		if (theFileDataPtr->fFilePtr == (FILE *) 0) {
			theResult = eRpFileNotOpen;
		}
		else {
			if (fclose(theFileDataPtr->fFilePtr) == EOF) {
				theResult = eRpFileCloseError;
			}
		}
	} 
#endif
	return theResult;
}


/*
 	The SfsCloseStatus routine is called to determine whether a file has 
 	been closed.  		

 	Inputs:
 		theFileNumber:			- file id 		
 		theCompletionStatusPtr:	- pointer to the call status			 	

 	Returns:
 		theCompletionStatusPtr:	- eSfsComplete, the file has been closed
 								- eSfsPending, the file is still open			
 		theResult:
	 		eRpNoError			- no error
	 		eRpFileNotOpen		- this file number not open
	 		eRpFileCloseError	- can't close file
*/

RpErrorCode SfsCloseStatus(SfsFileNumber theFileNumber, 
		SfsStatus *theCompletionStatusPtr) {
	FileDataPtr		theFileDataPtr;
	RpErrorCode		theResult;

	theResult = eRpNoError;
#if 0
	if (gFileDataPtr == (FileDataPtr) 0) {
		theResult = eRpFatalFileSystemError;
	}
	else {
		theFileDataPtr = gFileDataPtr + theFileNumber;
		if (theFileDataPtr->fFilePtr == (FILE *) 0) {
			theResult = eRpFileNotOpen;
		}
		else {
			theFileDataPtr->fFilePtr = (FILE *) 0;
			*theCompletionStatusPtr = eSfsComplete;
		}
	}
#endif
	return theResult;
}


/*
 	The SfsReadFile routine is called to start a read into the buffer provided 
 	for the number of bytes in the count.  The read takes place at the current 
 	file byte position with the file byte position being updated after the read 
 	completes.  
 	
 	Inputs:
 		theFileNumber:			- file id 		
 		theReadPtr:				- pointer to the read buffer					
 		theByteCount:			- number of bytes to read
		
 	Returns:
 		theResult:
	 		eRpNoError				- no error
	 		eRpFileNotOpen			- this file number not open
	 		eRpFileInvalidPosition	- can't read at this position
											(possibly after EOF)
	 		eRpFileReadError			- can't read file
*/

RpErrorCode SfsReadFile(SfsFileNumber theFileNumber, 
		char *theReadPtr, SfsLength theByteCount) {
	FileDataPtr		theFileDataPtr;
	RpErrorCode		theResult;


	theResult = eRpNoError;
#if 0

	if (gFileDataPtr == (FileDataPtr) 0) {
		theResult = eRpFatalFileSystemError;
	}
	else {
		theFileDataPtr = gFileDataPtr + theFileNumber;
		if (theFileDataPtr->fFilePtr == (FILE *) 0) {
			theResult = eRpFileNotOpen;
		}
		else {
			theFileDataPtr->fBytesRead = fread(theReadPtr, 1, theByteCount, 
				theFileDataPtr->fFilePtr);
		}
	} 
#endif
	return theResult;
}

/*
	The SfsReadStatus routine is called to determine whether the read 
	has finished.
 	
 	Inputs:
  		theFileNumber:			- file id 		
 		theCompletionStatusPtr:	- pointer to the call status			 		
 		theBytesReadPtr:		- pointer to length field				

 	Returns:
 		theCompletionStatusPtr:	- eSfsComplete, the file has been read
 								- eSfsEndOfFile, the file has been read,
									and there are no more bytes to be read.		
 								- eSfsPending, the read is outstanding	 		
 		theBytesReadPtr:		- the number of bytes actually read is
									stored in the caller's length field.
		theResult:
	 		eRpNoError				- no error
	 		eRpFileNotOpen			- this file number not open
	 		eRpFileInvalidPosition	- can't read at this position
											(possibly after EOF)
	 		eRpFileReadError			- can't read file
*/

RpErrorCode SfsReadStatus(SfsFileNumber theFileNumber, 
		SfsStatus *theCompletionStatusPtr, 
		SfsLength *theBytesReadPtr) {
	FileDataPtr		theFileDataPtr;
	RpErrorCode		theResult;

	theResult = eRpNoError;
#if 0
	if (gFileDataPtr == (FileDataPtr) 0) {
		theResult = eRpFatalFileSystemError;
	}
	else {
		theFileDataPtr = gFileDataPtr + theFileNumber;
		if (theFileDataPtr->fFilePtr == (FILE *) 0) {
			theResult = eRpFileNotOpen;
		}
		else {
			*theBytesReadPtr = theFileDataPtr->fBytesRead;
			if (theFileDataPtr->fBytesRead == 0) {
				if (feof(theFileDataPtr->fFilePtr)) {
					*theCompletionStatusPtr = eSfsEndOfFile;			
				}
				else {
					theResult = eRpFileReadError;
				}
			}
			else {
				*theCompletionStatusPtr = eSfsComplete;
			}
		}
	}
#endif
	return theResult;
}


/*
 	The SfsWriteFile routine is called to start a write from the buffer provided
 	for the number of bytes in the count.  The write takes place at the current 
 	file byte position with the file byte position being updated after the write 
 	completes.  
 	
 	Inputs:
 		theFileNumber:			- file id 		
 		theWritePtr:			- pointer to the write buffer					
 		theByteCount:			- number of bytes to write
		
 	Returns:
 		theResult:
	 		eRpNoError			- no error
	 		eRpFileNotOpen		- this file number not open
	 		eRpFileWriteError		- can't write file
*/

RpErrorCode SfsWriteFile(SfsFileNumber theFileNumber, 
		char *theWritePtr, SfsLength theByteCount) {
	FileDataPtr		theFileDataPtr;
	RpErrorCode		theResult;
	//FS_size_t			theBytesWritten;

	theResult = eRpNoError;
#if 0
	if (gFileDataPtr == (FileDataPtr) 0) {
		theResult = eRpFatalFileSystemError;
	}
	else {
		theFileDataPtr = gFileDataPtr + theFileNumber;
		if (theFileDataPtr->fFilePtr == (FILE *) 0) {
			theResult = eRpFileNotOpen;
		}
		else {
			theBytesWritten = fwrite(theWritePtr, 1, theByteCount, 
					theFileDataPtr->fFilePtr);
			if (theBytesWritten != theByteCount) {
				theResult = eRpFileWriteError;
			}
		}
	} 
#endif
	return theResult;
}


/*
 	The SfsWriteStatus routine is called to determine whether the write 
 	has finished.
 	
 	Inputs:
  		theFileNumber:			- file id 		
 		theCompletionStatusPtr:	- pointer to the call status			 		
 		theBytesWrittenPtr:		- pointer to length field				
 	Returns:
 		theCompletionStatusPtr:	- eSfsComplete, the file has been written
 								- eSfsPending, the write is outstanding	 		
 		theBytesWrittenPtr:		- the number of bytes actually written is
									stored in the caller's length field.
		theResult:
	 		eRpNoError			- no error
	 		eRpFileNotOpen		- this file number not open
	 		eRpFileWriteError	- can't write file
*/

RpErrorCode SfsWriteStatus(SfsFileNumber theFileNumber, 
		SfsStatus *theCompletionStatusPtr, 
		SfsLength *theBytesWrittenPtr) {
	RpErrorCode		theResult;

	theResult = eRpNoError;
#if 0
	if (gFileDataPtr == (FileDataPtr) 0) {
		theResult = eRpFatalFileSystemError;
	}
	else {
		if ((gFileDataPtr + theFileNumber)->fFilePtr == (FILE *) 0) {
			theResult = eRpFileNotOpen;
		}
		*theCompletionStatusPtr = eSfsComplete;
	}
#endif
	return theResult;
}


/*
 	The SfsSetFilePosition routine is called to set a new current file byte
	position for subsequent reads or writes.  
 	
 	Inputs:
 		theFileNumber:			- file id 		
		theBytePosition:		- the new file byte position
		
 	Returns:
 		theResult:
	 		eRpNoError				- no error
	 		eRpFileNotOpen			- this file number not open
	 		eRpFileInvalidPosition	- can't set to this position
*/

RpErrorCode SfsSetFilePosition(SfsFileNumber theFileNumber,
		SfsLength theBytePosition) {
	FileDataPtr		theFileDataPtr;
	RpErrorCode		theResult;

	theResult = eRpNoError;
#if 0
	if (gFileDataPtr == (FileDataPtr) 0) {
		theResult = eRpFatalFileSystemError;
	}
	else {
		theFileDataPtr = gFileDataPtr + theFileNumber;
		if (theFileDataPtr->fFilePtr == (FILE *) 0) {
			theResult = eRpFileNotOpen;
		}
	}
	if (theResult == eRpNoError) {
		if (fseek(theFileDataPtr->fFilePtr, theBytePosition, SEEK_SET) != 0) {
			theResult = eRpFileInvalidPosition;
		}
	} 
#endif
	return theResult;
}

/*
	The SfsSetFilePositionStatus routine is called to determine whether the 
	positioning operation has finished.
 	
 	Inputs:
  		theFileNumber:			- file id 		
 		theCompletionStatusPtr:	- pointer to the call status			 	
 	Returns:
 		theCompletionStatusPtr:	- eSfsComplete, the position has been set
 								- eSfsPending, the call is outstanding			
 		theResult:
	 		eRpNoError				- no error
	 		eRpFileNotOpen			- this file number not open
	 		eRpFileInvalidPosition	- can't set to this position
*/

RpErrorCode SfsSetFilePositionStatus(SfsFileNumber theFileNumber, 
		SfsStatus *theCompletionStatusPtr) {
	RpErrorCode		theResult;

	theResult = eRpNoError;
#if 0
	if (gFileDataPtr == (FileDataPtr) 0) {
		theResult = eRpFatalFileSystemError;
	}
	else {
		if ((gFileDataPtr + theFileNumber)->fFilePtr == (FILE *) 0) {
			theResult = eRpFileNotOpen;
		}
		*theCompletionStatusPtr = eSfsComplete;
	}
#endif
	return theResult;
}

