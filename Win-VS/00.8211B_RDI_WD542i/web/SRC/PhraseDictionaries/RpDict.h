/*
 *	File:		RpDict.h
 *
 *	Contains:	Phrase dictionary definitions
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *	Copyright:	© 1995-2002 by Allegro Software Development Corporation
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
 *		06/13/00	bva		add  BGCOLOR="# tag to system dictionary
 * * * * Release 3.0 * * * *
 * * * * Release 2.0 * * * *
 *		11/17/97	bva		add <DIV> tags to system dictionary
 *		06/05/97	bva		add kRpCompressionEscape, remove kRpUserPhraseStart
 * * * * Release 1.6 * * * *
 *		04/14/97	bva		add newlines to close tags in dictionary
 *		02/28/97	bva		add frame definitions
 *		02/22/97	bva		update documentation
 * * * * Release 1.5 * * * *
 *		11/01/96	bva		created from RpPages.h
 * * * * Release 1.4 * * * *
 *
 *	To Do:
 */

#ifndef	_RPDICT_
#define	_RPDICT_

/*
	The RomPager engine uses a phrase dictionary technique to provide 
	compression for the static ASCII text strings.  As elements of type 
	eRpItemType_DataZeroTerminated are served, they are examined for 
	characters with the high-bit set and replacement phrases are substituted.  
	Normal ASCII characters are in the range <000> to <177> and pass through
	the dictionary expansion process unchanged. For international pages 
	(such as the SJIS form of Japanese) that use the high-bit characters, 
	the international HTML is stored using the eRpItemType_ExtendedAscii 
	element type, and are not passed through the dictionary expansion process.  

	There are two dictionaries, the system dictionary which uses single 
	character tokens, and the user dictionary which uses double character 
	tokens.
	  
	The Extended ASCII characters of <200> to <372> are used by the 
	built-in system dictionary.  Each of these character is a direct index
	to a commonly used HTML tag or other phrase.  There are 123 possible
	system dictionary entries of which 92 are currently used.
	
	The Extended ASCII character of <373> (defined by kRpCompressionEscape) is 
	used to signal that the following character should not be passed through 
	the dictionary expansion process.

	The characters starting with <374> to <377> are used to signal a user 
	dictionary phrase.  In the user dictionary, the next character is used 
	to index into the user phrase dictionary.  
	
	There are four sets of 256 entries that each take 2 bytes to represent in 
	the source phrase. The first 256 entries are selected with characters in 
	the range <377><000> to <377><377>, the next 256 entries are selected with 
	characters in the range <376><000> to <376><377>, and the fourth set of 256 
	entries are selected are selected with characters in the range <374><000> 
	to <374><377>.
	
	The kRpCompressionEscape value (the default of <373> is defined in 
	RpConfig.h) defines the layout of the phrase dictionaries.  If this value
	were <371>, for instance, the system dictionary would have entries of
	<200> to <370>, and the user dictionary would contain 6 sets of 256 entries
	ranging from <377><000> to <372><377>.
		
	The definitions of the built-in system phrase dictionary are below, and 
	examples of use including the custom dictionary are shown in the 
	RpPages.c file.  Macro definitions such as C_oCENTER and C_xCENTER for 
	<CENTER> and </CENTER> are used to preserve the readability of the HTML 
	in the C source code format.  
	
	These definitions are the mnemonic macros used to represent the phrase 
	dictionary characters.  The data for the phrases is stored in RpData.c
*/

