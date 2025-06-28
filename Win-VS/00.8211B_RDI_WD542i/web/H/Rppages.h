/*
 *	File:		RpPages.h
 *
 *	Contains:	Elements and structures used for Rom Pager page descriptions
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Copyright:	© 1995-1997 by Allegro Software Development Corporation
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
 * * * * Release 2.0 * * * *
 *		11/26/97	pjr		add eRpObjectTypeRemote
 *		11/10/97	rhb		add support for 64 bit integers
 *		09/23/97	bva		add eRpItemType_FormNamedSubmit
 *		09/15/97	rhb		add eRpItemType_FormRadioButtonDyn, 
 *								eRpItemType_FormFixedMultiDyn,  
 *								eRpItemType_FormVariableSingleDyn, and
 *								eRpItemType_FormVariableMultiDyn
 *		09/14/97	bva		add eRpItemType_ImageSource
 *		09/09/97	bva		add eRpItemType_FormFixedSingleDyn 
 *		07/31/97	rhb		add eRpItemType_UrlState, eRpItemType_FormTextAreaBuf 
 *		07/16/97	bva		add eRpItemType_FormTextDyn 
 *		07/12/97	bva		add kRpPageAccess_AllRealms 
 *		07/07/97	bva		add rpFileFormItem 
 *		06/25/97	pjr		add eRpItemType_FormFile
 *		06/19/97	bva		add eRpItemType_RepeatGroupWhile
 * * * * Release 1.6 * * * *
 *		04/18/97	pjr		add Security Digest feature
 *		03/05/97	bva		add eRpItemType_FormCheckboxDyn 
 *		02/22/97	bva		replace fDirect with fFlags in rpObjectExtension
 *		02/12/97	bva		cleanup warnings
 *		02/01/97	bva		refreshSeconds becomes Unsigned16
 *		01/27/97	rhb		moved MIME types to separate RpMimes.h
 *		01/22/97	bva		add eRpItemType_NamedDisplayText
 *		01/04/97	bva		add eRpSecurity_Disabled
 *		12/27/96	bva		replace fKeepAliveCount with fDirect in 
 *								rpObjectExtension, add eRpDataTypeFormGet
 *		12/04/96	bva		reorder rpDataType to support gMimeTypes
 * * * * Release 1.5 * * * *
 *		11/18/96	bva		add eRpItemType_ExtendedAscii definition
 *		11/01/96	bva		reorganize include files
 *		10/22/96	bva		add rpObjectType definitions,
 *							add delayed function definitions.
 *		10/20/96	bva		add rpFetchTextComplexPtr for text area
 *		10/17/96	rhb		add fResetOptionsPtr to rpVariableSelectFormItem
 *		10/16/96	bva		reorder eRpItemType_IndexDisplay_0 - eRpItemType_IndexDisplay_5
 *		10/14/96	bva		add fJavaScript to form item structures,
 *							add eRpDataTypeText, eRpDataTypeOther
 *		10/14/96	rhb		add phrase dictionary 
 *		09/24/96	rhb		support dynamically allocated engine data 
 * * * * Release 1.4 * * * *
 *		08/16/96	bva		add eRpItemType_FormRadioGroupDyn, 
 *								fRefreshPageIndex -> fRefreshPagePtr
 *								fDefaultPageIndex -> fDefaultPagePtr
 *								fPageIndex -> fPagePtr
 * * * * Release 1.3 * * * *
 *		07/30/96	bva		remove BooleanPtr, remove fAccess from page/form 
 *								structures, 
 *							add more documentation
 *		07/29/96	bva		add eRpTextType_ASCII_Fixed
 *		07/22/96	bva		rework object header, add index pointer to 
 *								rpProcessDataFuncPtr
 *		07/11/96	bva		add eRpItemType_RepeatGroupDynamic,
 *							eRpItemType_IndexedItem -> 
 *								eRpItemType_DynamicDisplay,
 *							eRpItemType_IndexedGroup -> 
 *								eRpItemType_RepeatGroup
 *		07/09/96	bva		add eRpItemType_IndexDisplay types, 
 *							add complex Get/Set function call types
 *		07/05/96	bva		merge page and form headers
 *		06/27/96	bva		added eRpItemType_HtmlReferer
 *		06/24/96	rhb		added eRpTextType_HexColonForm
 *		06/22/96	rhb		remove error page return from store function calls
 *		06/12/96	bva		add multiple realm support, remove 
 *								eRpItemType_FormPasswordItem
 *		06/11/96	rhb		added support for circle and polygon areas in image 
 *								maps
 * * * * Release 1.2 * * * *
 *		05/30/96	bva		fixed rpIndexedItem structure for picky compiler
 * * * * Release 1.1 * * * *
 *		04/25/96	rhb		split apart fixed and variable select types
 *		04/24/96	rhb		added typedefs for converting text items
 * * * * Release 1.0 * * * *
 *		03/15/96	bva		added eRpDataTypeApplet
 *		03/09/96	bva		added eRpItemType_IndexedItem, rpIndexedItem 
 *								structure
 *		03/09/96	bva		added eRpItemType_FormHiddenText
 *		03/09/96	bva		added fFormAccess to rpFormDescription
 *		02/16/96	bva		added eRpItemType_HtmlClose, 
 *								eRpItemType_HtmlFormClose
 *		02/13/96	bva		added eRpItemType_HtmlTitle
 *		02/03/96	bva		added image map types
 *		01/05/96	bva		added group item type
 *		10/30/95	rhb		created
 *
 *	To Do:
 */

#ifndef	_RPPAGES_
#define	_RPPAGES_

