/* Created with PageBuilder version 4.2 on Mon Nov 17 12:51:25 2003 */

#define C_S_ObjectNotFound	"\377\000"	/* "Object Not Found" */
#define C_S_ProtectedObject	"\377\001"	/* "Protected Object" */
#define C_S_ServerBusy		"\377\002"	/* "Server Busy" */
#define C_S_RequestedUrl	"\377\003"	/* "The requested URL \'" */
#define C_S_WasNotFound		"\377\004"	/* "\' was not found on the " */
#define C_S_ReturnTo		"\377\005"	/* "Return to " */
#define C_S_LastPage		"\377\006"	/* "last page" */
#define C_S_ThisObjectOnThe	"\377\007"	/* "This object on the " */
#define C_S_IsProtected		"\377\010"	/* " is protected" */
#define C_S_Busy1			"\377\011"	/* "The object you requested on the " */
#define C_S_Busy2			"\377\012"	/* " is being used by someone else.  Please try again later" */
#define C_S_Error1			"\377\013"	/* "The item was not found" */
#define C_S_Error2			"\377\014"	/* "This single" C_S_NotRecognized */
#define C_S_Error3			"\377\015"	/* "This multiple" C_S_NotRecognized */
#define C_S_Error4			"\377\016"	/* "Illegal hexadecimal character was detected" */
#define C_S_Error5			"\377\017"	/* "An odd number of hexadecimal characters were detected" */
#define C_S_Error6			"\377\020"	/* "A colon was expected following a number" */
#define C_S_Error7			"\377\021"	/* "Extra characters were detected in the number" */
#define C_S_Error8			"\377\022"	/* C_S_TheNumberToo C_S_small C_S_ForSigned "8" C_S_BitInteger */
#define C_S_Error9			"\377\023"	/* C_S_TheNumberToo C_S_large C_S_ForSigned "8" C_S_BitInteger */
#define C_S_Error10			"\377\024"	/* C_S_TheNumberToo C_S_small C_S_ForSigned "16" C_S_BitInteger */
#define C_S_Error11			"\377\025"	/* C_S_TheNumberToo C_S_large C_S_ForSigned "16" C_S_BitInteger */
#define C_S_Error12			"\377\026"	/* C_S_TheNumberToo C_S_small C_S_ForSigned "32" C_S_BitInteger */
#define C_S_Error13			"\377\027"	/* C_S_TheNumberToo C_S_large C_S_ForSigned "32" C_S_BitInteger */
#define C_S_Error14			"\377\030"	/* C_S_TheNumberToo C_S_small C_S_ForSigned "64" C_S_BitInteger */
#define C_S_Error15			"\377\031"	/* C_S_TheNumberToo C_S_large C_S_ForSigned "64" C_S_BitInteger */
#define C_S_Error16			"\377\032"	/* C_S_TheNumberToo C_S_large C_S_ForUnsigned "8" C_S_BitInteger */
#define C_S_Error17			"\377\033"	/* C_S_TheNumberToo C_S_large C_S_ForUnsigned "16" C_S_BitInteger */
#define C_S_Error18			"\377\034"	/* C_S_TheNumberToo C_S_large C_S_ForUnsigned "32" C_S_BitInteger */
#define C_S_Error19			"\377\035"	/* C_S_TheNumberToo C_S_large C_S_ForUnsigned "64" C_S_BitInteger */
#define C_S_Error20			"\377\036"	/* "A decimal digit" C_S_WasExpected */
#define C_S_Error21			"\377\037"	/* "An integer" C_S_WasExpected */
#define C_S_Error22			"\377\040"	/* "An Extended ACSII character was detected" */
#define C_S_TheNumberToo	"\377\041"	/* "The number was too " */
#define C_S_small			"\377\042"	/* "small " */
#define C_S_large			"\377\043"	/* "large " */
#define C_S_ForSigned		"\377\044"	/* "for a signed " */
#define C_S_ForUnsigned		"\377\045"	/* "for an unsigned " */
#define C_S_BitInteger		"\377\046"	/* " bit integer" */
#define C_S_NotRecognized	"\377\047"	/* " selection option was not recognized" */
#define C_S_WasExpected		"\377\050"	/* " was expected" */
#define C_S_EntryError		"\377\051"	/* "Entry Error" */
#define C_S_ErrorDetected	"\377\052"	/* "An Error Was Detected On The Previous Page" */
#define C_S_HtmlItem		"\377\053"	/* "HTML Item " */
#define C_S_Name			"\377\054"	/* "name: " */
#define C_S_Value			"\377\055"	/* "value: " */
#define C_S_Enabled			"\377\056"	/* "Enabled" */
#define C_S_Disabled		"\377\057"	/* "Disabled" */
#define C_S_ServerError		"\377\060"	/* "Server Error" */
#define C_S_FSErrorDetected	"\377\061"	/* "A file system error was detected on the " */
#define C_S_UnexpectedMp	"\377\062"	/* "Unexpected multipart form data" */
#define C_S_GeneralError	"\377\063"	/* ": general error" */
#define C_S_DupFilename		"\377\064"	/* ": duplicate filename" */
#define C_S_DiskFull		"\377\065"	/* ": disk full" */
#define C_S_InvalidTag		"\377\066"	/* "SoftPage Error: Invalid tag." */
#define C_S_TooFewResources	"\377\067"	/* "SoftPage Error: Insufficient device resources for request." */
#define C_S_NoSuchName		"\377\070"	/* "SoftPage Error: No device item matches Name value, " */
#define C_S_TypeMismatch	"\377\071"	/* "SoftPage Error: Request type doesn\'t match device item type -- " */
#define C_S_Reserved1		"\377\072"	/* "" */
#define C_S_Reserved2		"\377\073"	/* "" */
#define C_S_Reserved3		"\377\074"	/* "" */
#define C_S_Reserved4		"\377\075"	/* "" */
#define C_S_Reserved5		"\377\076"	/* "" */
#define C_S_Reserved6		"\377\077"	/* "" */
#define C_S_Reserved7		"\377\100"	/* "" */
#define C_S_RomPager		"\377\101"	/* "RomPager" */
#define C_S_Allegro			"\377\102"	/* "Allegro" */
#define C_S_AllegroLogo		"\377\103"	/* C_oHR C_oP C_oCENTER C_oIMG_SRC "/Images/Main\"" C_WIDTH "160" C_HEIGHT "80>" C_xCENTER */
#define C_S_NBSP4			"\377\104"	/* C_NBSP C_NBSP C_NBSP C_NBSP */
#define C_S_NBSP8			"\377\105"	/* C_S_NBSP4 C_S_NBSP4 */
#define C_S_NBSP12			"\377\106"	/* C_S_NBSP8 C_S_NBSP4 "\n" */
#define C_S_ConnectionParm	"\377\107"	/* "stcpConnection theConnection" */
#define C_S_StcpCallStart	"\377\110"	/* "extern RpErrorCode Stcp" */
#define C_S_IPAddress		"\377\111"	/* "IP address" */
#define C_S_Position_Absolute	"\377\112"	/* "position:absolute;" */
#define C_S_Visibility_Hidden	"\377\113"	/* "visibility:\"hidden\";" */