#define C_oP_NBSP_xP			"\200"	/* "<P>&nbsp;</P>\n" */
#define C_xTD_oP_xTR_xTABLE		"\201"	/* "</TD><P></TR></TABLE>\n" */
#define C_xTD_oP_xTR			"\202"	/* "</TD><P></TR>\n" */
#define C_oTR_ALIGN_LEFT		"\203"	/* "<TR ALIGN=left" */
#define C_oTR_ALIGN_CENTER		"\204"	/* "<TR ALIGN=center" */
#define C_oTR_ALIGN_RIGHT		"\205"	/* "<TR ALIGN=right" */
#define C_oTH_ALIGN_LEFT		"\206"	/* "<TH ALIGN=left" */
#define C_oTH_ALIGN_CENTER		"\207"	/* "<TH ALIGN=center" */
#define C_oTH_ALIGN_RIGHT		"\210"	/* "<TH ALIGN=right" */
#define C_oTD_ALIGN_LEFT		"\211"	/* "<TD ALIGN=left" */
#define C_oTD_ALIGN_CENTER		"\212"	/* "<TD ALIGN=center" */
#define C_oTD_ALIGN_RIGHT		"\213"	/* "<TD ALIGN=right" */
#define C_oTD_oBR_xTD			"\214"	/* "<TD><BR></TD>" */
#define C_oTABLE_CELLPADDING	"\215"	/* "<TABLE CELLPADDING=" */
#define C_oTABLE_CELLSPACING	"\216"	/* "<TABLE CELLSPACING=" */
#define C_oTABLE_BORDER			"\217"	/* "<TABLE BORDER=" */
#define C_oTD_oANCHOR_HREF		"\220"	/* "<TD><A HREF=\"" */
#define C_xANCHOR_xTD			"\221"	/* "</A></TD>\n" */
#define C_oHTML_oHEAD_oTITLE	"\222"	/* "<HTML>\n<HEAD>\n<TITLE>" */
#define C_xTITLE_xHEAD_oBODY	"\223"	/* "</TITLE>\n</HEAD>\n<BODY>\n" */
#define C_xBODY_xHTML			"\224"	/* "\n</BODY>\n</HTML>\n" */
#define C_xFORM					"\225"	/* "\n</FORM>" */
#define C_oCENTER				"\226"	/* "<CENTER>" */
#define C_xCENTER				"\227"	/* "</CENTER>\n" */
#define C_oBLOCKQUOTE			"\230"	/* "<BLOCKQUOTE>\n" */
#define C_xBLOCKQUOTE			"\231"	/* "\n</BLOCKQUOTE>\n" */
#define C_oANCHOR_HREF			"\232"	/* "<A HREF=\"" */
#define C_xANCHOR				"\233"	/* "</A>" */
#define C_oIMG_SRC				"\234"	/* "<IMG SRC=\"" */
#define C_oH1					"\235"	/* "<H1>" */
#define C_xH1					"\236"	/* "</H1>\n" */
#define C_oH2					"\237"	/* "<H2>" */
#define C_xH2					"\240"	/* "</H2>\n" */
#define C_oH3					"\241"	/* "<H3>" */
#define C_xH3					"\242"	/* "</H3>\n" */
#define C_oH4					"\243"	/* "<H4>" */
#define C_xH4					"\244"	/* "</H4>\n" */
#define C_oP					"\245"	/* "<P>\n" */
#define C_xP					"\246"	/* "</P>" */
#define C_oHR					"\247"	/* "<HR>\n" */
#define C_NBSP					"\250"	/* "&nbsp;" */
#define C_oBR					"\251"	/* "<BR>\n" */
#define C_oTABLE				"\252"	/* "<TABLE>" */
#define C_xTABLE				"\253"	/* "</TABLE>\n" */
#define C_oTR					"\254"	/* "<TR>" */
#define C_xTR					"\255"	/* "</TR>\n" */
#define C_oTH					"\256"	/* "<TH>" */
#define C_xTH					"\257"	/* "</TH>\n" */
#define C_oTD					"\260"	/* "<TD>" */
#define C_xTD					"\261"	/* "</TD>\n" */
#define C_oCODE					"\262"	/* "<CODE>\n" */
#define C_xCODE					"\263"	/* "\n</CODE>\n" */
#define C_oFONT_SIZE			"\264"	/* "<FONT SIZE=" */
#define C_oB					"\265"	/* "<B>" */
#define C_xB					"\266"	/* "</B>" */
#define C_oI					"\267"	/* "<I>" */
#define C_xI					"\270"	/* "</I>" */
#define C_oHTML					"\271"	/* "<HTML>" */
#define C_xHTML					"\272"	/* "</HTML>\n" */
#define C_oHEAD					"\273"	/* "<HEAD>" */
#define C_xHEAD					"\274"	/* "</HEAD>\n" */
#define C_oMETA					"\275"	/* "<META" */
#define C_oBODY					"\276"	/* "<BODY" */
#define C_xBODY					"\277"	/* "</BODY>\n" */
#define C_oTITLE				"\300"	/* "<TITLE>" */
#define C_xTITLE				"\301"	/* "</TITLE>\n" */
#define C_WIDTH					"\302"	/* " WIDTH=" */
#define C_HEIGHT				"\303"	/* " HEIGHT=" */
#define C_ALIGN_TOP				"\304"	/* " ALIGN=top" */
#define C_ALIGN_MIDDLE			"\305"	/* " ALIGN=middle" */
#define C_ALIGN_BOTTOM			"\306"	/* " ALIGN=bottom" */
#define C_ALIGN_LEFT			"\307"	/* " ALIGN=left" */
#define C_ALIGN_CENTER			"\310"	/* " ALIGN=center" */
#define C_ALIGN_RIGHT			"\311"	/* " ALIGN=right" */
#define C_VALIGN_TOP			"\312"	/* " VALIGN=top" */
#define C_VALIGN_MIDDLE			"\313"	/* " VALIGN=middle" */
#define C_VALIGN_BOTTOM			"\314"	/* " VALIGN=bottom" */
#define C_CELLPADDING			"\315"	/* " CELLPADDING=" */
#define C_BORDER				"\316"	/* " BORDER=" */
#define C_CELLSPACING			"\317"	/* " CELLSPACING=" */
#define C_COLSPAN				"\320"	/* " COLSPAN=" */
#define C_ROWSPAN				"\321"	/* " ROWSPAN=" */
#define C_NAME					"\322"	/* " NAME=" */
#define C_CONTENT				"\323"	/* " CONTENT=" */
#define C_ALT					"\324"	/* " ALT=\"" */
#define C_oTD_COLSPAN			"\325"	/* "<TD COLSPAN=" */
#define C_xFONT					"\326"	/* "</FONT>" */
#define C_oFRAMESET				"\327"	/* "<FRAMESET>" */
#define C_xFRAMESET				"\330"	/* "</FRAMESET>\n" */
#define C_oFRAME_SCROLLING		"\331"	/* "<FRAME SCROLLING=" */
#define C_ALIGN					"\332"	/* " ALIGN=" */
#define C_QUOTE					"\333"	/* "&quot;" */
#define C_oDIV					"\334"	/* "<DIV>" */
#define C_oDIV_					"\335"	/* "<DIV" */
#define C_xDIV					"\336"	/* "</DIV>\n" */
#define C_BGCOLOR				"\337"	/* " BGCOLOR=\"#" */

/*
								"\373"	   reserved
								"\374"	   reserved
								"\375"	   reserved
								"\376"	   reserved
								"\377"	   reserved
*/
								

#endif