/*	
	A page or form is stored as a series of items.  Each item has a type and a 
	pointer to the appropriate structure or data.  The item types are:
	
	eRpItemType_LastItemInList  			
	
		This type ends the list, as pages and forms are variable groups of 
		items.  It has an empty pointer.

	eRpItemType_DataZeroTerminated			
	
		This is the basic type for all static ASCII text. The pointer is to a 
		null-terminated string.
											
	eRpItemType_DataLengthEncoded			
	
		This type is for length delimited characters.  The pointer is to a 
		two-byte length field followed by the characters.  Images that are 
		parsed by the PageBuilder program are stored in this format.
											
	eRpItemType_ExtendedAscii			
	
		This type is used for static ASCII text that has characters with the
		high bit set.  Some Japanese HTML pages use this technique. The pointer 
		is to a null-terminated string.
											
	eRpItemType_DisplayText					
	
		This type points to an rpTextDisplayItem structure that contains the 
		information for dynamic insertion of data into an HTML page.
											
	eRpItemType_NamedDisplayText					
	
		This type points to an rpNamedTextDisplayItem structure that contains 
		the information for dynamic insertion of data into an HTML page.  This 
		item is used the same way the previous item is used, but has a string 
		associated with the structure that is passed to a complex Get function.
											
	eRpItemType_FormHeader					
		
		This type points to the rpObjectDescription of a form associated with 
		the page. This item is used to trigger the <FORM> tag HTML generation.
											
	eRpItemType_FormCheckbox
		
		This type points to an rpCheckboxFormItem structure.  This type is used 
		both for generation of the <INPUT TYPE=CHECKBOX> tag and for processing 
		the form values.
											
	eRpItemType_FormRadioButton
		
		This type points to an rpRadioButtonFormItem structure.  This type is 
		used both for generation of the <INPUT TYPE=RADIO> tag and for 
		processing the form values.
											
	eRpItemType_FormAsciiText
		
		This type points to an rpTextFormItem structure.  This type is used both 
		for generation of the <INPUT TYPE=TEXT> tag and for processing the form 
		values.
											
	eRpItemType_FormPasswordText
		
		This type points to an rpTextFormItem structure.  This type is used both 
		for generation of the <INPUT TYPE=PASSWORD> tag and for processing the 
		form values.
											
	eRpItemType_FormHiddenText
		
		This type points to an rpTextFormItem structure.  This type is used both 
		for generation of the <INPUT TYPE=HIDDEN> tag and for processing the 
		form values.
											
	eRpItemType_FormFile
		
		This type points to an rpFileFormItem structure.  This type is used both 
		for generation of the <INPUT TYPE=FILE> tag and for processing the 
		file upload.
											
	eRpItemType_FormSubmit
		
		This type points to an rpButtonFormItem structure.  This type is used 
		both for generation of the <INPUT TYPE=SUBMIT> tag and for processing 
		the form values.
											
	eRpItemType_FormNamedSubmit					
	
		This type points to an rpCheckboxFormItem structure that contains 
		the information for generation of the <INPUT TYPE=SUBMIT> tag and for 
		processing the form values.

	eRpItemType_FormReset
		
		This type points to an rpButtonFormItem structure.  This type is used to
		generate the <INPUT TYPE=RESET> tag.  No value is sent in for forms.
											
	eRpItemType_FormFixedSingleSelect
		
		This type points to an rpFixedSingleSelectFormItem structure.  This type 
		is used to generate the <SELECT SIZE=nn> tag and to process the form 
		values.
											
	eRpItemType_FormFixedMultiSelect
		
		This type points to an rpFixedMultiSelectFormItem structure.  This type 
		is used both for generation of the <SELECT SIZE=nn MULTIPLE> tag and 
		for processing the form values.
											
	eRpItemType_FormVariableSingleSelect
		
		This type points to an rpVariableSelectFormItem structure.  This type is 
		used to generate the <SELECT> tag and to process the form values.
											
	eRpItemType_FormVariableMultiSelect
		
		This type points to an rpVariableSelectFormItem structure.  This type is 
		used to generate the <SELECT MULTIPLE> tag and to process the form 
		values.
											
	eRpItemType_FormTextArea
		
		This type points to an rpTextAreaFormItem structure.  This type is used 
		both for generation of the <TEXTAREA> tag and for processing of any 
		input values. The get function passes a line at a time.
											
	eRpItemType_FormTextAreaBuf
		
		This type points to an rpTextAreaFormItem structure.  This type is used 
		both for generation of the <TEXTAREA> tag and for processing of any 
		input values. The get and set functions each pass a pointer to the 
		complete buffer.
											
	eRpItemType_FormImageMap
		
		This type points to an rpImageMapFormItem structure.  This type occurs 
		only in forms and is used to process server-side image maps.
											
	eRpItemType_ImageSource
		
		This type is used in pages to the lead in HTML for an image. This type 
		points to the rpObjectDescription of the image which contains the URL
		to generate HTML like this:
			 <IMG SRC="ObjectURL"
		In general this tag is not necessary since the phrase dictionaries
		provide adequate compression.  However, if a page with embedded images
		is to sent with RomMailer, this tag is required.

	eRpItemType_HtmlReferer
		
		This type is used in pages to generate a dynamic link to the page that 
		the request came from.  It is used in RomPager error pages, and may be 
		used in other pages, also.  The pointer is to a null-terminated string 
		that is embedded in the <HREF> tag like this:
			 <A HREF="ReferringURL">string</A>

	eRpItemType_HtmlTitle
		
		This type was used in pages before version 1.5 to generate commonly 
		used HTML.  The phrase dictionary approach yields greater compression.
		The pointer is to a null-terminated string that is embedded in the 
		opening tags like this:
			<HTML>\n<HEAD>\n<TITLE>string</TITLE>\n</HEAD>\n<BODY>

	eRpItemType_HtmlClose
		
		This type was used in pages before version 1.5 to generate commonly 
		used HTML.  The phrase dictionary approach yields greater compression.
		The pointer is to a null-terminated string that is ended with the 
		closing tags like this:
			string\n</BODY>\n</HTML>\n

	eRpItemType_HtmlFormClose
		
		This type was used in pages before version 1.5 to generate commonly 
		used HTML.  The phrase dictionary approach yields greater compression.
		The pointer is to a null-terminated string that is ended with the 
		closing tags like this:
			string\n</FORM>\n</BODY>\n</HTML>\n

	eRpItemType_DynamicDisplay

		This type points to an rpDynamicDisplayItem structure.  This structure 
		points to a value and a group of items (rpItem).  The value uses a 
		rpVariableType to determine how it is retrieved, and the retrieved 
		value is used as an index into the item group to determine which item 
		to display.  This allows different HTML, static or dynamic data to be 
		displayed depending on the state of the variable.  This is useful for 
		status messages and dynamic table creation.

	eRpItemType_ItemGroup

		This type points to a group of items (rpItem).  For common groups of 
		items that are used in more than one place, this is a way of saving 
		items in the top level item list.  Since this sets up a set of nested 
		pointers, the porter needs to be aware of two things.  The number of 
		levels that can be nested is 3 greater than the value of the constant 
		kRpIndexQueryDepth (defined in RpConfig.h).  No checking for recursion 
		is done, so it is possible to set up items that point to themselves 
		thus generating an infinite amount of HTML and browser user dismay.

	eRpItemType_RepeatGroup

		This item is similar to the previous item in that a group of items will 
		be displayed.  The items in the eRpItemType_ItemGroup type are 
		displayed only once.  This item type points to a rpRepeatGroupItem 
		structure that contains Start, Limit, and Increment values as well as a 
		pointer to the item group. The values act to define a loop (for Start 
		thru Limit vary by Increment) that determine how many times to repeat 
		the item group.  This item type is very useful for table generation.  
		An example of the use of this item is shown in the IndexPortSelect page 
		to set up a row/column display of ports and slots.  Static items such 
		as the <TR>,<TH>, or <TD> tags may be repeated easily.  Dynamic items 
		(such as eRpItemType_DisplayText) that use functions of the 
		eRpVarType_Complex can easily access variables based on the current 
		value of the repeat group index.  Repeat groups may be nested to the 
		depth of kRpIndexQueryDepth, to create row/column (i,j) displays or 
		more complex retrievals (i,j,k,l).
		
	eRpItemType_RepeatGroupDynamic

		This item is like the eRpItemType_RepeatGroup item except that it points
		to the rpRepeatGroupDynItem structure which contains a pointer to a function 
		that will return the initial values of Start, Limit, and Increment.  This 
		allows the porter to easily create dynamic lists such as print jobs in 
		a print queue.  An example of use of this item is shown in the 
		PrinterStatus page.

	eRpItemType_RepeatGroupWhile

		This item is like the eRpItemType_RepeatGroup item except that it points 
		to the rpRepeatGroupDynItem structure which contains a pointer to a function 
		that is passed and returns a (void) pointer.  The initial call to the function
		passes a zero as the value of the pointer.  As long as the return value of 
		the function is a non-zero pointer, the items in the repeat group will be
		displayed.  The value returned from each RepeatGroupWhile function call is 
		passed to the next iteration of the call.  It may also may be retrieved with
		the callback routine RpGetRepeatWhileValue.  The eRpItemType_RepeatGroupWhile
		item can be used to implement GetNext types of functions.  An example of use 
		of this item is shown in the Dynamic Text Form Validation page.

	eRpItemType_FormRadioGroupDyn

		This item is used within repeat groups to set up a group of radio 
		buttons whose value will match that of the index value.	The pointer is 
		to a rpRadioGroupInfo structure.  This type is useful for setting up 
		selection buttons for elements in a list.
		
	eRpItemType_FormCheckboxDyn

		This item is used within repeat groups to set up a group of check 
		boxes that are processed by a common routine and qualified by the index 
		value.  The pointer is to an rpDynCheckboxFormItem structure.  The 
		fGetPtrType and fSetPtrType should both be of type eRpVarType_Complex 
		when used with this item.  This type is useful for setting up selection 
		boxes for elements in a list.
		
	eRpItemType_FormRadioButtonDyn

		This item is used within repeat groups to set up groups of radio buttons
		that are processed by a common routine and qualified by the index value.
		The pointer is to an rpRadioButtonFormItem structure.  The fGetPtrType and 
		fSetPtrType should both be of type eRpVarType_Complex when used with this 
		item.  This type is useful for setting up groups of a small number of 
		choices in a list or table.
		
	eRpItemType_FormTextDyn

		This item is used within repeat groups to set up a group of text 
		input fields that are processed by a common routine and qualified
		by the index value.  The pointer is to an rpTextFormItem structure.  The
		fGetPtrType and fSetPtrType should both be of type eRpVarType_Complex when
		used with this item.  This item type is useful for setting up input fields 
		as elements in a list or table.
		
	eRpItemType_FormFixedSingleDyn

		This item is used within repeat groups to set up a group of pop-up menu list 
		input fields that are processed by a common routine and qualified
		by the index value.  The pointer is to an rpFixedSingleSelectFormItem 
		structure.  The fGetPtrType and fSetPtrType should both be of type
		eRpVarType_Complex when used with this item.  This item type is useful for  
		setting up pop-up menu fields as elements in a list or table when a just a 
		single member can be chosen.
		
	eRpItemType_FormFixedMultiDyn

		This item is used within repeat groups to set up a group of pop-up menu list 
		input fields that are processed by a common routine and qualified
		by the index value.  The pointer is to an rpFixedMultiDynSelectFormItem 
		structure.  The fGetPtrType and fSetPtrType should both be of type 
		eRpVarType_Complex when used with this item.  This item type is useful for 
		setting up pop-up menu fields as elements in a list or table when multiple 
		members can be chosen.
		
	eRpItemType_FormVariableSingleDyn

		This item is used within repeat groups to set up a group of pop-up menu list 
		input fields that are processed by a common routine and qualified
		by the index value.  The pointer is to an rpVariableSelectFormItem 
		structure.  The fGetPtrType and fSetPtrType should both be of type 
		eRpVarType_Complex when used with this item.  This item type is useful for 
		setting up pop-up menu fields as elements in a list or table when a just a 
		single member can be chosen and the members are to be specified at runtime.
		
	eRpItemType_FormVariableMultiDyn

		This item is used within repeat groups to set up a group of pop-up menu list 
		input fields that are processed by a common routine and qualified
		by the index value.  The pointer is to an rpVariableSelectFormItem 
		structure.  The fGetPtrType and fSetPtrType should both be of type 
		eRpVarType_Complex when used with this item.  This item type is useful for 
		setting up pop-up menu fields as elements in a list or table when multiple 
		members can be chosen and the members are to be specified at runtime.
				
	eRpItemType_UrlState

		These items are used to create URLs that include the URL state.  The
		pointer is to a null-terminated string that will be appended with the 
		state lead-in sequence and the current URL state. 

	eRpItemType_IndexDisplay_0
	eRpItemType_IndexDisplay_1
	eRpItemType_IndexDisplay_2
	eRpItemType_IndexDisplay_3
	eRpItemType_IndexDisplay_4
	eRpItemType_IndexDisplay_5

		These items are used within repeat groups to display the current value 
		of various indices.  The pointer is to a null-terminated string that 
		will be appended with the index value.  The defined item types will 
		return the current index value, and the current value of previous index 
		levels. The last digit in the eRpItemType_IndexDisplay item defines the 
		depth of the previous index level.  If the current state of repeat 
		groups is 3 levels (i,j,k) for instance, then the 
		eRpItemType_IndexDisplay_0 item will display the k value, the 
		eRpItemType_IndexDisplay_1 item will display the j value, and the
		eRpItemType_IndexDisplay_2 will display the i value.

	eRpItemType_QueryDisplay
	
		This item points to a null-terminated string that will be appended with 
		the current state of the index values in the form of an HTML query.  
		For an index level of 3, for instance, the generated string would be 
		"string?i,j,k"  where i, j, and k are the current values of the index.  
		The RomPager engine uses the HTML query format to pass index levels 
		from one page to another. The sample pages show the use of this 
		technique in the IndexPortSelect, PortInfo, and PortConfig pages.  This 
		item type is used on the IndexPortSelect page to generate a series of 
		<HREF> tags that point to the PortInfo page.  Since this item is within 
		two repeat groups on the IndexPortSelect page, the query appended to 
		the PortInfo tag is of the form "?i,j".  When the request for the 
		PortInfo page is made by the browser user, the query is stripped off by 
		the RomPager engine and turned back into index values and levels.  
		Thus, the PortInfo page starts its display with an index level of 2.  
		When the eRpItemType_QueryDisplay item is used on this page to set up 
		an <HREF> to the PortConfig page the query is also of the form "?i,j".  
		The sample pages show an easy way to set up parent and sibling 
		relationships between pages.  If a repeat group was used on either the 
		PortInfo page or the PortConfig page, the group would start at index 
		level 3, and a eRpItemType_QueryDisplay item would generate a tag of 
		the form "?i,j,k".
		
		
	Note:
	
		Index values are defined in the eRpItemType_RepeatGroup and 
		eRpItemType_RepeatGroupDynamic items as 1-relative.  When they are 
		displayed to the user and/or passed to other pages with the 
		eRpItemType_IndexDisplay_n or eRpItemType_QueryDisplay items, they are 
		also 1-relative.  Internally, inside the engine, they are stored as 
		0-relative and when passed to eRpVarType_Complex functions they are 
		passed as 0-relative.  The general rule is when the value will be 
		visible to an end user it is 1-relative, and when it is being passed to 
		a function for internal use in indexed storage access, it will be
		0-relative.

*/



/*
	Item Type Definitions
*/
typedef enum {
	eRpItemType_LastItemInList,
	eRpItemType_DataZeroTerminated,
	eRpItemType_DataLengthEncoded,
	eRpItemType_ExtendedAscii,
	eRpItemType_DisplayText,
	eRpItemType_NamedDisplayText,
#if RomPagerForms
	eRpItemType_FormHeader,
	eRpItemType_FormAsciiText,
	eRpItemType_FormPasswordText,
	eRpItemType_FormHiddenText,
	eRpItemType_FormSubmit,
	eRpItemType_FormReset,
#if RpHtmlCheckbox
	eRpItemType_FormCheckbox,
#endif
#if RpHtmlNamedSubmit
	eRpItemType_FormNamedSubmit,					
#endif
#if RpHtmlRadio
	eRpItemType_FormRadioButton,
#endif
#if RpHtmlSelectFixedSingle
	eRpItemType_FormFixedSingleSelect,
#endif
#if RpHtmlSelectFixedMulti
	eRpItemType_FormFixedMultiSelect,
#endif
#if RpHtmlSelectVariable
	eRpItemType_FormVariableSingleSelect,
	eRpItemType_FormVariableMultiSelect,
#endif
#if RpHtmlTextArea
	eRpItemType_FormTextArea,
#endif
#if RpHtmlTextAreaBuf
	eRpItemType_FormTextAreaBuf,
#endif
#if RomPagerImageMapping
	eRpItemType_FormImageMap,
#endif
#if RpHtmlRadioDynamic
	eRpItemType_FormRadioGroupDyn,
#endif
#if RpHtmlCheckboxDynamic
	eRpItemType_FormCheckboxDyn,
#endif
#if RpHtmlTextFormDynamic
	eRpItemType_FormTextDyn,
#endif
#if RpHtmlFixedSingleDynamic
	eRpItemType_FormFixedSingleDyn,
#endif
#if RpHtmlFixedMultiDynamic
	eRpItemType_FormFixedMultiDyn,
#endif
#if RpHtmlVariableDynamic
	eRpItemType_FormVariableSingleDyn,
	eRpItemType_FormVariableMultiDyn,
#endif
#if RpHtmlRadioButtonDynamic
	eRpItemType_FormRadioButtonDyn,
#endif
#if RomPagerFileUpload
	eRpItemType_FormFile,
#endif	/* RomPagerFileUpload */
#endif	/* RomPagerForms */
#if RomPagerPreOneFive
	eRpItemType_HtmlTitle,
	eRpItemType_HtmlClose,
	eRpItemType_HtmlFormClose,
#endif
	eRpItemType_ImageSource,
	eRpItemType_HtmlReferer,
	eRpItemType_DynamicDisplay,
#if RomPagerUrlState
	eRpItemType_UrlState,
#endif
#if RomPagerQueryIndex
	eRpItemType_RepeatGroup,
	eRpItemType_RepeatGroupDynamic,
	eRpItemType_RepeatGroupWhile,
	eRpItemType_IndexDisplay_0,		/*	These items must be in sequential order	*/
	eRpItemType_IndexDisplay_1,
	eRpItemType_IndexDisplay_2,
	eRpItemType_IndexDisplay_3,
	eRpItemType_IndexDisplay_4,
	eRpItemType_IndexDisplay_5,
	eRpItemType_QueryDisplay,
#endif
	eRpItemType_ItemGroup
} rpItemType;

typedef enum {
	eRpVarType_Direct,
	eRpVarType_Function,
	eRpVarType_Complex
} rpVariableType;

typedef enum {
	eRpTextType_ASCII,				/* ASCII null-terminated string           */
	eRpTextType_ASCII_Fixed,		/* ASCII chars with length specified      */
	eRpTextType_Hex,
	eRpTextType_HexColonForm,		/* Hex display with colons between chars  */
	eRpTextType_DotForm,			/* Dotted decimal - nnn.nnn.nnn.nnn       */
	eRpTextType_Signed8,
	eRpTextType_Signed16,
	eRpTextType_Signed32,
#if RomPagerUse64BitIntegers
	eRpTextType_Signed64,
	eRpTextType_Unsigned64,
#endif
	eRpTextType_Unsigned8,
	eRpTextType_Unsigned16,
	eRpTextType_Unsigned32
} rpTextType;


/*

	The PageBuilder parser looks for HTML Form elements and converts them into 
	internal data structures.  Each form element (ie. radio button, check box, 
	text element, etc.) has a different structure.  These structures are used 
	both for page creation (using Get routines to access the variables) and 
	form processing (using Set routines to access the variables).  
	
	The form element structures all contain Set/Get elements as appropriate.  
	These Set/Get elements are:
	
		void *			fGetPtr;			
		void *			fSetPtr;			
		rpVariableType	fGetPtrType;	
		rpVariableType	fSetPtrType;	
	
	
	The rpVariableType field is used to indicate whether the Set/Get pointers 
	point directly to a variable address (eRpVarType_Direct), point to a simple 
	function to store/retrieve the variable (eRpVarType_Function), or point to 
	a complex function (eRpVarType_Complex).  Complex functions are passed the 
	HTML name of the variable, and the current state of the repeat group 
	indices, as well as a pointer to the variable to be stored/retrieved.
	
	The data typing of the Set/Get value can be implicit or explicit.  For 
	checkbox form elements, for example, the data type of the value is 
	implicitly Boolean, so if an address of the value is specified in fGetPtr 
	or fSetPtr, the RomPager routines will assume they are setting or getting a 
	Boolean.  If a function call is specified in fGetPtr, then the function is 
	assumed to have the form:
	
		typedef Boolean	(*rpFetchBooleanFuncPtr)(void);
	
	If a function call is specified in fSetPtr, the function is assumed to have 
	the form:
	
		typedef void	(*rpStoreBooleanFuncPtr)(Boolean theValue);
	
	For HTML elements that use text for display or entry, the form element 
	structure contains an additional element that specifies an explicit data 
	type so that RomPager can perform conversions if necessary.
	
		rpTextType	fTextType;		
		
	The types and their explanations are defined above.
*/									


/*
	The text display item structure.
*/
typedef struct {
	void *			fGetPtr;		/*	Pointer to either the memory location 
										that the Get value is stored in, or a 
										function to retrieve the Get value.  */
	rpVariableType	fGetPtrType;	
	rpTextType		fTextType;		/*	Used to indicate what type of 
										conversion is necessary to turn the Get 
										variable into display text  */
	Unsigned8		fFieldSize;		/*	Length of text on the page for the 
										variable.  Used by hex and dot-form 
										variables.  */
} rpTextDisplayItem, *rpTextDisplayItemPtr;


/*
	The named text display item structure.
*/
typedef struct {
	char *			fNamePtr;		/*	The name of the display text item     */
	void *			fGetPtr;		/*	Pointer to either the memory location 
										that the Get value is stored in, or a 
										function to retrieve the Get value.  */
	rpVariableType	fGetPtrType;	
	rpTextType		fTextType;		/*	Used to indicate what type of 
										conversion is necessary to turn the Get 
										variable into display text  */
	Unsigned8		fFieldSize;		/*	Length of text on the page for the 
										variable.  Used by hex and dot-form 
										variables.  */
} rpNamedTextDisplayItem, *rpNamedTextDisplayItemPtr;



/*	
	Checkbox form item structure.
*/
typedef struct {
	char *			fNamePtr;			/*	The HTML name of the checkbox 
											item                        		*/
	void *			fGetPtr;			/*	Pointer to either the memory 
											location that the Get value is 
											stored in, or a function to 
											retrieve the Get value.      		*/
	void *			fSetPtr;			/*	Pointer to either the memory 
											location that the Set value is 
											stored in, or a function to 
											store the Set value.           		*/
	rpVariableType	fGetPtrType;	
	rpVariableType	fSetPtrType;	
#if RomPagerJavaScript
	char *			fJavaScriptPtr;		/*	The pointer to the Java Script		*/ 
#endif
} rpCheckboxFormItem, *rpCheckboxFormItemPtr;


/*	
	Dynamic Checkbox form item structure.
*/
typedef struct {
	char *			fNamePtr;			/*	The HTML name of the checkbox 
											item                           		*/
	void *			fGetPtr;			/*	Pointer to either the memory 
											location that the Get value is 
											stored in, or a function to 
											retrieve the Get value.       		*/
	void *			fSetPtr;			/*	Pointer to either the memory 
											location that the Set value is 
											stored in, or a function to 
											store the Set value.      			*/
	rpVariableType	fGetPtrType;	
	rpVariableType	fSetPtrType;	
	void *			fResetPtr;			/*	Pointer to routine to reset
											the checkbox array					*/
#if RomPagerJavaScript
	char *			fJavaScriptPtr;		/*	The pointer to the Java Script		*/ 
#endif
} rpDynCheckboxFormItem, *rpDynCheckboxFormItemPtr;

/*	
	Radio buttons require a group structure with the hooks to the Get/Set 
	routines and individual structures for each button that refer back to the 
	group structure.  The HTML value that is displayed and selected corresponds 
	to an actual value that is Get/Set in the parameter values.
*/

typedef struct {
	char *			fNamePtr;			/*	The HTML name of the radio box 
											item                        	*/
	void *			fGetPtr;			/*	Pointer to either the memory 
											location that the Get value is 
											stored in, or a function to 
											retrieve the Get value.    		*/
	void *			fSetPtr;			/*	Pointer to either the memory 
											location that the Set value is 
											stored in, or a function to 
											store the Set value.      		*/
	rpVariableType	fGetPtrType;	
	rpVariableType	fSetPtrType;	
} rpRadioGroupInfo, *rpRadioGroupInfoPtr;

typedef struct {
	rpRadioGroupInfoPtr	fRadioGroupPtr;	/*	Pointer to radio button group 
											structure                         */
	char *				fValue;			/*	The HTML Value string of the radio 
											button                            */
	rpOneOfSeveral		fButtonNumber;	/*	The actual value of the radio 
											button                            */
#if RomPagerJavaScript
	char *			fJavaScriptPtr;		/*	The pointer to the Java Script		*/ 
#endif
} rpRadioButtonFormItem, *rpRadioButtonFormItemPtr;


/*
	The text form item is used to display and process text fields.  The same 
	structure is used for plain text and for HTML password test fields.
*/
typedef struct {
	char *			fNamePtr;			/*	The HTML name of the text form 
											item                              	*/
	void *			fGetPtr;			/*	Pointer to either the memory 
											location that the Get value is 
											stored in, or a function to 
											retrieve the Get value.           	*/
	void *			fSetPtr;			/*	Pointer to either the memory 
											location that the Set value is 
											stored in, or a function to 
											store the Set value.           		*/
	rpVariableType	fGetPtrType;	
	rpVariableType	fSetPtrType;	
	rpTextType		fTextType;			/*	Used to indicate what type of 
											conversion is necessary to turn 
											variable to and from text         	*/
	Unsigned8		fFieldSize;			/*	Length of text on the page for the 
											variable.  Used by conversions for 
											ASCII-Fixed, hex, and dot-form 
											variables and equivalent to HTML. 	*/
	Unsigned8		fFieldMaxLength;	/*	Equivalent to HTML maxsize.  Used 
											to control browser field entry.   	*/
#if RomPagerJavaScript
	char *			fJavaScriptPtr;		/*	The pointer to the Java Script		*/ 
#endif
} rpTextFormItem, *rpTextFormItemPtr;


/*
	The button structure is used define the HTML for Submit and Reset buttons.  
	Reset buttons do not return any information to a form, but only trigger 
	local browser actions.  Submit buttons are returned to the form with an 
	argument format of "Submit=xxxxx"  where xxxxx is the contents of 
	fLabelText.  This value is stored in the global gRpSubmitButtonValue during 
	forms processing.  The global can be examined by optional forms 
	post-processing routines to determine appropriate actions if multiple 
	Submit buttons are present in a single form.  The post processing routine 
	is triggered by the presence of a rpProcessDataFuncPtr in the form's 
	rpObjectExtension structure.
*/
typedef struct {
	char *			fLabelText;			/*	The HTML value of the button item  	*/
#if RomPagerJavaScript
	char *			fJavaScriptPtr;		/*	The pointer to the Java Script		*/ 
#endif
} rpButtonFormItem, *rpButtonFormItemPtr;

#if 0
/*
	This button structure is used define the HTML for Submit buttons that have both
	an HTML name and a value.  The value which will be displayed in the button is
	retrieved using the fGetPtr.  When these Submit buttons are returned from the 
	form they have an argument format of "nnnn=xxxxx"  where nnnn is the contents of 
	fNamePtr, and xxxxx is the value.  The fSetPtr points to a the 
*/
typedef struct {
	char *				fNamePtr;			/*	The HTML name of the button item	*/
	void *				fGetPtr;			/*	Pointer to either the 
												memory location that the 
												Get value is stored in, or 
												a function to retrieve the 
												Get value.                			*/
	void *				fSetPtr;			/*	Pointer to either the 
												memory location that the 
												Set value is stored in, or 
												a function to store the 
												Set value.                			*/
	rpVariableType		fGetPtrType;	
	rpVariableType		fSetPtrType;	
#if RomPagerJavaScript
	char *				fJavaScriptPtr;		/*	The pointer to the Java Script		*/ 
#endif
} rpNamedSubmitItem, *rpNamedSubmitItemPtr;
#endif

/*
	The rpFixedSingleSelectFormItem item is used to display and process pop-up 
	menu text lists where a single element can be chosen from a fixed list of 
	elements. The list behaves like a set of radio buttons, where each element 
	in the list corresponds to a value to be set. When an element is chosen 
	from the list, its fOptionNumber is the value Set. 
*/
typedef struct rpOption_OneSelect {
	struct rpOption_OneSelect *	fNextPtr;		/*	Pointer to the next item in 
													a fixed list              */
	char *						fValuePtr;		/*	HTML value to display     */
	rpOneOfSeveral				fOptionNumber;	/*	Item value to Set if 
													selected                  */
} rpOption_OneSelect, *rpOption_OneSelectPtr;

typedef struct {	
	char *					fNamePtr;			/*	The HTML name of the list 
													item                      */	
	rpOption_OneSelectPtr	fFirstOptionPtr;	/* 	Pointer to the first 
													element in the list       */
	void *					fGetPtr;			/*	Pointer to either the 
													memory location that the 
													Get value is stored in, or 
													a function to retrieve the 
													Get value.                */
	void *					fSetPtr;			/*	Pointer to either the 
													memory location that the 
													Set value is stored in, or 
													a function to store the 
													Set value.                */
	rpVariableType			fGetPtrType;	
	rpVariableType			fSetPtrType;	
	Unsigned8				fListLength;		/*	HTML length of the list 
													(controls display)        */
#if RomPagerJavaScript
	char *			fJavaScriptPtr;				/*	The pointer to the Java Script	*/ 
#endif
} rpFixedSingleSelectFormItem, *rpFixedSingleSelectFormItemPtr;


/*
	The rpVariableSelectFormItem item is used to display and process pop-up 
	menu lists where an element can be chosen from a variable length list of 
	elements. The elements can be of any text type. When a value is chosen from 
	the list, it is converted to the specified type and stored as determined by 
	the Set pointer. 
*/

typedef struct {
	char *					fNamePtr;			/*	The HTML name of the list 
													item                      */	
	rpResetVarSelectPtr		fResetOptionsPtr;	/*	Pointer to function to 
													reset all elements for 
													Variable Multiple Select  */
	void *					fGetPtr;			/*	Pointer to a function to 
													retrieve the Get value for 
													an element.               */
	void *					fSetPtr;			/*	Pointer to either the 
													memory location that an 
													element's Set value is 
													stored in, or a function to 
													store the Set value.      */
	rpVariableType			fGetPtrType;	
	rpVariableType			fSetPtrType;	
	rpTextType				fTextType;			/*	Used to indicate what type 
													of conversion is necessary 
													to turn an element to and  
													from text                 */
	Unsigned8				fFieldSize;			/*	Length of text on the page 
													for hex and dot-form 
													elements.                 */
	Unsigned8				fListLength;		/*	HTML length of the list 
													(controls display)        */
#if RomPagerJavaScript
	char *			fJavaScriptPtr;				/*	The pointer to the Java Script	*/ 
#endif
} rpVariableSelectFormItem, *rpVariableSelectFormItemPtr;


/*
	The rpFixedMultiSelectFormItem item is used to display and process pop-up 
	menu text lists where multiple elements can be chosen from a fixed list of 
	elements. This list behaves like a set of checkboxes, that is, each element 
	in the list has its own Set/Get structure.
*/
typedef struct rpOption_MultiSelect {
	struct rpOption_MultiSelect *	fNextPtr;	/*	Pointer to the next item in 
													a fixed list              */
	char *							fValuePtr;	/*	HTML value to display     */
	void *							fGetPtr;	/*	Pointer to either the 
													memory location that the 
													Get value is stored in, or 
													a function to retrieve the 
													Get value.                */
	void *							fSetPtr;	/*	Pointer to either the 
													memory location that the 
													Set value is stored in, or 
													a function to store the 
													Set value.               */
	rpVariableType					fGetPtrType;	
	rpVariableType					fSetPtrType;	
} rpOption_MultiSelect, *rpOption_MultiSelectPtr;

typedef struct {
	char *					fNamePtr;			/*	The HTML name of the list 
													item                      */
	rpOption_MultiSelectPtr	fFirstOptionPtr;	/* 	Pointer to the first item 
													in the list               */
	Unsigned8				fListLength;		/*	HTML length of the list 
													(controls display)        */
#if RomPagerJavaScript
	char *			fJavaScriptPtr;				/*	The pointer to the Java Script	*/ 
#endif
} rpFixedMultiSelectFormItem, *rpFixedMultiSelectFormItemPtr;


/*
	The rpFixedMultiDynSelectFormItem item is like a rpFixedMultiSelectFormItem, 
	but it is used when the list is within a repeat group.
*/
typedef struct {
	char *					fNamePtr;			/*	The HTML name of the list 
													item                      */
	rpOption_MultiSelectPtr	fFirstOptionPtr;	/* 	Pointer to the first item 
													in the list               */
	Unsigned8				fListLength;		/*	HTML length of the list 
													(controls display)        */
	void *					fResetPtr;			/*	Pointer to routine to reset
													the all option values     */
#if RomPagerJavaScript
	char *					fJavaScriptPtr;		/*	The pointer to the Java Script	*/ 
#endif
} rpFixedMultiDynSelectFormItem, *rpFixedMultiDynSelectFormItemPtr;


/*
	The text area form item is used to display and process text area fields.  
	Although the Get and Set pointers are here, use of these items only makes 
	sense for variable information displayed or gathered with a function call.
*/
typedef struct {
	char *				fNamePtr;			/*	The HTML name of the text area	*/
	void *				fGetPtr;			/*	Pointer to either the memory 
												location that the Get value is 
												stored in, or a function to 
												retrieve the Get value.       	*/
	void *				fSetPtr;			/*	Pointer to either the memory 
												location that the Set value is 
												stored in, or a function to 
												store the Set value.       	*/
	rpVariableType		fGetPtrType;	
	rpVariableType		fSetPtrType;	
	Unsigned8			fRows;				/*	The HTML number of rows for 
												the text area                 	*/
	Unsigned8			fColumns;			/*	The HTML number of columns for 
												the text area                 	*/
#if RomPagerJavaScript
	char *			fJavaScriptPtr;			/*	The pointer to the Java Script	*/ 
#endif
} rpTextAreaFormItem, *rpTextAreaFormItemPtr;


/*
	The file form item is used to display and process the file upload tag.  
*/
typedef struct {
	char *			fNamePtr;			/*	The HTML name of the file form 
											item                              	*/
	Unsigned8		fFieldSize;			/*	Length of filename to display */
	Unsigned8		fFieldMaxLength;	/*	Equivalent to HTML maxsize.  Used 
											to control browser field entry.   	*/
#if RomPagerJavaScript
	char *			fJavaScriptPtr;		/*	The pointer to the Java Script		*/ 
#endif
} rpFileFormItem, *rpFileFormItemPtr;

/*	
	Image mapping works in a similar fashion to radio buttons and require a 
	group structure with the hooks to the Set variable and individual 
	structures for each location in the map that can trigger an action.  The 
	location to be selected is determined by coordinates, and selects an 
	arbitrary value stored in the structure, as well as the next page to be 
	served.

	The fTop, fBottom, fLeft, fRight values in rpImageMapLocation describe the
	attributes of the location depending on its type as follows:
		Circle    - fTop and fLeft is the center with fRight being the radius;
		Rectangle - fTop, fBottom, fLeft, fRight are the bounds; and
		Polygon   - fRight is the number of points that fPolyPointPtr points to
*/

typedef enum {
	eRpLocationType_Rectangle,
	eRpLocationType_Circle,
	eRpLocationType_Polygon
} rpLocationType;

typedef struct {
	Signed16				fX;
	Signed16				fY;
} rpPoint, *rpPointPtr;

typedef struct rpImageMapLocation {
	struct rpImageMapLocation *	fNextPtr;		/*	Pointer to the next 
													location item             */
	struct rpObjectDescription *fPagePtr;		/*	Page to serve if this 
													location is selected.     */
	rpLocationType				fType;			/*	Rectangle, Circle or 
													Polygon                   */
	rpPointPtr					fPolyPointPtr;	/*	Pointer to a polygon's 
													array of points           */
	Unsigned8					fTop;
	Unsigned8					fBottom;
	Unsigned8					fLeft; 
	Unsigned8					fRight;
	rpMapLocation				fLocation;		/*	The value to be Set if this 
													location selected         */
} rpImageMapLocation, *rpImageMapLocationPtr;

typedef struct {
	struct rpObjectDescription	*fDefaultPagePtr;	/*	Page to serve if image 
														map is clicked on, but 
														no specific location 
														item matches.  Value 
														will be set to 0 if no 
														location item matches	*/		
	rpImageMapLocationPtr		fFirstLocationPtr;	/*	Pointer to the first 
														location item      		*/
	void *						fSetPtr;			/*	Pointer to either the 
														memory location that 
														the Set value is stored 
														in, or a function to
														store the Set value.	*/
	rpVariableType				fSetPtrType;	
} rpImageMapFormItem, *rpImageMapFormItemPtr;


/*
	The page/form item structure.
*/
typedef struct {
	rpItemType				fType;
	void *					fAddress;
} rpItem, *rpItemPtr;


/*
	The dynamic display item structure.
*/
typedef struct {
	void *					fGetPtr;
	rpVariableType			fGetPtrType;	
	Unsigned8				fItemCount;
	rpItem					*fItemsArrayPtr;		/*	Pointer to array of 
														object items          */
} rpDynamicDisplayItem, *rpDynamicDisplayItemPtr;


/*
	The repeat group item structure.
*/
typedef struct {
	Signed16				fIndexStart;
	Signed16				fIndexLimit;
	Signed16				fIndexIncrement;
	rpItem					*fItemsArrayPtr;		/*	Pointer to array of 
														object items          */
} rpRepeatGroupItem, *rpRepeatGroupItemPtr;


/*
	The dynamic repeat group item structure.
	(also used for repeat group while)
*/

typedef struct {
	void *					fFunctionPtr;
	rpItem					*fItemsArrayPtr;		/*	Pointer to array of 
														object items          */
} rpRepeatGroupDynItem, *rpRepeatGroupDynItemPtr;



/* 
	Object Type Definitions for cache control
*/
typedef enum {
	eRpObjectTypeStatic,
	eRpObjectTypeDynamic,
	eRpObjectTypeFile,
	eRpObjectTypeRemote
} rpObjectType;


/* 
	Password Access Definitions 
*/
#define _SupportOutOfRealm8_

typedef Unsigned32 rpAccess;

#define	kRpPageAccess_Unprotected	(0x00000000)
#define	kRpPageAccess_Realm1		(0x00000001)
#define	kRpPageAccess_Realm2		(kRpPageAccess_Realm1 << 1)
#define	kRpPageAccess_Realm3		(kRpPageAccess_Realm2 << 1)
#define	kRpPageAccess_Realm4		(kRpPageAccess_Realm3 << 1)
#define	kRpPageAccess_Realm5		(kRpPageAccess_Realm4 << 1)
#ifndef _SupportOutOfRealm8_
#define	kRpPageAccess_Realm6		(kRpPageAccess_Realm5 << 1)
#define	kRpPageAccess_Realm7		(kRpPageAccess_Realm6 << 1)
#define	kRpPageAccess_Realm8		(kRpPageAccess_Realm7 << 1)
#else
#define	kRpPageAccess_ReadWrite		(0x00FF0000)
#define	kRpPageAccess_ReadPlus		(0x0000FF00)
#define	kRpPageAccess_ReadOnly		(0x000000FF)
#define	kRpPageAccess_Realm6		(kRpPageAccess_ReadPlus | kRpPageAccess_ReadWrite)
#define	kRpPageAccess_Realm7		(kRpPageAccess_ReadOnly |  kRpPageAccess_ReadPlus | kRpPageAccess_ReadWrite)
#define	kRpPageAccess_Realm8		(kRpPageAccess_ReadWrite)
#endif

#define	kRpPageAccess_AllRealms		(0xFFFFFFFF)

/* 
	Object Flag Definitions 
*/
typedef Unsigned8 rpObjectFlags;

#define	kRpObjFlags_None			(0x0000)
		/* 	
			set to force the direct serve of the next object after forms handling
		*/
#define	kRpObjFlag_Direct			(0x0001)
		/* 	
			set to trigger "server push" handling for this object.
		*/
#define	kRpObjFlag_ServerPush		(kRpObjFlag_Direct << 1)	
#define	kRpObjFlag_DebugFlow		(kRpObjFlag_ServerPush << 1)
#define	kRpObjFlag_4				(kRpObjFlag_DebugFlow << 1)
#define	kRpObjFlag_5				(kRpObjFlag_4 << 1)
#define	kRpObjFlag_6				(kRpObjFlag_5 << 1)
#define	kRpObjFlag_7				(kRpObjFlag_6 << 1)
#define	kRpObjFlag_Aggregate		(kRpObjFlag_7 << 1)	

/*
	Security Level states
*/

typedef enum {
#if RomPagerSecurityDigest
	eRpSecurity_StrictDigestAndIpAddress,	/*	digest password with a new nonce
												for each request and IP Address	*/
	eRpSecurity_DigestAndIpAddress,			/*	digest password with a new nonce
												per session and IP Address		*/
	eRpSecurity_DigestPasswordOnly,			/*	digest password with a new nonce
												per session						*/
#endif	/* RomPagerSecurityDigest */
	eRpSecurity_PasswordAndIpAddress,
	eRpSecurity_PasswordOnly,
	eRpSecurity_Disabled
} rpSecurityLevel;



/*
	The Object Extension structure definition
*/

typedef struct rpObjectExtension {
	rpProcessDataFuncPtr		fProcessDataFuncPtr;	/*	Pointer to optional 
															routine called 
															after form items 
															are processed or 
															before page is 
															served            	*/
	struct rpObjectDescription	*fPagePtr;				/*	Pointer to results 
															page that will be served 
															after the form is
															processed         	*/
	struct rpObjectDescription	*fRefreshPagePtr;		/*	Pointer to page to 
															serve for 
															client-pull 
															refresh           	*/
	Unsigned16					fRefreshSeconds;		/*	optional time to 
															trigger client-pull 
															page refreshing   	*/
	rpObjectFlags				fFlags;					/*	Flags defined above.	*/
#if RomPagerJavaScript
	char *						fJavaScriptPtr;			/*	The pointer to the 
															Java Script			*/ 
#endif
} rpObjectExtension, *rpObjectExtensionPtr;


/*
	The Object structure definition
*/
typedef struct rpObjectDescription {
	char *					fURL;					/*	Path used to find 
														object                */
	rpItem					*fItemsArrayPtr;		/*	Pointer to array of 
														object items          */
	rpObjectExtensionPtr	fExtensionPtr;			/*	Pointer to structure of 
														optional info         */
	Unsigned32				fLength;				/*	Length of data in 
														object (used for images 
														and applets)          */
	rpAccess				fObjectAccess;			/*	Indicates type of 
														access control for 
														page                  */
	rpDataType				fMimeDataType;			/*	Indicates page type 
														(used for MIME type 
														checking)             */
	rpObjectType			fCacheObjectType;		/*	Static, Dynamic and File
														types will trigger
														different HTTP 
														headers to control 
														browser caching       */
} rpObjectDescription, *rpObjectDescriptionPtr;


/*
	miscellaneous definitions used in page/form definitions
*/

#define	kRootPage			0

extern rpObjectDescriptionPtr	gRpObjectList[];

/*
	error definitions used by conversion routines
*/


typedef enum {
	eRpItemError_UseErrorPagePtr						= -1,
	eRpItemError_NoError								=  0,
	eRpItemError_ItemNotFound,
	eRpItemError_SingleSelectionOptionNotFound,
	eRpItemError_MultiSelectionOptionNotFound,
	eRpItemError_IllegalHexCharacter,
	eRpItemError_OddNumberOfHexCharacters,
	eRpItemError_ExpectingAColonInHexColonForm,
	eRpItemError_TooManyCharacters,
	eRpItemError_Signed8TooSmall,
	eRpItemError_Signed8TooLarge,
	eRpItemError_Signed16TooSmall,
	eRpItemError_Signed16TooLarge,
	eRpItemError_Signed32TooSmall,
	eRpItemError_Signed32TooLarge,
	eRpItemError_Signed64TooSmall,
	eRpItemError_Signed64TooLarge,
	eRpItemError_Unsigned8TooLarge,
	eRpItemError_Unsigned16TooLarge,
	eRpItemError_Unsigned32TooLarge,
	eRpItemError_Unsigned64TooLarge,
	eRpItemError_ExpectingADecimalDigit,
	eRpItemError_ExpectingAnInteger,
	eRpItemError_UserError					/* must be last	*/
} rpItemError;



#endif

