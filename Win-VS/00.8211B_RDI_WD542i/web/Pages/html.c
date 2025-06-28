/* Created with PageBuilder version 4.2 on Tue Jul 19 18:32:06 2011 */

#include "AsExtern.h"

#if RomPagerServer



/* ************************************** */
/* *    Built from "html\start.html"    * */
/* ************************************** */

extern rpObjectDescription Pgstart;

static char Pgstart_Item_1[] =
	C_oHTML "<script>parent.Timer_reset=1;</script>\n"
	C_oHEAD "\n"
	C_oTITLE "\n";

extern char *sWEB_Main_TitleDisplay_Get(void);
static rpTextDisplayItem Pgstart_Item_2 = {
	(void*)sWEB_Main_TitleDisplay_Get,
	eRpVarType_Function,
	eRpTextType_ASCII,
	20
};

static char Pgstart_Item_3[] =
	C_xTITLE C_xHEAD "<script language=\"JavaScript\">\n"
	"//";

static char Pgstart_Item_4[] =
	"\n"
	"function err()\n"
	"{\n"
	"\tunauth();\n"
	"\treturn true;\n"
	"}\n"
	"function unauth()\n"
	"{\n"
	"\talert(\"Unauthorized action!\");\n"
	"\tlocation.href=\"/\";\n"
	"}\n"
	"function urgy()\n"
	"{\n"
	"\tif((self==top) && (location.href.indexOf(\"/RH0\")!=-1))\n"
	"\t{\n"
	"\t\tonerror=err;\n"
	"\t\tif(window.opener==null)\n"
	"\t\t{\n"
	"\t\t\tunauth();\n"
	"\t\t}\n"
	"\t\telse\n"
	"\t\t{\n"
	"\t\t\tif(!window.opener.closed)\n"
	"\t\t\t{\n";

static char Pgstart_Item_5[] =
	"\t\t\t  if(window.opener.parent.parent.urgy==null)\n"
	"\t\t\t  {\n"
	"\t\t\t\tunauth();\n"
	"\t\t\t  }\n"
	"\t\t\t}\n"
	"\t\t\telse\n"
	"\t\t\t{\n"
	"\t\t\t\tunauth();\n"
	"\t\t\t}\n"
	"\t\t}\n"
	"\t\tonerror=null;\n"
	"\t}\n"
	"}\n"
	"urgy();\n"
	"\n"
	"//";

static char Pgstart_Item_6[] =
	"</script>\n";

extern char *html_GetStartPageProcess(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpTextDisplayItem Pgstart_Item_7 = {
	(void*)html_GetStartPageProcess,
	eRpVarType_Complex,
	eRpTextType_ASCII,
	20
};

static char Pgstart_Item_8[] =
	C_xHTML;


static rpItem Pgstart_Items[] = { 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgstart_Item_1 }, 
	{ eRpItemType_DisplayText, (void*)&Pgstart_Item_2 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgstart_Item_3 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgstart_Item_4 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgstart_Item_5 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgstart_Item_6 }, 
	{ eRpItemType_DisplayText, (void*)&Pgstart_Item_7 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgstart_Item_8 }, 
	{ eRpItemType_LastItemInList } 
};


rpObjectDescription Pgstart = {
	"/",
	Pgstart_Items,
	(rpObjectExtensionPtr) 0,
	(Unsigned32) 0,
	kRpPageAccess_Unprotected,
	eRpDataTypeHtml,
	eRpObjectTypeDynamic
};


/* ************************************* */
/* *    Built from "html\jsComm.js"    * */
/* ************************************* */

extern rpObjectDescription PgjsComm;

static char PgjsComm_Item_1[] =
	"\n";

static char PgjsComm_Item_2[] =
	"function ipCheck(textValue)\n"
	"  {\n"
	"    \tif (textValue.indexOf(\',\') != -1)     return false;\t";

static char PgjsComm_Item_3[] =
	"    \tif (textValue.indexOf(\' \') != -1)     return false;\n"
	"    re1=/(\\d+)(\\W)(\\d+)(\\W)(\\d+)(\\W)(\\d+)/\n"
	"    Check=textValue.search(re1);\n"
	"    if(Check==-1)   return false;\n"
	"\n"
	"    else\n"
	"    {\n"
	"\n"
	"\n"
	"       var ipSplit=textValue.split(\'.\');  ";

static char PgjsComm_Item_4[] =
	"       if(ipSplit.length!=4)\n"
	"       \t\treturn false;\n"
	"\t\tfor(i=0; i<ipSplit.length; i++)\n"
	"\t\t{\n"
	"\t\t\tif(isNaN(ipSplit[i]))  return false;\n"
	"\t\t}\n"
	"       \t"
	"if(eval(ipSplit[0])+eval(ipSplit[1])+eval(ipSplit[2])+eval(ipSplit[3])"
	"==0)  ";

static char PgjsComm_Item_5[] =
	"       \t\treturn false;\n"
	"\n"
	"       \t"
	"if(eval(ipSplit[0])+eval(ipSplit[1])+eval(ipSplit[2])+eval(ipSplit[3])"
	"==1020)  ";

static char PgjsComm_Item_6[] =
	"       \t\treturn false;\n"
	"\n"
	"       for(i=0; i<ipSplit.length; i++)\n"
	"       {\n"
	"         if(ipSplit[i]>255 || ipSplit[i]<0)  return false;\n"
	"       }\n"
	"\n"
	"       return true;\n"
	"\n"
	"    }\n"
	" }\n"
	"\n";

static char PgjsComm_Item_7[] =
	"\n"
	"function ipCheck2(textValue)\n"
	"{\n"
	"\tre1=/(\\d+)(\\W)(\\d+)(\\W)(\\d+)(\\W)(\\d+)/\n"
	"\tCheck=textValue.search(re1);\n"
	"\tif(Check==-1)   return false;\n"
	"\n"
	"\telse\n"
	"\t{\n"
	"\t\n"
	"       var ipSplit=textValue.split(\'.\');  ";

static char PgjsComm_Item_8[] =
	"    \tif (textValue.indexOf(\',\') != -1)     return false;\t";

static char PgjsComm_Item_9[] =
	"    \tif (textValue.indexOf(\' \') != -1)     return false;\n"
	"\t\tif(ipSplit.length!=4) return false;\n"
	"\t\tfor(i=0; i<ipSplit.length; i++)\n"
	"\t\t{\n"
	"\t\t\tif(isNaN(ipSplit[i]))  return false;\n"
	"\t\t\tif(ipSplit[i] > 255)  return false;\n"
	"\t\t\tif(ipSplit[i] < 0 )  return false;\n"
	"\t\t}\n"
	"\n"
	"\t\tif((ipSplit[0]==255) && (ipSplit[1]==255) && (ipSplit[2]==255)\n"
	"\t\t\t&& (ipSplit[3]==255)) return false;\n"
	"\t\tif(ipSplit[0]==0) return false;\n"
	"\t\tif(ipSplit[0]> 223) return false;\n"
	"\n"
	"\t\treturn true;\n"
	"    }\n"
	" }\n"
	"\n"
	"\n"
	"\n"
	"function gatwayCheck(textValue)\n"
	"{\n"
	"\tre1=/(\\d+)(\\W)(\\d+)(\\W)(\\d+)(\\W)(\\d+)/\n"
	"\tCheck=textValue.search(re1);\n"
	"\tif(Check==-1)   return false;\n"
	"\n"
	"\telse\n"
	"\t{\n"
	"\t\t//re2=/(\\W)/\n"
	"\t\tvar ipSplit=textValue.split(\'.\');  ";

static char PgjsComm_Item_10[] =
	"    \tif (textValue.indexOf(\',\') != -1)     return false;\t";

static char PgjsComm_Item_11[] =
	"    \tif (textValue.indexOf(\' \') != -1)     return false;\n"
	"\t\tif(ipSplit.length!=4)\n"
	"\t\t\treturn false;\n"
	"\t\tfor(i=0; i<ipSplit.length; i++)\n"
	"\t\t{\n"
	"\t\t\tif(isNaN(ipSplit[i]))  return false;\n"
	"\t\t}\n"
	"\n"
	"\t\t"
	"if(eval(ipSplit[0])+eval(ipSplit[1])+eval(ipSplit[2])+eval(ipSplit[3])"
	"==1020)  ";

static char PgjsComm_Item_12[] =
	"\t\t\treturn false;\n"
	"\n"
	"\t\tfor(i=0; i<ipSplit.length; i++)\n"
	"\t\t{\n"
	"\t\t\tif(ipSplit[i]>=255 || ipSplit[i]<0)  return false;\n"
	"\t\t}\n"
	"\n"
	"\t\treturn true;\n"
	"    }\n"
	"}\n"
	"\n"
	"function hostAddrCheck(textValue)\n"
	"{\n"
	"\tvar re1=/(\\d+)(\\W)(\\d+)(\\W)(\\d+)(\\W)(\\d+)/\n"
	"\n"
	"\tvar check=textValue.search(re1);\n"
	"\n"
	"\tif(check == -1)\n"
	"\t\treturn false;\n"
	"\telse\n"
	"\t{\n"
	"\t\t//var re2=/(\\W)/\n"
	"\t\tvar ipString=textValue.split(\'.\');\n"
	"    \tif (textValue.indexOf(\',\') != -1)     return false;\t";

static char PgjsComm_Item_13[] =
	"    \tif (textValue.indexOf(\' \') != -1)     return false;\n"
	"\t\tif(ipString.length!=4)\n"
	"\t\t\treturn false;\n"
	"\t\t/* 255.255.255.255 */\n"
	"\t\t"
	"if(eval(ipString[0])+eval(ipString[1])+eval(ipString[2])+eval(ipString"
	"[3])==1020)\n"
	"\t\t\treturn false;\t\n"
	"\t\telse if(eval(ipString[0])>=256 || eval(ipString[1])>=256 || "
	"eval(ipString[2])>=256 || eval(ipString[3])>=256)\n"
	"\t\t\treturn false;\n"
	"\t\t/* 127.0.0.0 - 127.255.255.255 */\n"
	"\t\telse if(ipString[0] == 127)\n"
	"\t\t\treturn false;\n"
	"\t\telse\n"
	"\t\t{\n"
	"\t\t\t/* over Class-C is not allowed! */\n"
	"\t\t\tif(ipString[0] > 223)\n"
	"\t\t\t\treturn false;\n"
	"\t\t}\n"
	"\t\tif(ipString[0]==0)  /* 0.x.x.x  is not allowed  */\n"
	"\t\t\treturn false;\n"
	"\t\treturn true;\n"
	"    }\n"
	"}\n"
	"\n";

static char PgjsComm_Item_14[] =
	"\n"
	" function ipCheck3(textValue)\n"
	"  {\n"
	"\n"
	"    re1=/(\\d+)(\\W)(\\d+)(\\W)(\\d+)(\\W)(\\d+)/\n"
	"    Check=textValue.search(re1);\n"
	"    if(Check==-1)   return false;\n"
	"\n"
	"    else\n"
	"    {\n"
	"       var ipSplit=textValue.split(\'.\');  ";

static char PgjsComm_Item_15[] =
	"    \tif (textValue.indexOf(\',\') != -1)     return false;\t";

static char PgjsComm_Item_16[] =
	"    \tif (textValue.indexOf(\' \') != -1)     return false;\n"
	"       ipSplit=textValue.split(\'.\');\n"
	"       if(ipSplit.length!=4) return false;\n"
	"       for(i=0; i<ipSplit.length; i++)\n"
	"       {\n"
	"\t     if(isNaN(ipSplit[i]))  return false;\n"
	"         if(ipSplit[i]>255)  return false;\n"
	"       }\n"
	"\n"
	"\t   if(ipSplit[0]==0 ) return false;\t//case 11\n"
	"\n"
	"       if(ipSplit[0]==127) return false;  ";

static char PgjsComm_Item_17[] =
	"       if(ipSplit[0]>=224 && ipSplit[0]<=255) return false; ";

static char PgjsComm_Item_18[] =
	"\n"
	"       if(ipSplit[0]==128 && ipSplit[1]==0) return false;   ";

static char PgjsComm_Item_19[] =
	"       if(ipSplit[0]==191 && ipSplit[1]==255) return false;  ";

static char PgjsComm_Item_20[] =
	"\n"
	"\n"
	"       if(ipSplit[3]==255 || ipSplit[3]==0)\n"
	"        {\n"
	"           if(ipSplit[0]>=192 && ipSplit[0]<=255) return false;  ";

static char PgjsComm_Item_21[] =
	"\n"
	"           if(ipSplit[2]==255 || ipSplit[2]==0)\n"
	"           {\n"
	"           \t  if(ipSplit[0]>=128 && ipSplit[0]<=191) return false; ";

static char PgjsComm_Item_22[] =
	"\n"
	"           \t  if(ipSplit[1]==255 || ipSplit[1]==0)\n"
	"           \t  if(ipSplit[0]>=64 && ipSplit[0]<=127) return false; ";

static char PgjsComm_Item_23[] =
	"           }\n"
	"\n"
	"        }\n"
	"\n"
	"       return true;\n"
	"\n"
	"    }\n"
	" }\n"
	"\n";

static char PgjsComm_Item_24[] =
	"\n"
	" function ipCheckZero(textValue)\n"
	"  {\n"
	"\n"
	"    if (textValue.indexOf(\',\') != -1)     return false;\t";

static char PgjsComm_Item_25[] =
	"    if (textValue.indexOf(\' \') != -1)     return false;\n"
	"\n"
	"    re1=/(\\d+)(\\W)(\\d+)(\\W)(\\d+)(\\W)(\\d+)/\n"
	"    Check=textValue.search(re1);\n"
	"\n"
	"    if(Check==-1)\n"
	"       return false;\n"
	"    else\n"
	"    {\n"
	"       var ipSplit=textValue.split(\'.\');  ";

static char PgjsComm_Item_26[] =
	"\n"
	"       if(ipSplit.length!=4) return false;\n"
	"\n"
	"       for(i=0; i<ipSplit.length; i++)\n"
	"       {\n"
	"\t\t if(isNaN(ipSplit[i]))  return false;\n"
	"         if(ipSplit[i]<=0)  return false;\n"
	"       }\n"
	"\n"
	"       return true;\n"
	"\n"
	"    }\n"
	" }\n";

static char PgjsComm_Item_27[] =
	"function McaseIpCheck(textValue)\n"
	"{\n"
	"\n"
	"\tre1=/(\\d+)(\\W)(\\d+)(\\W)(\\d+)(\\W)(\\d+)/\n"
	"    Check=textValue.search(re1);\n"
	"    if(Check==-1)   return false;\n"
	"\n"
	"    else\n"
	"    {\n"
	"       var ipSplit=textValue.split(\'.\');  ";

static char PgjsComm_Item_28[] =
	"    \tif (textValue.indexOf(\',\') != -1)     return false;\t";

static char PgjsComm_Item_29[] =
	"    \tif (textValue.indexOf(\' \') != -1)     return false;\n"
	"       ipSplit=textValue.split(\'.\');\n"
	"       if(ipSplit.length!=4) return false;\n"
	"       for(i=0; i<ipSplit.length; i++)\n"
	"       {\n"
	"\t     if(isNaN(ipSplit[i]))  return false;\n"
	"         if(ipSplit[i]>255)  return false;\n"
	"       }\n"
	"\n"
	"       if(ipSplit[0]<224 || ipSplit[0]>255) return false; ";

static char PgjsComm_Item_30[] =
	"\n"
	"       return true;\n"
	"\n"
	"    }}\n"
	"\n"
	"\n";

static char PgjsComm_Item_31[] =
	"\n"
	" function MaskCheck(textValue)\n"
	" {\n"
	"    var Len= textValue.length\n"
	"    var  flag=1;\n"
	"\tif (textValue.indexOf(\',\') != -1)     return false;\n"
	"\tif (textValue.indexOf(\' \') != -1)     return false;\n"
	"    for(var i=0; i<Len; i++)\n"
	"    {\n"
	"      if(textValue.charAt(i)==\'.\')  flag=0;\n"
	"\n"
	"    }\n"
	"\n"
	"   if (flag==0) return  false;\n"
	"\n"
	"\n"
	"        re2=/[^A-Fa-f0-9]/\n"
	"\n"
	"        Check=textValue.search(re2);\n"
	"\n"
	"     \t  if(Check==-1)  return true;\n"
	"     \t  else return  false;\n"
	"\n"
	"}\n"
	"\n";

static char PgjsComm_Item_32[] =
	"\n"
	" function macCheck(textValue)\n"
	" {\n"
	"\tre1=/(-)|(:)|(\\s)/\n"
	"\tCheck=textValue.search(re1);\n"
	"\tif (textValue.indexOf(\',\') != -1)     \n"
	"\t\treturn false;\n"
	"\tif(Check==-1)\n"
	"\t{\n"
	"\t\tif(textValue.length!=12)\n"
	"\t\t\treturn false;\n"
	"\t\telse  // accept mac format: xxxxxxxxxxxx\n"
	"\t\t{\n"
	"\t\t\tfor(i=0; i< textValue.length; i++)\n"
	"\t\t\t{\n"
	"\t\t\t\tre2=/[A-Fa-f0-9]/\n"
	"\t\t\t\tCheck=textValue.charAt(i).search(re2);\n"
	"\t\t\t\tif(Check==-1)  \n"
	"\t\t\t\t\treturn false;\n"
	"\t\t\t}\n"
	"\t\t}\n"
	"\t}\n"
	"   else\n"
	"   {\n"
	"\t\tmacSplit=textValue.split(\'-\');\n"
	"\t\tif(macSplit.length!=6)\n"
	"\t\t{\n"
	"\t\t\tmacSplit=textValue.split(\':\');\n"
	"\t\t\tif(macSplit.length!=6)\n"
	"\t\t\t{\n"
	"\t\t\t\tmacSplit=textValue.split(\' \');\n"
	"\t\t\t\tif(macSplit.length!=6)\n"
	"\t\t\t\t\treturn false;\n"
	"\t\t\t}\n"
	"\t\t}\n"
	"\n"
	"\t\tfor(i=0; i< macSplit.length; i++)\n"
	"\t\t{\n"
	"\t\t\tif (macSplit[i].length!=2)\n"
	"\t\t\t\treturn false;\n"
	"\t\t\n"
	"\t\t\tre2=/[A-Fa-f0-9][A-Fa-f0-9]/\n"
	"\t\t\tCheck=macSplit[i].search(re2);\n";

static char PgjsComm_Item_33[] =
	"\t\t\tif(Check==-1)  \n"
	"\t\t\t\treturn false;\n"
	"\t\t}\n"
	"   }\n"
	" return true;\n"
	"}\n"
	"\n";

static char PgjsComm_Item_34[] =
	"\n"
	" function macMcastCheck(textValue)\n"
	" {\n"
	"    var counter=0;\n"
	"    var ZeroCounter=0;\n"
	"\tif (textValue.indexOf(\',\') != -1)     return false;\n"
	"    for(var i=0; i<12+5;i++)\n"
	"    {\n"
	"      if(textValue.charAt(i)==\'F\' || textValue.charAt(i)==\'f\')\n"
	"       counter++;\n"
	"      if (i>1 && textValue.charAt(i)==\'0\')\n"
	"        ZeroCounter++;\n"
	"    }\n"
	"    if(counter==12) return false; ";

static char PgjsComm_Item_35[] =
	"    else if(ZeroCounter>9)\n"
	"\treturn false;  ";

static char PgjsComm_Item_36[] =
	"    else\n"
	"    {\n"
	"   \t\tcheck=textValue.charAt(1);\n"
	"    \tif((check==\'A\') ||\n"
	"    \t   (check==\'C\') ||\n"
	"    \t   (check==\'E\') ||\n"
	"    \t   (check==\'0\') ||\n"
	"           (check==\'2\') ||\n"
	"           (check==\'4\') ||\n"
	"           (check==\'6\') ||\n"
	"           (check==\'8\'))\n"
	"           return false;\n"
	"    }\n"
	"    return true;\n"
	" }\n"
	"\n";

static char PgjsComm_Item_37[] =
	"\n"
	" function macUcastCheck(textValue)\n"
	" {\n"
	"\n"
	"   \tcheck=textValue.charAt(1);\n"
	"    \tif((check==\'1\') ||\n"
	"    \t   (check==\'3\') ||\n"
	"    \t   (check==\'5\') ||\n"
	"    \t   (check==\'7\') ||\n"
	"           (check==\'8\') ||\n"
	"           (check==\'B\') ||\n"
	"           (check==\'D\') ||\n"
	"           (check==\'F\'))\n"
	"           return false;\n"
	"\n"
	"    return true;\n"
	" }\n"
	"\n";

static char PgjsComm_Item_38[] =
	"\n"
	" function UcastMACCheck(textValue)\n"
	" {\n"
	"\n"
	"   \tvar check1=textValue.charAt(0);\n"
	"   \tvar check2=textValue.charAt(1);\n"
	"   \tvar counter=0;\n"
	"\tif (textValue.indexOf(\',\') != -1)     return false;\n"
	"   \tfor(var i=0;i<12+5;i++)\n"
	"   \t{\n"
	"   \t\tif(textValue.charAt(i)==\'0\')\n"
	"   \t\t\tcounter++;\n"
	"   \t}\n"
	"\n"
	"   \tif(counter==12)\n"
	"   \t\treturn false;\n"
	"\n"
	"   \tif(check1 !=0 || check2 !=0)\n"
	"           return false;\n"
	"\n"
	"    return true;\n"
	" }\n"
	"\n"
	"\n";

static char PgjsComm_Item_39[] =
	"\n"
	"function CheckSubnetMask(textValue)\n"
	"{\n"
	"\tvar subIP=textValue.split(\'.\');\n"
	"\tvar i;\n"
	"\tif (textValue.indexOf(\',\') != -1)     return false;\n"
	"\tif (textValue.indexOf(\' \') != -1)     return false;\n"
	"\tif(subIP.length!=4)\n"
	"\t\treturn false;\n"
	"\n"
	"\tfor(i=0; i<subIP.length; i++)\n"
	"\t\t{\n"
	"\t\t\tif(isNaN(subIP[i]))  return false;\n"
	"\t\t}\n"
	"\n"
	"\tif(subIP[0]==0 && subIP[1]==0 && subIP[2]==0 && subIP[3]==0)\n"
	"\t\treturn false;\n"
	"\n"
	"\tfor(i=0;i<3;i++)\n"
	"\t{\n"
	"\t\tif(subIP[i]<subIP[i+1])\n"
	"\t\t\treturn false;\n"
	"\t}\n"
	"\n"
	"\tfor(i=0;i<4;i++)\n"
	"\t{\n"
	"\t\tif(subIP[i] != 255 && subIP[i] != 254 && subIP[i] != 252 && "
	"subIP[i] != 248 && subIP[i] != 240 && subIP[i] != 224\n"
	"\t\t&& subIP[i] != 192 && subIP[i] != 128 && subIP[i] != 0)\n"
	"\t\t\treturn false;\n"
	"\t}\n"
	"\n"
	"\tfor(i=0;i<3;i++)\n"
	"\t{\n"
	"\t\tif(subIP[i]!= 255 && subIP[i+1] !=0)\n"
	"\t\t\treturn false;\n"
	"\t}\n"
	"\n"
	"\treturn true;\n"
	"\n"
	"}\n"
	"\n";

static char PgjsComm_Item_40[] =
	"// -----------------------------------------------------------------\n"
	"// function isAsciiString()\n"
	"// purpose : If user keyin simple chinese or chinese will return fale\n"
	"// Author : jacob\n"
	"// -----------------------------------------------------------------\n"
	"function isAsciiString(s)\n"
	"{\n"
	"\tvar len=s.length;\n"
	"\tvar ch;\n"
	"\tfor(i=0;i<len;i++){\n"
	"\t\tch=s.charCodeAt(i);\n"
	"\t\tif(ch>0x7e||ch<0x20)\n"
	"\t\t\treturn false;\n"
	"\t}\n"
	"\treturn true;\n"
	"}\n";

static char PgjsComm_Item_41[] =
	"\n"
	"function MonitorIPCheck(textValue)\n"
	"  {\n"
	"\n"
	"    re1=/(\\d+)(\\W)(\\d+)(\\W)(\\d+)(\\W)(\\d+)/\n"
	"    Check=textValue.search(re1);\n"
	"    if(Check==-1)   return false;\n"
	"\n"
	"    else\n"
	"    {\n"
	"\n"
	"       var ipSplit=textValue.split(\'.\');  ";

static char PgjsComm_Item_42[] =
	"       if(ipSplit.length!=4)\n"
	"       \t\treturn false;\n"
	"\n"
	"       for(i=0; i<ipSplit.length; i++)\n"
	"       {\n"
	"         if (isNaN(ipSplit[i]))  return false;\n"
	"         if(ipSplit[i]>255 || ipSplit[i]<0)  return false;\n"
	"       }\n"
	"\n"
	"       return true;\n"
	"\n"
	"    }\n"
	"}\n"
	"\n";

static char PgjsComm_Item_43[] =
	"\n"
	"function MonitorIPCheck2(textValue)\n"
	"  {\n"
	"\n"
	"    re1=/(\\d+)(\\W)(\\d+)(\\W)(\\d+)(\\W)(\\d+)/\n"
	"    Check=textValue.search(re1);\n"
	"    if(Check==-1)   return false;\n"
	"\n"
	"    else\n"
	"    {\n"
	"\n"
	"       var ipSplit=textValue.split(\'.\');  ";

static char PgjsComm_Item_44[] =
	"       if(ipSplit.length!=4)\n"
	"       \t\treturn false;\n"
	"\n"
	"       for(i=0; i<ipSplit.length; i++)\n"
	"       {\n"
	"         if (isNaN(ipSplit[i]))  return false;\n"
	"         if(ipSplit[i]>255 || ipSplit[i]<0)  return false;\n"
	"       }\n"
	"       if(ipSplit[0]+ipSplit[1]+ipSplit[2]+ipSplit[3]==0)  return "
	"false;\n"
	"\n"
	"       return true;\n"
	"\n"
	"    }\n"
	"}\n"
	"\n"
	"function isSpaceInString(str)\n"
	"{\n"
	"    var i=0;\n"
	"\tvar ch=\' \';\n"
	"\n"
	"    for (i=0; i < str.length; i++)\n"
	"\t{\n"
	"      ch= str.charAt(i);\n"
	"      if (ch==\' \')\n"
	"\t      return true;\n"
	"\t}\n"
	"\treturn false;\n"
	"}\n"
	"\n"
	"/* ----------------------------------------------------- */\n"
	"/*\tFunction:\tcheckAccessRight()\n"
	"/*\tPurpose:\tCheck access right\n"
	"/*\tInput:\t\tNone\n"
	"/*\tOutput:\t\tNone\n"
	"/*\tReturn:\t\t0->false, 1->true\n"
	"/* ----------------------------------------------------- */\n"
	"\n";

static char PgjsComm_Item_45[] =
	"//===================================================================="
	"==========================\n"
	"//  Arthur Chow - Check Access Right\n"
	"//  Read-Only: show error message and return false\n"
	"//  Read-Write: return true\n"
	"//===================================================================="
	"==========================\n"
	"function checkAccessRight()\n"
	"{\n";

static char PgjsComm_Item_48[] =
	"alert(\"User No Access Right.\");\n"
	"return false;\n";

static rpItem PgjsComm_Item_47[] = { 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_48 }, 
	{ eRpItemType_LastItemInList } 
};

static char PgjsComm_Item_50[] =
	"return true;\n";

static rpItem PgjsComm_Item_49[] = { 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_50 }, 
	{ eRpItemType_LastItemInList } 
};

static rpItem PgjsComm_Item_46_Group[] = { 
	{ eRpItemType_ItemGroup, (void*)&PgjsComm_Item_47 }, 
	{ eRpItemType_ItemGroup, (void*)&PgjsComm_Item_49 }, 
	{ eRpItemType_LastItemInList } 
};

extern Unsigned8 html_RpWebID_GetAccessRight(void *theTaskDataPtr, Signed16Ptr theIndexValuesPtr);
static rpDynamicDisplayItem PgjsComm_Item_46 = {
	(void*)html_RpWebID_GetAccessRight,
	eRpVarType_Complex,
	2,
	PgjsComm_Item_46_Group
};

static char PgjsComm_Item_51[] =
	"}\n"
	"//===================================================================="
	"==========================\n"
	"//  Neil Chen - Check Admin Access Right\n"
	"//  Non Admin Access Right: show error message and return false\n"
	"//  Admin Access Right: return true\n"
	"//===================================================================="
	"==========================\n"
	"function checkAdminAccessRight()\n"
	"{\n";

static char PgjsComm_Item_54[] =
	"alert(\"User No Access Right.\");\n"
	"return false;\n";

static rpItem PgjsComm_Item_53[] = { 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_54 }, 
	{ eRpItemType_LastItemInList } 
};

static char PgjsComm_Item_56[] =
	"return true;\n";

static rpItem PgjsComm_Item_55[] = { 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_56 }, 
	{ eRpItemType_LastItemInList } 
};

static rpItem PgjsComm_Item_52_Group[] = { 
	{ eRpItemType_ItemGroup, (void*)&PgjsComm_Item_53 }, 
	{ eRpItemType_ItemGroup, (void*)&PgjsComm_Item_55 }, 
	{ eRpItemType_LastItemInList } 
};

extern Unsigned8 html_RpWebID_GetAdminAccessRight(void *theTaskDataPtr, Signed16Ptr theIndexValuesPtr);
static rpDynamicDisplayItem PgjsComm_Item_52 = {
	(void*)html_RpWebID_GetAdminAccessRight,
	eRpVarType_Complex,
	2,
	PgjsComm_Item_52_Group
};

static char PgjsComm_Item_57[] =
	"}\n"
	"\n"
	"\n"
	"\n"
	"/* ----------------------------------------------------- */\n"
	"/*\tFunction:\tcheckASCIICode()\n"
	"/*\tPurpose:\tfilter the user input\n"
	"/*\tInput:\t\tuser input\n"
	"/*\tOutput:\t\tNone\n"
	"/*\tReturn:\t\terror->false\n"
	"/* ----------------------------------------------------- */\n"
	"function checkASCIICode(input)\n"
	"{\n"
	"\tvar ch;\n"
	"\tvar len;\n"
	"\n"
	"\tlen=input.value.length;\n"
	"\n"
	"\tfor(i=0;i<len;i++)\n"
	"    {\n"
	"\t\tch=input.value.charCodeAt(i);\n"
	"\t\tif(ch>0x7e || ch<0x20)\n"
	"\t\t{\n"
	"\t\t\talert(\"Input Error!\");\n"
	"\t\t\tinput.focus();\n"
	"\t\t\tinput.select();\n"
	"\t\t\treturn false;\n"
	"\t\t}\n"
	"\t}\n"
	"\treturn true;\n"
	"}\n"
	"\n"
	"\n"
	"function openRH(url)\n"
	"{\n"
	"  open(url);\n"
	"}\n"
	"\n";

static char PgjsComm_Item_60[] =
	"<!-- English -->\n"
	"var current_language=0;//English\n";

static rpItem PgjsComm_Item_59[] = { 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_60 }, 
	{ eRpItemType_LastItemInList } 
};

static char PgjsComm_Item_62[] =
	"<!-- Traditional -->\n"
	"var current_language=1;//Traditional\n";

static rpItem PgjsComm_Item_61[] = { 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_62 }, 
	{ eRpItemType_LastItemInList } 
};

static rpItem PgjsComm_Item_58_Group[] = { 
	{ eRpItemType_ItemGroup, (void*)&PgjsComm_Item_59 }, 
	{ eRpItemType_ItemGroup, (void*)&PgjsComm_Item_61 }, 
	{ eRpItemType_LastItemInList } 
};

extern Unsigned8 sWeb_Language_DynamicDisplay(void *theTaskDataPtr, Signed16Ptr theIndexValuesPtr);
static rpDynamicDisplayItem PgjsComm_Item_58 = {
	(void*)sWeb_Language_DynamicDisplay,
	eRpVarType_Complex,
	2,
	PgjsComm_Item_58_Group
};

static char PgjsComm_Item_63[] =
	"if (parent.m!=null)\n"
	"{\n"
	"\tvar menu_url_string=parent.m.location.href;\n"
	"\tif (menu_url_string!=\"\")\n"
	"\t{\n"
	"\t\tvar "
	"menu_language=menu_url_string.charAt(menu_url_string.length-1);\n"
	"\t\tif (menu_language!=current_language)\n"
	"\t\t\tparent.m.location.href=\"./menu.htm?\"+current_language;\n"
	"\t}\n"
	"}\n";


static rpItem PgjsComm_Items[] = { 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_1 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_2 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_3 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_4 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_5 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_6 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_7 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_8 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_9 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_10 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_11 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_12 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_13 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_14 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_15 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_16 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_17 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_18 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_19 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_20 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_21 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_22 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_23 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_24 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_25 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_26 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_27 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_28 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_29 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_30 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_31 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_32 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_33 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_34 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_35 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_36 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_37 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_38 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_39 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_40 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_41 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_42 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_43 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_44 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_45 }, 
	{ eRpItemType_DynamicDisplay, (void*)&PgjsComm_Item_46 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_51 }, 
	{ eRpItemType_DynamicDisplay, (void*)&PgjsComm_Item_52 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_57 }, 
	{ eRpItemType_DynamicDisplay, (void*)&PgjsComm_Item_58 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgjsComm_Item_63 }, 
	{ eRpItemType_LastItemInList } 
};


rpObjectDescription PgjsComm = {
	"/html/jsComm.js",
	PgjsComm_Items,
	(rpObjectExtensionPtr) 0,
	(Unsigned32) 0,
	kRpPageAccess_Unprotected,
	eRpDataTypeJs,
	eRpObjectTypeDynamic
};


/* ********************************* */
/* *    Built from "html\ua.js"    * */
/* ********************************* */

extern rpObjectDescription Pgua;

static char Pgua_Item_1[] =
	"/*\n"
	" * $Log: ua.js,v $\n"
	" * Revision 1.9  2002/07/22 14:06:21  bc6ix\n"
	" * fix license path, change version reporting to use 2 digits for each"
	" level\n"
	" *\n"
	" * Revision 1.8  2002/07/07 08:23:07  bc6ix\n"
	" * fix line endings\n"
	" *\n"
	" * Revision 1.7  2002/05/14 16:52:52  bc6ix\n"
	" * use CVS Log for revision history\n"
	" *\n"
	" *\n"
	" */\n"
	"\n"
	"/* ***** BEGIN LICENSE BLOCK *****\n"
	" * Licensed under Version: MPL 1.1/GPL 2.0/LGPL 2.1\n"
	" * Full Terms at http://bclary.com/lib/js/license/mpl-tri-license.txt\n"
	" *\n"
	" * Software distributed under the License is distributed on an \"AS "
	"IS\" basis,\n"
	" * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the "
	"License\n"
	" * for the specific language governing rights and limitations under "
	"the\n"
	" * License.\n"
	" *\n"
	" * The Original Code is Netscape code.\n"
	" *\n"
	" * The Initial Developer of the Original Code is\n";

static char Pgua_Item_2[] =
	" * Netscape Corporation.\n"
	" * Portions created by the Initial Developer are Copyright (C) 2001\n"
	" * the Initial Developer. All Rights Reserved.\n"
	" *\n"
	" * Contributor(s): Bob Clary <bclary@netscape.com>\n"
	" *\n"
	" * ***** END LICENSE BLOCK ***** */\n"
	"\n"
	"function xbDetectBrowser()\n"
	"{\n"
	"  var oldOnError = window.onerror;\n"
	"  var element = null;\n"
	"\n"
	"  window.onerror = null;\n"
	"  \n"
	"  // work around bug in xpcdom Mozilla 0.9.1\n"
	"  window.saveNavigator = window.navigator;\n"
	"\n"
	"  navigator.OS    = \'\';\n"
	"  navigator.version  = parseFloat(navigator.appVersion);\n"
	"  navigator.org    = \'\';\n"
	"  navigator.family  = \'\';\n"
	"\n"
	"  var platform;\n"
	"  if (typeof(window.navigator.platform) != \'undefined\')\n"
	"  {\n"
	"    platform = window.navigator.platform.toLowerCase();\n"
	"    if (platform.indexOf(\'win\') != -1)\n"
	"      navigator.OS = \'win\';\n";

static char Pgua_Item_3[] =
	"    else if (platform.indexOf(\'mac\') != -1)\n"
	"      navigator.OS = \'mac\';\n"
	"    else if (platform.indexOf(\'unix\') != -1 || "
	"platform.indexOf(\'linux\') != -1 || platform.indexOf(\'sun\') != -1)\n"
	"      navigator.OS = \'nix\';\n"
	"  }\n"
	"\n"
	"  var i = 0;\n"
	"  var ua = window.navigator.userAgent.toLowerCase();\n"
	"  \n"
	"  if (ua.indexOf(\'safari\') != -1)\n"
	"  {\n"
	"    i = ua.indexOf(\'safari\');\n"
	"    navigator.family = \'safari\';\n"
	"    navigator.org = \'safari\';\n"
	"    navigator.version = parseFloat(\'0\' + ua.substr(i+7), 10);\n"
	"  }\n"
	"  else if (ua.indexOf(\'opera\') != -1)\n"
	"  {\n"
	"    i = ua.indexOf(\'opera\');\n"
	"    navigator.family  = \'opera\';\n"
	"    navigator.org    = \'opera\';\n"
	"    navigator.version  = parseFloat(\'0\' + ua.substr(i+6), 10);\n"
	"  }\n"
	"  else if ((i = ua.indexOf(\'msie\')) != -1)\n"
	"  {\n"
	"    navigator.org    = \'microsoft\';\n";

static char Pgua_Item_4[] =
	"    navigator.version  = parseFloat(\'0\' + ua.substr(i+5), 10);\n"
	"    \n"
	"    if (navigator.version < 4)\n"
	"      navigator.family = \'ie3\';\n"
	"    else\n"
	"      navigator.family = \'ie4\'\n"
	"  }\n"
	"  else if (ua.indexOf(\'gecko\') != -1)\n"
	"  {\n"
	"    navigator.family = \'gecko\';\n"
	"    var rvStart = ua.indexOf(\'rv:\');\n"
	"    var rvEnd   = ua.indexOf(\')\', rvStart);\n"
	"    var rv      = ua.substring(rvStart+3, rvEnd);\n"
	"    var rvParts = rv.split(\'.\');\n"
	"    var rvValue = 0;\n"
	"    var exp     = 1;\n"
	"\n"
	"    for (var i = 0; i < rvParts.length; i++)\n"
	"    {\n"
	"      var val = parseInt(rvParts[i]);\n"
	"      rvValue += val / exp;\n"
	"      exp *= 100;\n"
	"    }\n"
	"    navigator.version = rvValue;\n"
	"\n"
	"    if (ua.indexOf(\'netscape\') != -1)\n"
	"      navigator.org = \'netscape\';\n"
	"    else if (ua.indexOf(\'compuserve\') != -1)\n"
	"      navigator.org = \'compuserve\';\n"
	"    else\n";

static char Pgua_Item_5[] =
	"      navigator.org = \'mozilla\';\n"
	"  }\n"
	"  else if ((ua.indexOf(\'mozilla\') !=-1) && "
	"(ua.indexOf(\'spoofer\')==-1) && (ua.indexOf(\'compatible\') == -1) &&"
	" (ua.indexOf(\'opera\')==-1)&& (ua.indexOf(\'webtv\')==-1) && "
	"(ua.indexOf(\'hotjava\')==-1))\n"
	"  {\n"
	"    var is_major = parseFloat(navigator.appVersion);\n"
	"    \n"
	"    if (is_major < 4)\n"
	"      navigator.version = is_major;\n"
	"    else\n"
	"    {\n"
	"      i = ua.lastIndexOf(\'/\')\n"
	"      navigator.version = parseFloat(\'0\' + ua.substr(i+1), 10);\n"
	"    }\n"
	"    navigator.org = \'netscape\';\n"
	"    navigator.family = \'nn\' + parseInt(navigator.appVersion);\n"
	"  }\n"
	"  else if ((i = ua.indexOf(\'aol\')) != -1 )\n"
	"  {\n"
	"    // aol\n"
	"    navigator.family  = \'aol\';\n"
	"    navigator.org    = \'aol\';\n"
	"    navigator.version  = parseFloat(\'0\' + ua.substr(i+4), 10);\n"
	"  }\n"
	"  else if ((i = ua.indexOf(\'hotjava\')) != -1 )\n";

static char Pgua_Item_6[] =
	"  {\n"
	"    // hotjava\n"
	"    navigator.family  = \'hotjava\';\n"
	"    navigator.org    = \'sun\';\n"
	"    navigator.version  = parseFloat(navigator.appVersion);\n"
	"  }\n"
	"\n"
	"  window.onerror = oldOnError;\n"
	"}\n"
	"\n"
	"xbDetectBrowser();\n"
	"\n";


static rpItem Pgua_Items[] = { 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgua_Item_1 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgua_Item_2 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgua_Item_3 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgua_Item_4 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgua_Item_5 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgua_Item_6 }, 
	{ eRpItemType_LastItemInList } 
};


rpObjectDescription Pgua = {
	"/html/ua.js",
	Pgua_Items,
	(rpObjectExtensionPtr) 0,
	(Unsigned32) 0,
	kRpPageAccess_Unprotected,
	eRpDataTypeJs,
	eRpObjectTypeDynamic
};


/* ************************************** */
/* *    Built from "html\ftiens4.js"    * */
/* ************************************** */

extern rpObjectDescription Pgftiens4;

static char Pgftiens4_Item_1[] =
	"//****************************************************************\n"
	"// Keep this copyright notice:\n"
	"// This copy of the script is the property of the owner of the\n"
	"// particular web site you were visiting.\n"
	"// Do not download the script\'s files from there.\n"
	"// For a free download and full instructions go to:\n"
	"// http://www.geocities.com/marcelino_martins/foldertree.html\n"
	"//\n"
	"// Author: Marcelino Alves Martins (http://www.mmartins.com)\n"
	"// 1997--2001.\n"
	"//****************************************************************\n"
	"\n"
	"// Log of changes:\n"
	"//       10 Aug 01 - Support for Netscape 6\n"
	"//\n"
	"//       17 Feb 98 - Fix initialization flashing problem with Netscape"
	"\n"
	"//\n"
	"//       27 Jan 98 - Root folder starts open; support for "
	"USETEXTLINKS;\n"
	"//                   make the ftien4 a js file\n"
	"\n"
	"\n";

static char Pgftiens4_Item_2[] =
	"// Definition of class Folder\n"
	"// *****************************************************************\n"
	"\n"
	"///////////////////////////////////////////////////////////\n"
	"//Arthur Chow 2005/01/28 -- For Dynamic Menu Group Display\n"
	"\n"
	"function GetDynamicMenuGroupState(groupName)\n"
	"{\n"
	"  var i=0;\n"
	"\n"
	"  for (i=0; i<parent.menu.g_DynamicMenuGroupNum; i++)\n"
	"  {\n"
	"    if (parent.menu.g_DynamicMenuGroupName[i]==groupName)\n"
	"\t    return parent.menu.g_DynamicMenuGroupState[i];\n"
	"  }\n"
	"  return 0;\n"
	"}\n"
	"\n"
	"function GetDynamicMenuGroupIndex(groupName)\n"
	"{\n"
	"  var i=0;\n"
	"\n"
	"  for (i=0; i<parent.menu.g_DynamicMenuGroupNum; i++)\n"
	"  {\n"
	"    if (parent.menu.g_DynamicMenuGroupName[i]==groupName)\n"
	"\t    return i;\n"
	"  }\n"
	"  return -1;\n"
	"}\n"
	"\n"
	"function SetDynamicMenuGroup(groupName, groupState)\n"
	"{\n"
	"  var i=0;\n"
	"\n"
	"  for (i=0; i<parent.menu.g_DynamicMenuGroupNum; i++)\n"
	"  {\n";

static char Pgftiens4_Item_3[] =
	"    if (parent.menu.g_DynamicMenuGroupName[i]==groupName)\n"
	"\t{\n"
	"\t    parent.menu.g_DynamicMenuGroupState[i]=groupState;\n"
	"        clickOnNode(parent.menu.g_DynamicMenuGroupFolderID[i]);\n"
	"        clickOnNode(parent.menu.g_DynamicMenuGroupFolderID[i]);\n"
	"\t\treturn 1;\n"
	"\t}\n"
	"  }\n"
	"  return 0;\n"
	"}\n"
	"\n"
	"function DisplayDynamicMenuGroup(folder, childrenID)\n"
	"{\n"
	"     var i=0;\n"
	"     while (i<g_DynamicMenuGroupNum)\n"
	"\t {\n"
	" \t   if (folder.desc==g_DynamicMenuGroupName[i])\n"
	"\t   {\n"
	"\t     if "
	"(folder.children[childrenID].group==g_DynamicMenuGroupState[i])\n"
	"             return 1;\n"
	"         else\n"
	"             return 0;\n"
	"\t   }\n"
	"\t   i++;\n"
	"\t }\n"
	"     return 1;\n"
	"}\n"
	"\n"
	"//Arthur Chow 2005/01/28\n"
	"///////////////////////////////////////////////////////////\n"
	"\n"
	"function Folder(folderDescription, hreference, depth) //constructor\n"
	"{\n"
	"  //constant data\n";

static char Pgftiens4_Item_4[] =
	"  this.desc = folderDescription\n"
	"  this.hreference = hreference\n"
	"  this.id = -1\n"
	"  this.navObj = 0\n"
	"  this.iconImg = 0\n"
	"  this.nodeImg = 0\n"
	"  this.isLastNode = 0\n"
	"  this.depth=depth\n"
	"  this.group = 0\n"
	"\n"
	"  //dynamic data\n"
	"  this.isOpen = true\n"
	"  this.iconSrc = \"../Images/FolderTree/ftv2folderopen.gif\"\n"
	"  this.children = new Array\n"
	"  this.nChildren = 0\n"
	"\n"
	"  //methods\n"
	"  this.initialize = initializeFolder\n"
	"  this.setState = setStateFolder\n"
	"  this.addChild = addChild\n"
	"  this.createIndex = createEntryIndex\n"
	"  this.escondeBlock = escondeBlock\n"
	"  this.esconde = escondeFolder\n"
	"  this.mostra = mostra\n"
	"  this.renderOb = drawFolder\n"
	"  this.totalHeight = totalHeight\n"
	"  this.subEntries = folderSubEntries\n"
	"  this.outputLink = outputFolderLink\n"
	"  this.blockStart = blockStart\n"
	"  this.blockEnd = blockEnd\n"
	"}\n"
	"\n";

static char Pgftiens4_Item_5[] =
	"function initializeFolder(level, lastNode, leftSide, depth)\n"
	"{\n"
	"  var j=0\n"
	"  var i=0\n"
	"  var numberOfFolders\n"
	"  var numberOfDocs\n"
	"  var nc\n"
	"\n"
	"  nc = this.nChildren\n"
	"  this.depth=depth\n"
	"\n"
	"  this.createIndex()\n"
	"\n"
	"  if (this.group==-1)\n"
	"      "
	"parent.menu.g_DynamicMenuGroupFolderID[GetDynamicMenuGroupIndex(this.d"
	"esc)] = this.id;\n"
	"\n"
	"  var auxEv = \"\"\n"
	"\n"
	"  if (browserVersion > 0)\n"
	"    auxEv = \"<a href=\'javascript:clickOnNode(\"+this.id+\")\'>\"\n"
	"  else\n"
	"    auxEv = \"<a>\"\n"
	"\n"
	"  if (level>0)\n"
	"    if (lastNode) //the last child in the children array\n"
	"    {\n"
	"      this.renderOb(leftSide + auxEv + \"<img" C_NAME "\'nodeIcon\" + "
	"this.id + \"\' id=\'nodeIcon\" + this.id + \"\' "
	"src=\'../Images/FolderTree/ftv2lastnode.gif\'" C_WIDTH "16" C_HEIGHT
	"22" C_BORDER "0>" C_xANCHOR "\", this.depth)\n"
	"      leftSide = leftSide + \"<img ";

static char Pgftiens4_Item_6[] =
	"src=\'../Images/FolderTree/ftv2blank.gif\'" C_WIDTH "16" C_HEIGHT "22>"
	"\"\n"
	"      this.isLastNode = 1\n"
	"    }\n"
	"    else\n"
	"    {\n"
	"      this.renderOb(leftSide + auxEv + \"<img" C_NAME "\'nodeIcon\" + "
	"this.id + \"\' id=\'nodeIcon\" + this.id + \"\' "
	"src=\'../Images/FolderTree/ftv2node.gif\'" C_WIDTH "16" C_HEIGHT "22"
	C_BORDER "0>" C_xANCHOR "\", this.depth)\n"
	"      leftSide = leftSide + \"<img "
	"src=\'../Images/FolderTree/ftv2vertline.gif\'" C_WIDTH "16" C_HEIGHT
	"22>\"\n"
	"      this.isLastNode = 0\n"
	"    }\n"
	"  else\n"
	"    this.renderOb(\"\", this.depth)\n"
	"\n"
	"  if (nc > 0)\n"
	"  {\n"
	"    level = level + 1\n"
	"    for (i=0 ; i < this.nChildren; i++)\n"
	"    {\n"
	"\n"
	"      if (i == this.nChildren-1)\n"
	"        this.children[i].initialize(level, 1, leftSide, "
	"this.children[i].depth)\n"
	"\n"
	"      else\n"
	"\t\t{\n"
	"\t\tif (this.children[i].group_lastnode)\n"
	"        this.children[i].initialize(level, 1, leftSide, ";

static char Pgftiens4_Item_7[] =
	"this.children[i].depth)\n"
	"\t\telse\n"
	"        this.children[i].initialize(level, 0, leftSide, "
	"this.children[i].depth)\n"
	"\t\t}\n"
	"\n"
	"      }\n"
	"  }\n"
	"}\n"
	"\n"
	"function setStateFolder(isOpen)\n"
	"{\n"
	"  var subEntries\n"
	"  var totalHeight\n"
	"  var fIt = 0\n"
	"  var i=0\n"
	"\n"
	"  if (isOpen == this.isOpen)\n"
	"    return\n"
	"\n"
	"  if (browserVersion == 2)\n"
	"  {\n"
	"    totalHeight = 0\n"
	"    for (i=0; i < this.nChildren; i++)\n"
	"      totalHeight = totalHeight + this.children[i].navObj.clip.height\n"
	"      subEntries = this.subEntries()\n"
	"    if (this.isOpen)\n"
	"      totalHeight = 0 - totalHeight\n"
	"    for (fIt = this.id + subEntries + 1; fIt < nEntries; fIt++)\n"
	"      indexOfEntries[fIt].navObj.moveBy(0, totalHeight)\n"
	"  }\n"
	"  this.isOpen = isOpen\n"
	"  propagateChangesInState(this)\n"
	"}\n"
	"\n"
	"function propagateChangesInState(folder)\n"
	"{\n"
	"  var i=0\n"
	"  if (folder.isOpen)\n"
	"  {\n";

static char Pgftiens4_Item_8[] =
	"    if (folder.nodeImg)\n"
	"     if (folder.isLastNode)\n"
	"        folder.nodeImg.src = \"../Images/FolderTree/ftv2lastnode.gif\""
	"\n"
	"      else\n"
	"\t    folder.nodeImg.src = \"../Images/FolderTree/ftv2node.gif\"\n"
	"    folder.iconImg.src = \"../Images/FolderTree/ftv2folderopen.gif\"\n"
	"    for (i=0; i<folder.nChildren; i++)\n"
	"      if (DisplayDynamicMenuGroup(folder, i))\n"
	"      folder.children[i].mostra()\n"
	"  }\n"
	"  else\n"
	"  {\n"
	"    if (folder.nodeImg)\n"
	"      if (folder.isLastNode)\n"
	"        folder.nodeImg.src = \"../Images/FolderTree/ftv2lastnode.gif\""
	"\n"
	"      else\n"
	"\t    folder.nodeImg.src = \"../Images/FolderTree/ftv2node.gif\"\n"
	"    folder.iconImg.src = \"../Images/FolderTree/ftv2folderclosed.gif\""
	"\n"
	"    for (i=0; i<folder.nChildren; i++)\n"
	"      folder.children[i].esconde()\n"
	"  }\n"
	"}\n"
	"\n"
	"function escondeFolder()\n"
	"{\n"
	"  this.escondeBlock()\n"
	"\n";

static char Pgftiens4_Item_9[] =
	"  this.setState(0)\n"
	"}\n"
	"\n"
	"function drawFolder(leftSide, depth)\n"
	"{\n"
	"  var idParam = \"id=\'folder\" + this.id + \"\'\"\n"
	"  var tdWidth=0;\n"
	"\n"
	"  if (browserVersion == 2) {\n"
	"    if (!doc.yPos)\n"
	"      doc.yPos=20\n"
	"  }\n"
	"\n"
	"  this.blockStart(\"folder\")\n"
	"\n"
	"  tdWidth=0;\n"
	"  for(var i=0; i<depth-1; i++)   tdWidth+=17;\n"
	"  tdWidth+=25;\n"
	"  //alert(\"Dep:\"+depth, +\"width:\"+tdWidth)\n"
	"  doc.write(\"" C_oTR "<td nowrap" C_WIDTH "\"+tdWidth+\">\")\n"
	"  //doc.write(\"" C_oTR "<td nowrap>\")\n"
	"  doc.write(leftSide)\n"
	"  this.outputLink()\n"
	"  doc.write(\"<img id=\'folderIcon\" + this.id + \"\'" C_NAME
	"\'folderIcon\" + this.id + \"\' src=\'\" + this.iconSrc+\"\'"
	C_BORDER "0>" C_xANCHOR "\")\n"
	"  doc.write(\"" C_xTD "<td" C_VALIGN_MIDDLE " nowrap>\")\n"
	"  if (USETEXTLINKS)\n"
	"  {\n"
	"    this.outputLink()\n"
	"    doc.write(this.desc + \"" C_xANCHOR "\")\n"
	"    //doc.write(\"<font color=#FFFFFF>\"+this.desc + \"" C_xFONT
	C_xANCHOR "\");\n"
	"  }\n"
	"  else\n";

static char Pgftiens4_Item_10[] =
	"    doc.write(this.desc)\n"
	"  doc.write(\"" C_xTD "\")\n"
	"\n"
	"  this.blockEnd()\n"
	"\n"
	"  if (browserVersion == 1) {\n"
	"    this.navObj = doc.all[\"folder\"+this.id]\n"
	"    this.iconImg = doc.all[\"folderIcon\"+this.id]\n"
	"    this.nodeImg = doc.all[\"nodeIcon\"+this.id]\n"
	"  } else if (browserVersion == 2) {\n"
	"    this.navObj = doc.layers[\"folder\"+this.id]\n"
	"    this.iconImg = this.navObj.document.images[\"folderIcon\"+this.id]"
	"\n"
	"    this.nodeImg = this.navObj.document.images[\"nodeIcon\"+this.id]\n"
	"    doc.yPos=doc.yPos+this.navObj.clip.height\n"
	"  } else if (browserVersion == 3) {\n"
	"    this.navObj = doc.getElementById(\"folder\"+this.id)\n"
	"    this.iconImg = doc.getElementById(\"folderIcon\"+this.id)\n"
	"    this.nodeImg = doc.getElementById(\"nodeIcon\"+this.id)\n"
	"  }\n"
	"}\n"
	"\n"
	"function outputFolderLink()\n"
	"{\n"
	"  if (this.hreference)\n"
	"  {\n";

static char Pgftiens4_Item_11[] =
	"    doc.write(\"<a href=\'\" + this.hreference + \"\' "
	"TARGET=\\\"rbottom\\\" \")\n"
	"    if (browserVersion > 0)\n"
	"      doc.write(\"onClick=\'javascript:clickOnNode(\"+this.id+\")\'\")"
	"\n"
	"    doc.write(\">\")\n"
	"  }\n"
	"  else\n"
	"  // doc.write(\"<a>\")\n"
	"  doc.write(\"<a href=\'javascript:clickOnNode(\"+this.id+\")\'>\")\n"
	"}\n"
	"\n"
	"function addChild(group, group_lastnode, childNode, depth)\n"
	"{\n"
	"\tchildNode.depth=depth\n"
	"  this.children[this.nChildren] = childNode\n"
	"  this.children[this.nChildren].depth = depth;\n"
	"  this.children[this.nChildren].group=group;\n"
	"  this.children[this.nChildren].group_lastnode=group_lastnode;\n"
	"  this.nChildren++\n"
	"  return childNode\n"
	"}\n"
	"\n"
	"function folderSubEntries()\n"
	"{\n"
	"  var i = 0\n"
	"  var se = this.nChildren\n"
	"\n"
	"  for (i=0; i < this.nChildren; i++){\n"
	"    if (this.children[i].children) //is a folder\n";

static char Pgftiens4_Item_12[] =
	"      se = se + this.children[i].subEntries()\n"
	"  }\n"
	"\n"
	"  return se\n"
	"}\n"
	"\n"
	"\n"
	"// Definition of class Item (a document or link inside a Folder)\n"
	"// *************************************************************\n"
	"\n"
	"function Item(itemDescription, itemLink, depth) // Constructor\n"
	"{\n"
	"  // constant data\n"
	"  this.desc = itemDescription\n"
	"  this.link = itemLink\n"
	"  this.id = -1 //initialized in initalize()\n"
	"  this.navObj = 0 //initialized in render()\n"
	"  this.iconImg = 0 //initialized in render()\n"
	"  this.iconSrc = \"../Images/FolderTree/ftv2doc.gif\"\n"
	"  this.depth=depth; // add by WindChen,for Record the Tree Depth, "
	"10/29/2001\n"
	"\n"
	"  // methods\n"
	"  this.initialize = initializeItem\n"
	"  this.createIndex = createEntryIndex\n"
	"  this.esconde = escondeBlock\n"
	"  this.mostra = mostra\n"
	"  this.renderOb = drawItem\n"
	"  this.totalHeight = totalHeight\n";

static char Pgftiens4_Item_13[] =
	"  this.blockStart = blockStart\n"
	"  this.blockEnd = blockEnd\n"
	"}\n"
	"\n"
	"function initializeItem(level, lastNode, leftSide, depth)\n"
	"{\n"
	"  this.createIndex()\n"
	"\n"
	"  if (level>0)\n"
	"    if (lastNode) //the last \'brother\' in the children array\n"
	"    {\n"
	"      this.renderOb(leftSide + \"<img "
	"src=\'../Images/FolderTree/ftv2lastnode.gif\'" C_WIDTH "16" C_HEIGHT
	"22>\", depth)\n"
	"      leftSide = leftSide + \"<img "
	"src=\'../Images/FolderTree/ftv2blank.gif\'" C_WIDTH "16" C_HEIGHT "22>"
	"\"\n"
	"    }\n"
	"    else\n"
	"    {\n"
	"      this.renderOb(leftSide + \"<img "
	"src=\'../Images/FolderTree/ftv2node.gif\'" C_WIDTH "16" C_HEIGHT "22>"
	"\", depth)\n"
	"      leftSide = leftSide + \"<img "
	"src=\'../Images/FolderTree/ftv2vertline.gif\'" C_WIDTH "16" C_HEIGHT
	"22>\"\n"
	"    }\n"
	"  else\n"
	"    this.renderOb(\"\", depth)\n"
	"}\n"
	"\n"
	"function drawItem(leftSide, depth)\n"
	"{\n"
	"  this.blockStart(\"item\")\n"
	"  var tdWidth;\n"
	"\n"
	"  tdWidth=0;\n";

static char Pgftiens4_Item_14[] =
	"  for(var i=0; i<depth-1; i++)  tdWidth+=17;\n"
	"  tdWidth+=25;\n"
	"\n"
	"  doc.write(\"" C_oTR "<td nowrap" C_WIDTH "\"+tdWidth+\">\")\n"
	"  //doc.write(\"" C_oTR "<td nowrap>\")\n"
	"  doc.write(leftSide)\n"
	"  doc.write(\"" C_oANCHOR_HREF " + this.link + \">\")\n"
	"  doc.write(\"<img id=\'itemIcon\"+this.id+\"\' \")\n"
	"  doc.write(\"src=\'\"+this.iconSrc+\"\'" C_BORDER "0>\")\n"
	"  doc.write(\"" C_xANCHOR "\")\n"
	"  doc.write(\"" C_xTD "<td" C_VALIGN_MIDDLE " nowrap>\")\n"
	"  if (USETEXTLINKS)\n"
	"    doc.write(\"" C_oANCHOR_HREF " + this.link + \">\" + this.desc + \""
	C_xANCHOR "\")\n"
	"  else\n"
	"    doc.write(this.desc)\n"
	"\n"
	"  this.blockEnd()\n"
	"\n"
	"  if (browserVersion == 1) {\n"
	"    this.navObj = doc.all[\"item\"+this.id]\n"
	"    this.iconImg = doc.all[\"itemIcon\"+this.id]\n"
	"  } else if (browserVersion == 2) {\n"
	"    this.navObj = doc.layers[\"item\"+this.id]\n"
	"    this.iconImg = this.navObj.document.images[\"itemIcon\"+this.id]\n";

static char Pgftiens4_Item_15[] =
	"    doc.yPos=doc.yPos+this.navObj.clip.height\n"
	"  } else if (browserVersion == 3) {\n"
	"    this.navObj = doc.getElementById(\"item\"+this.id)\n"
	"    this.iconImg = doc.getElementById(\"itemIcon\"+this.id)\n"
	"  }\n"
	"}\n"
	"\n"
	"\n"
	"// Methods common to both objects (pseudo-inheritance)\n"
	"// ********************************************************\n"
	"\n"
	"function mostra()\n"
	"{\n"
	"  if (browserVersion == 1 || browserVersion == 3)\n"
	"    this.navObj.style.display = \"block\"\n"
	"  else\n"
	"    this.navObj.visibility = \"show\"\n"
	"}\n"
	"\n"
	"function escondeBlock()\n"
	"{\n"
	"  if (browserVersion == 1 || browserVersion == 3) {\n"
	"    if (this.navObj.style.display == \"none\")\n"
	"      return\n"
	"    this.navObj.style.display = \"none\"\n"
	"  } else {\n"
	"    if (this.navObj.visibility == \"hiden\")\n"
	"      return\n"
	"    this.navObj.visibility = \"hiden\"\n"
	"  }\n"
	"}\n"
	"\n"
	"function blockStart(idprefix) {\n";

static char Pgftiens4_Item_16[] =
	"  var idParam = \"id=\'\" + idprefix + this.id + \"\'\"\n"
	"\n"
	"  if (browserVersion == 2)\n"
	"    doc.write(\"<layer \"+ idParam + \" top=\" + doc.yPos + \" "
	"visibility=show>\")\n"
	"\n"
	"  if (browserVersion == 3) //N6 has bug on display property with "
	"tables\n"
	"    doc.write(\"" C_oDIV_ " \" + idParam + \" style=\'display:block; "
	"position:block;\'>\")\n"
	"\n"
	"  doc.write(\"" C_oTABLE_BORDER "0" C_CELLSPACING "0" C_CELLPADDING "0"
	" \")\n"
	"\n"
	"  if (browserVersion == 1)\n"
	"    doc.write(idParam + \" style=\'display:block; position:block; \'>"
	"\")\n"
	"  else\n"
	"    doc.write(\"width=300 >\")\n"
	"}\n"
	"\n"
	"function blockEnd() {\n"
	"  doc.write(\"" C_xTABLE "\")\n"
	"\n"
	"  if (browserVersion == 2)\n"
	"    doc.write(\"</layer>\")\n"
	"  if (browserVersion == 3)\n"
	"    doc.write(\"" C_xDIV "\")\n"
	"}\n"
	"\n"
	"function createEntryIndex()\n"
	"{\n"
	"  this.id = nEntries\n"
	"  indexOfEntries[nEntries] = this\n"
	"  nEntries++\n"
	"}\n"
	"\n"
	"// total height of subEntries open\n";

static char Pgftiens4_Item_17[] =
	"function totalHeight() //used with browserVersion == 2\n"
	"{\n"
	"  var h = this.navObj.clip.height\n"
	"  var i = 0\n"
	"\n"
	"  if (this.isOpen) //is a folder and _is_ open\n"
	"    for (i=0 ; i < this.nChildren; i++)\n"
	"      h = h + this.children[i].totalHeight()\n"
	"\n"
	"  return h\n"
	"}\n"
	"\n"
	"\n"
	"// Events\n"
	"// *********************************************************\n"
	"\n"
	"function clickOnFolder(folderId)\n"
	"{\n"
	"  var clicked = indexOfEntries[folderId]\n"
	"\n"
	"  if (!clicked.isOpen)\n"
	"    clickOnNode(folderId)\n"
	"\n"
	"  return\n"
	"\n"
	"  if (clicked.isSelected)\n"
	"    return\n"
	"}\n"
	"\n"
	"function clickOnNode(folderId)\n"
	"{\n"
	"  var clickedFolder = 0\n"
	"  var state = 0\n"
	"\n"
	"  if ((g_InitMenuFlag==0)&&(folderId==0)) return;\n"
	"\n"
	"  clickedFolder = indexOfEntries[folderId]\n"
	"  state = clickedFolder.isOpen\n"
	"\n"
	"  clickedFolder.setState(!state) //open<->close\n"
	"}\n"
	"\n"
	"\n";

static char Pgftiens4_Item_18[] =
	"// Auxiliary Functions for Folder-Tree backward compatibility\n"
	"// ***********************************************************\n"
	"\n"
	"function gFld(description, hreference, depth)\n"
	"{\n"
	"  folder = new Folder(description, hreference, depth)\n"
	"  return folder\n"
	"}\n"
	"\n"
	"function gLnk(target, description, linkData,depth)\n"
	"{\n"
	"  fullLink = \"\"\n"
	"\n"
	"  /*\n"
	"  if (target==0)\n"
	"  {\n"
	"    fullLink = \"\'\"+linkData+\"\' target=\\\"basefrm\\\"\"\n"
	"  }\n"
	"  else\n"
	"  {\n"
	"    if (target==1)\n"
	"       fullLink = \"\'http://\"+linkData+\"\' target=_blank\"\n"
	"    else\n"
	"       fullLink = \"\'http://\"+linkData+\"\' target=\\\"basefrm\\\"\""
	"\n"
	"  }\n"
	"  */\n"
	"  fullLink = \"\'\"+linkData+\"\' target=\\\"\"+ target + \"\\\"\"\n"
	"  linkItem = new Item(description, fullLink, depth)\n"
	"  return linkItem\n"
	"}\n"
	"\n"
	"//////////////////////////////////////////////////////////////////////"
	"/////////////////////////////\n";

static char Pgftiens4_Item_19[] =
	"// If group==0\n"
	"//   It means the folder a normal folder. \n"
	"// If group==-1\n"
	"//   It means the folder is a dynamic folder. \n"
	"// If group==1 or 2 or 3 or...\n"
	"//   It means the folder is a sub-directory of the dynamic folder "
	"group, \n"
	"//   and will be dynamic display by group ID. \n"
	"function insFld(group, group_lastnode, parentFolder, childFolder, "
	"depth)\n"
	"{\n"
	"  return parentFolder.addChild(group, group_lastnode, childFolder, "
	"depth)\n"
	"}\n"
	"\n"
	"//////////////////////////////////////////////////////////////////////"
	"/////////////////////////////\n"
	"// If group==0\n"
	"//   It means the item node a normal node that will always exist in "
	"the tree. \n"
	"// If group==1 or 2 or 3 or...\n"
	"//   It means the item node will be dynamic display by group ID. \n"
	"function insDoc(group, group_lastnode, parentFolder, document, depth)\n"
	"{\n";

static char Pgftiens4_Item_20[] =
	"  parentFolder.addChild(group, group_lastnode, document, depth)\n"
	"}\n"
	"\n"
	"\n"
	"// Global variables\n"
	"// ****************\n"
	"\n"
	"//These two variables are overwriten on defineMyTree.js if needed be\n"
	"USETEXTLINKS = 0\n"
	"STARTALLOPEN = 0\n"
	"indexOfEntries = new Array\n"
	"nEntries = 0\n"
	"doc = document\n"
	"browserVersion = 0\n"
	"selectedFolder=0\n"
	"g_InitMenuFlag = 0\n"
	"\n"
	"\n"
	"// Main function\n"
	"// *************\n"
	"\n"
	"// This function uses an object (navigator) defined in\n"
	"// ua.js, imported in the main html page (left frame).\n"
	"function initializeDocument()\n"
	"{\n"
	"  switch(navigator.family)\n"
	"  {\n"
	"    case \'ie4\':\n"
	"      browserVersion = 1 //IE4\n"
	"      break;\n"
	"    case \'nn4\':\n"
	"      browserVersion = 2 //NS4\n"
	"      break;\n"
	"    case \'gecko\':\n"
	"      browserVersion = 3 //NS6\n"
	"      break;\n"
	"    case \'safari\':\n";

static char Pgftiens4_Item_21[] =
	"      //browserVersion = 1 //Safari Beta 3 seems to behave like IE in "
	"spite of being based on Konkeror\n"
	"      browserVersion = 3 //Safari 1.0(v85) in Mac seems to behave like"
	" NS6    \n"
	"      break;\n"
	"\tdefault:\n"
	"\t  browserVersion = 0 //other\n"
	"\t  break;\n"
	"  }\n"
	"\n"
	"  //foldersTree (with the site\'s data) is created in an external .js\n"
	"  foldersTree.initialize(0, 1, \"\",1)\n"
	"\n"
	"  if (browserVersion == 2)\n"
	"    doc.write(\"<layer "
	"top=\"+indexOfEntries[nEntries-1].navObj.top+\">" C_NBSP "</layer>\")\n"
	"\n"
	"  //The tree starts in full display\n"
	"  if (!STARTALLOPEN)\n"
	"\t  if (browserVersion > 0) {\n"
	"\t\t// close the whole tree\n"
	"\t\tg_InitMenuFlag=1;\n"
	"\t\tclickOnNode(0)\n"
	"\t\t// open the root folder\n"
	"\t\tclickOnNode(0)\n"
	"\t\tg_InitMenuFlag=0;\n"
	"\t  }\n"
	"\n"
	"  if (browserVersion == 0)\n"
	"\tdoc.write(\"" C_oTABLE_BORDER "0>" C_oTR C_oTD C_oBR C_oBR;

static char Pgftiens4_Item_22[] =
	C_oFONT_SIZE "-1>This tree only expands or contracts with DHTML capable"
	" browsers" C_xFONT C_xTABLE "\")\n"
	"}\n"
	"\n";


static rpItem Pgftiens4_Items[] = { 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgftiens4_Item_1 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgftiens4_Item_2 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgftiens4_Item_3 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgftiens4_Item_4 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgftiens4_Item_5 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgftiens4_Item_6 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgftiens4_Item_7 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgftiens4_Item_8 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgftiens4_Item_9 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgftiens4_Item_10 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgftiens4_Item_11 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgftiens4_Item_12 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgftiens4_Item_13 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgftiens4_Item_14 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgftiens4_Item_15 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgftiens4_Item_16 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgftiens4_Item_17 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgftiens4_Item_18 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgftiens4_Item_19 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgftiens4_Item_20 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgftiens4_Item_21 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgftiens4_Item_22 }, 
	{ eRpItemType_LastItemInList } 
};


rpObjectDescription Pgftiens4 = {
	"/html/ftiens4.js",
	Pgftiens4_Items,
	(rpObjectExtensionPtr) 0,
	(Unsigned32) 0,
	kRpPageAccess_Unprotected,
	eRpDataTypeJs,
	eRpObjectTypeDynamic
};


/* ***************************************** */
/* *    Built from "html\web_style.css"    * */
/* ***************************************** */

extern rpObjectDescription Pgweb_style;

static char Pgweb_style_Item_1[] =
	"td.pageTitle {\n"
	"\t  background-color: #69F;\n"
	"\t  height:auto;\n"
	"\t  text-align: left;\n"
	"\t  color: #FFF;\n"
	"\t  font-family : Arial;\n"
	"\t  font-weight: bolder;\n"
	"\t  width: auto;\n"
	"\t  border: 1px solid #000;\n"
	"   \t}\n"
	"\n"
	"td.itemTitle {\n"
	"\tbackground-color:#FFC;\n"
	"\theight:25;\n"
	"\ttext-align: left;\n"
	"\tcolor: #666;\n"
	"        font-family: Times;\n"
	"\tfont-weight: bold;\n"
	"\tborder: 1px solid #000;\n"
	"\twidth: 150px;\n"
	"\t}\n"
	"\n"
	"td.itemContent {\n"
	"\tbackground-color:#FFC;\n"
	"        height:25;\n"
	"\ttext-align: left;\n"
	"\t  font-family: times;\n"
	"\tborder-top-width: 1px;\n"
	"\tborder-right-width: 1px;\n"
	"\tborder-bottom-width: 1px;\n"
	"\tborder-left-width: 1px;\n"
	"\tborder-top-style: solid;\n"
	"\tborder-right-style: solid;\n"
	"\tborder-bottom-style: solid;\n"
	"\tborder-left-style: solid;\n"
	"\tborder-top-color: #000;\n"
	"\tborder-right-color: #000;\n"
	"\tborder-bottom-color: #000;\n"
	"\tborder-left-color: #000;\n"
	"\t}\n"
	"\n";

static char Pgweb_style_Item_2[] =
	"tr.itemContent {\n"
	"\t  background-color:#D0D0D0;\n"
	"\t  height:25;\n"
	"\t  text-align: left;\n"
	"\t  font-family: times;\n"
	"\t}\n"
	"\n"
	"body{\n"
	"\ttext-align: left;\n"
	"}\n";


static rpItem Pgweb_style_Items[] = { 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgweb_style_Item_1 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgweb_style_Item_2 }, 
	{ eRpItemType_LastItemInList } 
};


rpObjectDescription Pgweb_style = {
	"/html/web_style.css",
	Pgweb_style_Items,
	(rpObjectExtensionPtr) 0,
	(Unsigned32) 0,
	kRpPageAccess_Unprotected,
	eRpDataTypeCss,
	eRpObjectTypeDynamic
};


/* ************************************** */
/* *    Built from "html\Hlogo.html"    * */
/* ************************************** */

extern rpObjectDescription PgHlogo;

static char PgHlogo_Item_1[] =
	C_oHTML "<script>parent.Timer_reset=1;</script>\n"
	C_oMETA " http-equiv=\"Content-Type\"" C_CONTENT "\"text/html; "
	"charset=iso-8859-1\">\n"
	C_oBODY " background=\"../Images/BackGround/LogoBg.gif\">\n"
	"\n"
	"<script>\n"
	"function DisplayLogo()\n"
	"{\n"
	"\tdocument.write(\"" C_oP "<a href=\'http://www.dlink.com.tw\' "
	"target=\'_blank\'><img src=\'../Images/BackGround/Logo_dlink.gif\'"
	C_WIDTH "143" C_HEIGHT "143" C_BORDER "0 "
	"alt=\'http://www.dlink.com.tw\'>" C_xANCHOR C_xP "\");\n"
	"}\n"
	"</script>\n"
	"\n"
	"\n"
	"\n"
	"<script>DisplayLogo()</script>\n"
	"\n"
	C_xBODY_xHTML;


static rpItem PgHlogo_Items[] = { 
	{ eRpItemType_DataZeroTerminated, (void*)&PgHlogo_Item_1 }, 
	{ eRpItemType_LastItemInList } 
};


rpObjectDescription PgHlogo = {
	"/html/Hlogo.html",
	PgHlogo_Items,
	(rpObjectExtensionPtr) 0,
	(Unsigned32) 0,
	kRpPageAccess_Unprotected,
	eRpDataTypeHtml,
	eRpObjectTypeDynamic
};


/* ************************************** */
/* *    Built from "html\Hmain.html"    * */
/* ************************************** */

extern rpObjectDescription PgHmain;

static char PgHmain_Item_1[] =
	C_oHTML_oHEAD_oTITLE "\n";

extern char *sWeb_Main_DeviceName_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpTextDisplayItem PgHmain_Item_2 = {
	(void*)sWeb_Main_DeviceName_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgHmain_Item_3[] =
	C_xTITLE C_xHEAD "<script>\n"
	"function err()\n"
	"{\n"
	"\tunauth();\n"
	"\treturn true;\n"
	"}\n"
	"\n"
	"function unauth()\n"
	"{\n"
	"\talert(\"Unauthorized action!\");\n"
	"\tlocation.href=\"/\";\n"
	"}\n"
	"\n"
	"function checkRH()\n"
	"{\n"
	"\tif(location.href.indexOf(\"/RH0\")!=-1)\n"
	"\t{\n"
	"\t\tif(parent.parent.location.href.indexOf(\"/Hmain_Member\")==-1)\n"
	"\t\t\tunauth();\n"
	"\t}\n"
	"}\n"
	"\n"
	"checkRH();\n"
	"</script>\n"
	"<FRAMESET Cols=\"240,*\">\n"
	"<FRAMESET" C_BORDER "0 Rows=\"*,0\">\n";

extern char *sWeb_Menu_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgHmain_Item_4 = {
	"1",
	(void*)sWeb_Menu_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII,
	20
};

static char PgHmain_Item_5[] =
	"<FRAME src=\"./hidden.htm?0\"" C_NAME "\"h\">\n"
	C_xFRAMESET "<FRAME src=\"./top.htm\"" C_NAME "\"r\">\n"
	"<NOFRAMES></NOFRAMES>\n"
	C_xFRAMESET
	C_xHTML;


static rpItem PgHmain_Items[] = { 
	{ eRpItemType_DataZeroTerminated, (void*)&PgHmain_Item_1 }, 
	{ eRpItemType_DisplayText, (void*)&PgHmain_Item_2 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgHmain_Item_3 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgHmain_Item_4 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgHmain_Item_5 }, 
	{ eRpItemType_LastItemInList } 
};


rpObjectDescription PgHmain = {
	"/html/Hmain.html",
	PgHmain_Items,
	(rpObjectExtensionPtr) 0,
	(Unsigned32) 0,
	kRpPageAccess_Realm7,
	eRpDataTypeHtml,
	eRpObjectTypeDynamic
};


/* ****************************************** */
/* *    Built from "html\MntLogout.html"    * */
/* ****************************************** */

extern rpObjectDescription PgMntLogout;

static char PgMntLogout_Item_1[] =
	C_oHTML "<script>parent.Timer_reset=1; </script>\n"
	"\n"
	C_oHEAD "\n"
	C_oTITLE "Web Logut Setup" C_xTITLE C_oMETA " "
	"http-equiv=\"Content-Type\"" C_CONTENT "\"text/html; "
	"charset=iso-8859-1\">\n"
	"<LINK REL=stylesheet TYPE=\"text/css\" HREF=\"web_style.css\">\n"
	C_xHEAD;

static char PgMntLogout_Item_2[] =
	C_oBODY ">\n"
	"    ";

static char PgMntLogout_Item_3[] =
	" target=\"_top\"";

static char PgMntLogout_Item_5[] =
	"  " C_oTABLE_BORDER "\"0\"" C_WIDTH "\"500\"" C_CELLSPACING "\"1\">\n"
	"    " C_oTR "\n"
	"      <td class=pageTitle" C_WIDTH "\"100%\">Logout" C_xTD C_xTR
	C_oTR_ALIGN_CENTER ">\n"
	"      <td class=itemContent" C_WIDTH "\"100%\">" C_oB "\n"
	"      \t Are you sure you want to logout of the Web Manager?\n"
	"      \t " C_oP "If yes, just click  the \"Logout\" button and return "
	"to the Main page.\n"
	"         " C_xB "\n"
	"      " C_xTD C_xTR C_oTR_ALIGN_CENTER ">\n"
	"      <td" C_WIDTH "\"100%\"" C_COLSPAN "\"2\">" C_oDIV_
	C_ALIGN_RIGHT ">"
	C_oP;

static rpButtonFormItem PgMntLogout_Item_6 = {
	"Logout",
	(char *) 0
};

static char PgMntLogout_Item_7[] =
	"      " C_xTD C_xTR C_xTABLE C_xFORM
	C_xBODY_xHTML;



static rpItem PgMntLogout_Form_1_Items[] = { 
	{ eRpItemType_FormSubmit, (void*)&PgMntLogout_Item_6 }, 
	{ eRpItemType_LastItemInList } 
};

extern void MainTainLogoutApply(void *theTaskDataPtr, Signed16Ptr theIndexValuesPtr);
static rpObjectExtension PgMntLogout_Form_1_ObjectExtension = {
	MainTainLogoutApply,
	&PgMntLogout,
	(rpObjectDescriptionPtr) 0,
	0,
	kRpObjFlags_None,
	(char *) PgMntLogout_Item_3
};

rpObjectDescription PgMntLogout_Form_1 = {
	"/Forms/FormWebLogout",
	PgMntLogout_Form_1_Items,
	&PgMntLogout_Form_1_ObjectExtension,
	(Unsigned32) 0,
	kRpPageAccess_Realm7,
	eRpDataTypeForm,
	eRpObjectTypeDynamic
};

static rpItem PgMntLogout_Items[] = { 
	{ eRpItemType_DataZeroTerminated, (void*)&PgMntLogout_Item_1 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgMntLogout_Item_2 }, 
	{ eRpItemType_FormHeader, (void*)&PgMntLogout_Form_1 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgMntLogout_Item_5 }, 
	{ eRpItemType_FormSubmit, (void*)&PgMntLogout_Item_6 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgMntLogout_Item_7 }, 
	{ eRpItemType_LastItemInList } 
};


rpObjectDescription PgMntLogout = {
	"/html/MntLogout.html",
	PgMntLogout_Items,
	(rpObjectExtensionPtr) 0,
	(Unsigned32) 0,
	kRpPageAccess_Realm7,
	eRpDataTypeHtml,
	eRpObjectTypeDynamic
};


/* ********************************************** */
/* *    Built from "html\MessageWindow.html"    * */
/* ********************************************** */

extern rpObjectDescription PgMessageWindow;

static char PgMessageWindow_Item_1[] =
	C_oHTML "<script>\n"
	"function CheckCloseWindow()\n"
	"{\n"
	"\tif (opener.message_window_opened==null)\n"
	"\t\twindow.close();\n"
	"\tif (opener.message_window_opened==0)\n"
	"\t\twindow.close();\n"
	"\tsetTimeout(\'CheckCloseWindow()\', 500);\n"
	"}\n"
	"setTimeout(\'CheckCloseWindow()\', 500);\n"
	"</script>\n"
	C_oCENTER "\n"
	"Please Wait . . .\n"
	C_xCENTER
	C_xHTML;


static rpItem PgMessageWindow_Items[] = { 
	{ eRpItemType_DataZeroTerminated, (void*)&PgMessageWindow_Item_1 }, 
	{ eRpItemType_LastItemInList } 
};


rpObjectDescription PgMessageWindow = {
	"/html/MessageWindow.html",
	PgMessageWindow_Items,
	(rpObjectExtensionPtr) 0,
	(Unsigned32) 0,
	kRpPageAccess_Unprotected,
	eRpDataTypeHtml,
	eRpObjectTypeDynamic
};


/* ************************************************ */
/* *    Built from "html\ProtectedObject.html"    * */
/* ************************************************ */

extern rpObjectDescription PgProtectedObject;

static char PgProtectedObject_Item_1[] =
	C_oHTML_oHEAD_oTITLE C_S_ProtectedObject C_xTITLE_xHEAD_oBODY C_oH1
	C_S_ProtectedObject C_xH1 C_S_ThisObjectOnThe "server" C_S_IsProtected
	".\n"
	C_xBODY;

extern char *html_ErrorMessage_Get(void);
static rpTextDisplayItem PgProtectedObject_Item_2 = {
	(void*)html_ErrorMessage_Get,
	eRpVarType_Function,
	eRpTextType_ASCII,
	20
};

extern char *html_MyProtectedObjectProcess(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpTextDisplayItem PgProtectedObject_Item_3 = {
	(void*)html_MyProtectedObjectProcess,
	eRpVarType_Complex,
	eRpTextType_ASCII,
	0
};

static char PgProtectedObject_Item_4[] =
	C_xHTML;


static rpItem PgProtectedObject_Items[] = { 
	{ eRpItemType_DataZeroTerminated, (void*)&PgProtectedObject_Item_1 }, 
	{ eRpItemType_DisplayText, (void*)&PgProtectedObject_Item_2 }, 
	{ eRpItemType_DisplayText, (void*)&PgProtectedObject_Item_3 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgProtectedObject_Item_4 }, 
	{ eRpItemType_LastItemInList } 
};


rpObjectDescription PgProtectedObject = {
	"/html/ProtectedObject.html",
	PgProtectedObject_Items,
	(rpObjectExtensionPtr) 0,
	(Unsigned32) 0,
	kRpPageAccess_Unprotected,
	eRpDataTypeHtml,
	eRpObjectTypeDynamic
};


/* ************************************ */
/* *    Built from "html\menu.htm"    * */
/* ************************************ */

extern rpObjectDescription Pgmenu;

static char Pgmenu_Item_1[] =
	C_oHTML "<head>\n";

extern char *sWeb_Menu_Language_Set(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpTextDisplayItem Pgmenu_Item_2 = {
	(void*)sWeb_Menu_Language_Set,
	eRpVarType_Complex,
	eRpTextType_ASCII,
	20
};

static char Pgmenu_Item_3[] =
	C_oMETA " http-equiv=\"Content-Type\"" C_CONTENT "\"text/html; ";

extern char *sWeb_Menu_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem Pgmenu_Item_4 = {
	"69",
	(void*)sWeb_Menu_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char Pgmenu_Item_5[] =
	"\">\n"
	C_oTITLE "Menu" C_xTITLE C_xHEAD "<SCRIPT language=\"Javascript\">\n"
	"\n"
	"<!--\n"
	"function javamagic(layerName) \n"
	"{\n"
	"\tvar targetId;\n"
	"\ttargetId = layerName + \"d\";\n"
	"\tif(document.all) \n"
	"\t{\n"
	"\t\tif(document.all[layerName].style.display == \'none\') \n"
	"\t\t{\n"
	"\t\t\tdocument.all[layerName].style.display = \'block\';\n"
	"\t\t\tdocument.all[targetId].src = \'../images/open.png\';\n"
	"\t\t} \n"
	"\t\telse \n"
	"\t\t{\n"
	"\t\t\tdocument.all[layerName].style.display = \'none\';\n"
	"\t\t\tdocument.all[targetId].src = \'../images/close.png\';\n"
	"\t\t}\n"
	"\t}\n"
	"\telse if(document.layers) \n"
	"\t{\n"
	"\t\tif (document.layers[layerName].display == \'none\') \n"
	"\t\t{\n"
	"\t\t\tdocument.layers[layerName].display = \'block\';\n"
	"\t\t\tdocument.layers[targetId].src = \'../images/open.png\';\n"
	"\t\t} \n"
	"\t\telse \n"
	"\t\t{\n"
	"\t\t\tdocument.layers[layerName].display = \'none\';\n"
	"\t\t\tdocument.layers[targetId].src = \'../images/close.png\';\n"
	"\t\t}\n"
	"\t}\n";

static char Pgmenu_Item_6[] =
	"\telse if(document.getElementById) \n"
	"\t{\n"
	"\t\tif (document.getElementById(layerName).style.display == \'none\') "
	"\n"
	"\t\t{\n"
	"\t\t\tdocument.getElementById(layerName).style.display = \'block\';\n"
	"\t\t\tdocument.getElementById(targetId).src = \'../images/open.png\';\n"
	"\t\t} \n"
	"\t\telse \n"
	"\t\t{\n"
	"\t\t\tdocument.getElementById(layerName).style.display = \'none\';\n"
	"\t\t\tdocument.getElementById(targetId).src = \'../images/close.png\';"
	"\n"
	"\t\t}\n"
	"\t}\n"
	"}\n"
	"\n"
	"// -->\n"
	"</SCRIPT>\n"
	"\n"
	C_oBODY " text=\"#000000\" bgcolor=\"ffffff\" link=\"0000CD\">\n"
	"\n"
	"<IMG" C_HEIGHT "\"35\"" C_ALT "mars\" src=\"../images/logo_mars.jpg\""
	C_WIDTH "\"101\">" C_oP "<TABLE" C_WIDTH "205" C_BORDER "\"0\""
	C_CELLPADDING "\"1\"" C_BGCOLOR "ffffff\">\n"
	C_oTR "\n"
	"<TD nowrap>" C_oB "\n";

static char Pgmenu_Item_9[] =
	"<!-- DeviceModel_A1013 -->\n"
	"A1013" C_NBSP "\n";

static rpItem Pgmenu_Item_8[] = { 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgmenu_Item_9 }, 
	{ eRpItemType_LastItemInList } 
};

static char Pgmenu_Item_11[] =
	"<!-- DeviceModel_Other -->\n"
	"Other" C_NBSP "\n";

static rpItem Pgmenu_Item_10[] = { 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgmenu_Item_11 }, 
	{ eRpItemType_LastItemInList } 
};

static rpItem Pgmenu_Item_7_Group[] = { 
	{ eRpItemType_ItemGroup, (void*)&Pgmenu_Item_8 }, 
	{ eRpItemType_ItemGroup, (void*)&Pgmenu_Item_10 }, 
	{ eRpItemType_LastItemInList } 
};

extern Unsigned8 sWeb_Main_DeviceModel_Get(void *theTaskDataPtr, Signed16Ptr theIndexValuesPtr);
static rpDynamicDisplayItem Pgmenu_Item_7 = {
	(void*)sWeb_Main_DeviceModel_Get,
	eRpVarType_Complex,
	2,
	Pgmenu_Item_7_Group
};

extern char *sWeb_Menu_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem Pgmenu_Item_12 = {
	"3",
	(void*)sWeb_Menu_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char Pgmenu_Item_13[] =
	C_xB C_xTD C_xTR C_xTABLE C_oHR C_oANCHOR_HREF "top.htm\" target=\"r\">";

extern char *sWeb_Menu_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem Pgmenu_Item_14 = {
	"4",
	(void*)sWeb_Menu_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char Pgmenu_Item_15[] =
	C_xANCHOR C_oHR C_oTABLE_BORDER "\"1\" bordercolor=\"#eae7ef\""
	C_CELLPADDING "\"3\"" C_BGCOLOR "ffffff\"" C_WIDTH "\"230\">\n"
	C_oTR "\n"
	C_oTD C_oDIV_ " ID=\"OutA1\">\n"
	"  " C_oANCHOR_HREF "javascript:javamagic(\'OutA2\');\" class=\"icon\">"
	C_oIMG_SRC "../images/close.png\" ID=\"OutA2d\" CLASS=\"Outline\" "
	"STYLE=\"cursor: hand\"" C_WIDTH "\"16\"" C_HEIGHT "\"16\"" C_BORDER
	"\"0\">" C_xANCHOR
	C_NBSP;

extern char *sWeb_Menu_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem Pgmenu_Item_16 = {
	"5",
	(void*)sWeb_Menu_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char Pgmenu_Item_17[] =
	C_oBR C_oDIV_ " ID=\"OutA2\" STYLE=\"display:None\">\n"
	"  " C_oFONT_SIZE "\"-1\">\n"
	"       " C_S_NBSP4 C_oANCHOR_HREF "sysinfo.htm\" target=\"r\">";

extern char *sWeb_Menu_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem Pgmenu_Item_18 = {
	"6",
	(void*)sWeb_Menu_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char Pgmenu_Item_19[] =
	C_xANCHOR C_oBR C_S_NBSP4 C_oANCHOR_HREF "vdslstatus.htm\" "
	"target=\"r\">";

extern char *sWeb_Menu_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem Pgmenu_Item_20 = {
	"7",
	(void*)sWeb_Menu_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char Pgmenu_Item_21[] =
	C_xANCHOR C_oBR C_S_NBSP4 C_oANCHOR_HREF "btm.htm?1\" target=\"r\">";

extern char *sWeb_Menu_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem Pgmenu_Item_22 = {
	"8",
	(void*)sWeb_Menu_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char Pgmenu_Item_23[] =
	C_xANCHOR C_oP C_S_NBSP4 "\n"
	"  " C_xFONT "\n"
	"  " C_xDIV C_xDIV C_oDIV_ " ID=\"OutB1\">\n"
	"  " C_oANCHOR_HREF "javascript:javamagic(\'OutB2\');\" class=\"icon\">"
	C_oIMG_SRC "../images/close.png\" ID=\"OutB2d\" CLASS=\"Outline\" "
	"STYLE=\"cursor: hand\"" C_WIDTH "\"16\"" C_HEIGHT "\"16\"" C_BORDER
	"\"0\">" C_xANCHOR
	C_NBSP;

extern char *sWeb_Menu_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem Pgmenu_Item_24 = {
	"20",
	(void*)sWeb_Menu_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char Pgmenu_Item_25[] =
	C_oBR C_oDIV_ " ID=\"OutB2\" STYLE=\"display:None\">\n"
	"  " C_oFONT_SIZE "\"-1\">\n"
	"       " C_S_NBSP4 C_oANCHOR_HREF "RecSetting.html?0\" target=\"r\">";

extern char *sWeb_Menu_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem Pgmenu_Item_26 = {
	"21",
	(void*)sWeb_Menu_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char Pgmenu_Item_27[] =
	C_xANCHOR C_oBR C_S_NBSP4 C_oANCHOR_HREF "DisplaySetting.html\" "
	"target=\"r\">";

extern char *sWeb_Menu_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem Pgmenu_Item_28 = {
	"22",
	(void*)sWeb_Menu_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char Pgmenu_Item_29[] =
	C_xANCHOR C_oBR C_S_NBSP4 C_oANCHOR_HREF "ImageSetting.html\" "
	"target=\"r\">";

extern char *sWeb_Menu_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem Pgmenu_Item_30 = {
	"23",
	(void*)sWeb_Menu_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char Pgmenu_Item_31[] =
	C_xANCHOR C_oBR C_S_NBSP4 C_oANCHOR_HREF "log_level.htm\" target=\"r\">";

extern char *sWeb_Menu_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem Pgmenu_Item_32 = {
	"24",
	(void*)sWeb_Menu_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char Pgmenu_Item_33[] =
	C_xANCHOR C_oBR C_S_NBSP4 C_oANCHOR_HREF "snmp.htm\" target=\"r\">";

extern char *sWeb_Menu_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem Pgmenu_Item_34 = {
	"25",
	(void*)sWeb_Menu_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char Pgmenu_Item_35[] =
	C_xANCHOR C_oBR C_S_NBSP4 C_oANCHOR_HREF "mail.htm\" target=\"r\">";

extern char *sWeb_Menu_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem Pgmenu_Item_36 = {
	"26",
	(void*)sWeb_Menu_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char Pgmenu_Item_37[] =
	C_xANCHOR C_oBR C_S_NBSP4 C_oANCHOR_HREF "8021x.htm\" target=\"r\">";

extern char *sWeb_Menu_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem Pgmenu_Item_38 = {
	"27",
	(void*)sWeb_Menu_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char Pgmenu_Item_39[] =
	C_xANCHOR C_oBR C_S_NBSP4 C_oANCHOR_HREF "cluster.htm\" target=\"r\">";

extern char *sWeb_Menu_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem Pgmenu_Item_40 = {
	"28",
	(void*)sWeb_Menu_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char Pgmenu_Item_41[] =
	C_xANCHOR C_oP C_S_NBSP4 "\n"
	"  " C_xFONT "\n"
	"  " C_xDIV C_xDIV C_oDIV_ " ID=\"OutC1\">\n"
	"  " C_oANCHOR_HREF "javascript:javamagic(\'OutC2\');\" class=\"icon\">"
	C_oIMG_SRC "../images/close.png\" ID=\"OutC2d\" CLASS=\"Outline\" "
	"STYLE=\"cursor: hand\"" C_WIDTH "\"16\"" C_HEIGHT "\"16\"" C_BORDER
	"\"0\">" C_xANCHOR
	C_NBSP;

extern char *sWeb_Menu_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem Pgmenu_Item_42 = {
	"29",
	(void*)sWeb_Menu_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char Pgmenu_Item_43[] =
	C_oBR C_oDIV_ " ID=\"OutC2\" STYLE=\"display:None\">\n"
	"  " C_oFONT_SIZE "\"-1\">\n"
	"       " C_S_NBSP4 C_oANCHOR_HREF "ether.htm\" target=\"r\">";

extern char *sWeb_Menu_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem Pgmenu_Item_44 = {
	"30",
	(void*)sWeb_Menu_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char Pgmenu_Item_45[] =
	C_xANCHOR C_oBR C_oP C_S_NBSP4 "\n"
	"  " C_xFONT "\n"
	"  " C_xDIV C_xDIV C_oDIV_ " ID=\"OutI1\">\n"
	"  " C_oANCHOR_HREF "javascript:javamagic(\'OutI2\');\" class=\"icon\">"
	C_oIMG_SRC "../images/close.png\" ID=\"OutI2d\" CLASS=\"Outline\" "
	"STYLE=\"cursor: hand\"" C_WIDTH "\"16\"" C_HEIGHT "\"16\"" C_BORDER
	"\"0\">" C_xANCHOR C_NBSP "Language" C_oBR C_oDIV_ " ID=\"OutI2\" "
	"STYLE=\"display:None\">\n"
	"  <!-- <INPUT type=\"hidden\" value=\"0\"" C_NAME "\"usedLanguage\"> "
	"-->\n"
	"  " C_oFONT_SIZE "\"-1\">" C_S_NBSP4 "\n";

extern char *sWeb_Menu_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem Pgmenu_Item_46 = {
	"2",
	(void*)sWeb_Menu_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII,
	20
};

static char Pgmenu_Item_47[] =
	"  " C_xFONT "\n"
	"  " C_xFONT "\n"
	"  " C_xDIV C_xDIV C_oBR C_xTD C_xTR C_xTABLE C_xTABLE "<SCRIPT "
	"LANGUAGE=\"JavaScript\">\n"
	"function clickHandler() {\n"
	"  var targetId, srcElement, targetElement;\n"
	"  srcElement = window.event.srcElement;\n"
	"  if (srcElement.className == \"icon\") {\n"
	"\t targetId = srcElement.id + \"d\";\n"
	"     targetElement = document.all[targetId];\n"
	"     if (targetElement.style.display == \"none\") {\n"
	"        targetElement.style.display = \"\";\n"
	"        srcElement.src = \"./gif/open.gif\";\n"
	"     } else {\n"
	"        targetElement.style.display = \"none\";\n"
	"        srcElement.src = \"./gif/close.gif\";\n"
	"     }\n"
	"  }\n"
	"}\n"
	"\n"
	"document.onclick = clickHandler;\n"
	"\n"
	"\n"
	"function onMasterClusterSelect(ctrl)\n"
	"{\n"
	"\topenRH(\"../RH\"+ctrl.options[ctrl.selectedIndex].value+\"/\", "
	"ctrl.options[ctrl.selectedIndex].value);\n"
	"\tctrl.selectedIndex=0;\n"
	"}\n"
	"\n";

static char Pgmenu_Item_48[] =
	"function onMemberClusterSelect(ctrl)\n"
	"{\n"
	"\tif (ctrl.selectedIndex==0)//Select Manager\n"
	"\t{\n"
	"\t\tparent.parent.location.href=\"../../\";\n"
	"\t}\n"
	"}\n"
	"\n"
	"function openRH(url, memberid)\n"
	"{\n"
	"  //open(url);\n"
	"  //parent.location.href=url;\n"
	"  parent.location.href=\"./Hmain_Member.htm?\"+memberid;\n"
	"}\n"
	"\n"
	"var TimeID=window.setTimeout(\"initItems()\",100);\n"
	"function initItems()\n"
	"{\n"
	"window.clearTimeout(TimeID);\n"
	"//parent.b.location.href=\"./button.htm\";\n"
	"if (parent.r.current_language!=null)\n"
	"{\n"
	"\tvar rUrl=parent.r.location.href;\n"
	"\tif (rUrl!=\"\")\n"
	"\t{\n"
	"\t\tvar menu_url_string=self.location.href;\n"
	"\t\tvar "
	"menu_language=menu_url_string.charAt(menu_url_string.length-1);\n"
	"\t\tif (menu_language!=parent.r.current_language)\n"
	"\t\t\tparent.r.location.href=rUrl;\n"
	"\t}\n"
	"}\n"
	"}\n"
	"</SCRIPT>\n"
	"\n"
	"\n"
	"\n"
	C_xBODY_xHTML;


static rpItem Pgmenu_Items[] = { 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgmenu_Item_1 }, 
	{ eRpItemType_DisplayText, (void*)&Pgmenu_Item_2 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgmenu_Item_3 }, 
	{ eRpItemType_NamedDisplayText, (void*)&Pgmenu_Item_4 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgmenu_Item_5 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgmenu_Item_6 }, 
	{ eRpItemType_DynamicDisplay, (void*)&Pgmenu_Item_7 }, 
	{ eRpItemType_NamedDisplayText, (void*)&Pgmenu_Item_12 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgmenu_Item_13 }, 
	{ eRpItemType_NamedDisplayText, (void*)&Pgmenu_Item_14 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgmenu_Item_15 }, 
	{ eRpItemType_NamedDisplayText, (void*)&Pgmenu_Item_16 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgmenu_Item_17 }, 
	{ eRpItemType_NamedDisplayText, (void*)&Pgmenu_Item_18 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgmenu_Item_19 }, 
	{ eRpItemType_NamedDisplayText, (void*)&Pgmenu_Item_20 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgmenu_Item_21 }, 
	{ eRpItemType_NamedDisplayText, (void*)&Pgmenu_Item_22 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgmenu_Item_23 }, 
	{ eRpItemType_NamedDisplayText, (void*)&Pgmenu_Item_24 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgmenu_Item_25 }, 
	{ eRpItemType_NamedDisplayText, (void*)&Pgmenu_Item_26 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgmenu_Item_27 }, 
	{ eRpItemType_NamedDisplayText, (void*)&Pgmenu_Item_28 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgmenu_Item_29 }, 
	{ eRpItemType_NamedDisplayText, (void*)&Pgmenu_Item_30 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgmenu_Item_31 }, 
	{ eRpItemType_NamedDisplayText, (void*)&Pgmenu_Item_32 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgmenu_Item_33 }, 
	{ eRpItemType_NamedDisplayText, (void*)&Pgmenu_Item_34 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgmenu_Item_35 }, 
	{ eRpItemType_NamedDisplayText, (void*)&Pgmenu_Item_36 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgmenu_Item_37 }, 
	{ eRpItemType_NamedDisplayText, (void*)&Pgmenu_Item_38 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgmenu_Item_39 }, 
	{ eRpItemType_NamedDisplayText, (void*)&Pgmenu_Item_40 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgmenu_Item_41 }, 
	{ eRpItemType_NamedDisplayText, (void*)&Pgmenu_Item_42 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgmenu_Item_43 }, 
	{ eRpItemType_NamedDisplayText, (void*)&Pgmenu_Item_44 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgmenu_Item_45 }, 
	{ eRpItemType_NamedDisplayText, (void*)&Pgmenu_Item_46 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgmenu_Item_47 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&Pgmenu_Item_48 }, 
	{ eRpItemType_LastItemInList } 
};


rpObjectDescription Pgmenu = {
	"/html/menu.htm",
	Pgmenu_Items,
	(rpObjectExtensionPtr) 0,
	(Unsigned32) 0,
	kRpPageAccess_Realm7,
	eRpDataTypeHtml,
	eRpObjectTypeDynamic
};


/* ******************************************* */
/* *    Built from "html\RecSetting.html"    * */
/* ******************************************* */

extern rpObjectDescription PgRecSetting;

static char PgRecSetting_Item_1[] =
	C_oHTML C_oHEAD "\n"
	C_oMETA " http-equiv=\"Content-Type\"" C_CONTENT "\"text/html; ";

extern char *sWeb_Menu_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgRecSetting_Item_2 = {
	"69",
	(void*)sWeb_Menu_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgRecSetting_Item_3[] =
	"\">\n"
	C_oTITLE "Rec Setting" C_xTITLE "<LINK REL=stylesheet TYPE=\"text/css\""
	" HREF=\"web_style.css\">\n"
	C_xHEAD "<script src=\"jsComm.js\"></script>\n"
	"<SCRIPT language=\"javascript\">\n";

extern char *sWeb_RecSet_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpTextDisplayItem PgRecSetting_Item_4 = {
	(void*)sWeb_RecSet_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII,
	20
};

static char PgRecSetting_Item_5[] =
	"\n"
	"function formCheck(f)\n"
	"{\n"
	"\tif(!checkAccessRight())\n"
	"\t\treturn false;\n"
	"\tf.sWeb_RecSet_ChannelSel.disabled=true;\n"
	"\treturn true;\n"
	"}\n"
	"\n"
	"function ReloadByChannel()\n"
	"{\n"
	"\tvar s=document.forms[0];\n"
	"\tlocation.href=\"RecSetting.html?\"+s.sWeb_RecSet_ChannelSel.value;\n"
	"}\n"
	"\n"
	"</SCRIPT>\n"
	"\n"
	C_oBODY " text=\"#000000\" bgcolor=\"ffffff\" link=\"#0000CD\">\n"
	"\n";

static char PgRecSetting_Item_7[] =
	"\n"
	"\n"
	"<LAYER style=\"position : absolute; top : -50px\">\n"
	C_oDIV_ " style=\"position : absolute; top : -50px\">\n"
	"<!-- <INPUT type=\"radio\" style=\"background : #000000\"> -->\n"
	C_xDIV "</LAYER>\n"
	C_oBR C_oP C_oFONT_SIZE "\"3\">"
	C_oB;

extern char *sWeb_RecSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgRecSetting_Item_8 = {
	"1",
	(void*)sWeb_RecSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgRecSetting_Item_9[] =
	C_xB C_xFONT "\n"
	C_oHR C_oBR C_oTABLE_CELLPADDING "\"0\"" C_BORDER "\"0\" "
	"class=\"pageTitle\">\n"
	" <TR" C_BGCOLOR "B4CDCD\"" C_ALIGN_CENTER ">\n"
	"  <TD nowrap class=pageTitle>";

extern char *sWeb_RecSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgRecSetting_Item_10 = {
	"2",
	(void*)sWeb_RecSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgRecSetting_Item_11[] =
	C_xTD "<TD nowrap" C_COLSPAN "2 class=pageTitle >";

extern char *sWeb_RecSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgRecSetting_Item_12 = {
	"3",
	(void*)sWeb_RecSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgRecSetting_Item_13[] =
	C_xTD C_xTR C_oTR_ALIGN_CENTER ">\n"
	"  <TD nowrap class=\"itemTitle\">";

extern char *sWeb_RecSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgRecSetting_Item_14 = {
	"21",
	(void*)sWeb_RecSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgRecSetting_Item_15[] =
	C_xTD "<TD nowrap" C_COLSPAN "2 class=\"itemTitle\">\n";

extern char *sWeb_RecSet_Channel_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
extern void sWeb_RecSet_Channel_Set(void *theTaskDataPtr, char *theValuePtr,
		char *theNamePtr, Signed16Ptr theIndexValuesPtr);
static rpTextFormItem PgRecSetting_Item_16 = {
	"sWeb_RecSet_Channel",
	(void*)sWeb_RecSet_Channel_Get,
	(void*)sWeb_RecSet_Channel_Set,
	eRpVarType_Complex,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	1,
	18,
	(char *) 0
};

static char PgRecSetting_Item_17[] =
	"  ";

extern char *sWeb_RecSet_ChannelSel_Select_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgRecSetting_Item_18 = {
	"sWeb_RecSet_ChannelSel",
	(void*)sWeb_RecSet_ChannelSel_Select_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgRecSetting_Item_19[] =
	C_xTD C_xTR C_oTR_ALIGN_CENTER ">\n"
	"  <TD nowrap" C_ROWSPAN "8 class=\"itemTitle\">";

extern char *sWeb_RecSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgRecSetting_Item_20 = {
	"20",
	(void*)sWeb_RecSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgRecSetting_Item_21[] =
	C_xTD "<TD nowrap" C_ROWSPAN "2 class=\"itemTitle\">";

extern char *sWeb_RecSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgRecSetting_Item_22 = {
	"4",
	(void*)sWeb_RecSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgRecSetting_Item_23[] =
	C_xTD "<TD nowrap class=\"itemContent\">\n"
	"   ";

extern rpOneOfSeveral sWeb_RecSet_Manual_Radio_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
extern void sWeb_RecSet_Manual_Radio_Set(void *theTaskDataPtr, rpOneOfSeveral theValue,
		char *theNamePtr, Signed16Ptr theIndexValuesPtr);
static rpRadioGroupInfo PgRecSetting_RadioGroup_1 = {
	"sWeb_RecSet_Manual_Radio",
	(void*)sWeb_RecSet_Manual_Radio_Get,
	(void*)sWeb_RecSet_Manual_Radio_Set,
	eRpVarType_Complex,
	eRpVarType_Complex
};

static char PgRecSetting_Item_24[] =
	"   ";

static rpRadioButtonFormItem PgRecSetting_Item_25 = {
	&PgRecSetting_RadioGroup_1,
	"0",
	0,
	(char *) 0
};

extern char *sWeb_RecSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgRecSetting_Item_26 = {
	"9",
	(void*)sWeb_RecSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgRecSetting_Item_27[] =
	C_NBSP C_NBSP "\n"
	"  " C_xTD C_xTR "<TR >\n"
	"   <TD nowrap class=\"itemContent\">\n"
	"   ";

static rpRadioButtonFormItem PgRecSetting_Item_28 = {
	&PgRecSetting_RadioGroup_1,
	"1",
	1,
	(char *) 0
};

extern char *sWeb_RecSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgRecSetting_Item_29 = {
	"10",
	(void*)sWeb_RecSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgRecSetting_Item_30[] =
	C_NBSP C_NBSP "\n"
	"  " C_xTD C_xTR C_oTR_ALIGN_CENTER ">\n"
	"  <TD nowrap" C_ROWSPAN "2 class=\"itemTitle\">";

extern char *sWeb_RecSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgRecSetting_Item_31 = {
	"5",
	(void*)sWeb_RecSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgRecSetting_Item_32[] =
	C_xTD "<TD nowrap class=\"itemContent\">\n"
	"   ";

extern rpOneOfSeveral sWeb_RecSet_Schedule_Radio_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
extern void sWeb_RecSet_Schedule_Radio_Set(void *theTaskDataPtr, rpOneOfSeveral theValue,
		char *theNamePtr, Signed16Ptr theIndexValuesPtr);
static rpRadioGroupInfo PgRecSetting_RadioGroup_2 = {
	"sWeb_RecSet_Schedule_Radio",
	(void*)sWeb_RecSet_Schedule_Radio_Get,
	(void*)sWeb_RecSet_Schedule_Radio_Set,
	eRpVarType_Complex,
	eRpVarType_Complex
};

static char PgRecSetting_Item_33[] =
	"   ";

static rpRadioButtonFormItem PgRecSetting_Item_34 = {
	&PgRecSetting_RadioGroup_2,
	"0",
	0,
	(char *) 0
};

extern char *sWeb_RecSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgRecSetting_Item_35 = {
	"9",
	(void*)sWeb_RecSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgRecSetting_Item_36[] =
	C_NBSP C_NBSP "\n"
	"  " C_xTD C_xTR "<TR >\n"
	"   <TD nowrap class=\"itemContent\">\n"
	"   ";

static rpRadioButtonFormItem PgRecSetting_Item_37 = {
	&PgRecSetting_RadioGroup_2,
	"1",
	1,
	(char *) 0
};

extern char *sWeb_RecSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgRecSetting_Item_38 = {
	"10",
	(void*)sWeb_RecSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgRecSetting_Item_39[] =
	C_NBSP C_NBSP "\n"
	"  " C_xTD C_xTR C_oTR_ALIGN_CENTER ">\n"
	"  <TD nowrap" C_ROWSPAN "2 class=\"itemTitle\">";

extern char *sWeb_RecSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgRecSetting_Item_40 = {
	"6",
	(void*)sWeb_RecSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgRecSetting_Item_41[] =
	C_xTD "<TD nowrap class=\"itemContent\">\n"
	"   ";

extern rpOneOfSeveral sWeb_RecSet_Motion_Radio_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
extern void sWeb_RecSet_Motion_Radio_Set(void *theTaskDataPtr, rpOneOfSeveral theValue,
		char *theNamePtr, Signed16Ptr theIndexValuesPtr);
static rpRadioGroupInfo PgRecSetting_RadioGroup_3 = {
	"sWeb_RecSet_Motion_Radio",
	(void*)sWeb_RecSet_Motion_Radio_Get,
	(void*)sWeb_RecSet_Motion_Radio_Set,
	eRpVarType_Complex,
	eRpVarType_Complex
};

static char PgRecSetting_Item_42[] =
	"   ";

static rpRadioButtonFormItem PgRecSetting_Item_43 = {
	&PgRecSetting_RadioGroup_3,
	"0",
	0,
	(char *) 0
};

extern char *sWeb_RecSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgRecSetting_Item_44 = {
	"9",
	(void*)sWeb_RecSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgRecSetting_Item_45[] =
	C_NBSP C_NBSP "\n"
	"  " C_xTD C_xTR "<TR >\n"
	"   <TD nowrap class=\"itemContent\">\n"
	"   ";

static rpRadioButtonFormItem PgRecSetting_Item_46 = {
	&PgRecSetting_RadioGroup_3,
	"1",
	1,
	(char *) 0
};

extern char *sWeb_RecSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgRecSetting_Item_47 = {
	"10",
	(void*)sWeb_RecSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgRecSetting_Item_48[] =
	C_NBSP C_NBSP "\n"
	"  " C_xTD C_xTR C_oTR_ALIGN_CENTER ">\n"
	"  <TD nowrap" C_ROWSPAN "2 class=\"itemTitle\">";

extern char *sWeb_RecSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgRecSetting_Item_49 = {
	"7",
	(void*)sWeb_RecSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgRecSetting_Item_50[] =
	C_xTD "<TD nowrap class=\"itemContent\">\n"
	"   ";

extern rpOneOfSeveral sWeb_RecSet_Alarm_Radio_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
extern void sWeb_RecSet_Alarm_Radio_Set(void *theTaskDataPtr, rpOneOfSeveral theValue,
		char *theNamePtr, Signed16Ptr theIndexValuesPtr);
static rpRadioGroupInfo PgRecSetting_RadioGroup_4 = {
	"sWeb_RecSet_Alarm_Radio",
	(void*)sWeb_RecSet_Alarm_Radio_Get,
	(void*)sWeb_RecSet_Alarm_Radio_Set,
	eRpVarType_Complex,
	eRpVarType_Complex
};

static char PgRecSetting_Item_51[] =
	"   ";

static rpRadioButtonFormItem PgRecSetting_Item_52 = {
	&PgRecSetting_RadioGroup_4,
	"0",
	0,
	(char *) 0
};

extern char *sWeb_RecSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgRecSetting_Item_53 = {
	"9",
	(void*)sWeb_RecSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgRecSetting_Item_54[] =
	C_NBSP C_NBSP "\n"
	"  " C_xTD C_xTR "<TR >\n"
	"   <TD nowrap class=\"itemContent\">\n"
	"   ";

static rpRadioButtonFormItem PgRecSetting_Item_55 = {
	&PgRecSetting_RadioGroup_4,
	"1",
	1,
	(char *) 0
};

extern char *sWeb_RecSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgRecSetting_Item_56 = {
	"10",
	(void*)sWeb_RecSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgRecSetting_Item_57[] =
	C_NBSP C_NBSP "\n"
	"  " C_xTD C_xTR C_oTR_ALIGN_CENTER ">\n"
	"  <TD nowrap" C_ROWSPAN "3 class=\"itemTitle\">";

extern char *sWeb_RecSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgRecSetting_Item_58 = {
	"8",
	(void*)sWeb_RecSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgRecSetting_Item_59[] =
	C_xTD "<TD nowrap" C_COLSPAN "2 class=\"itemContent\">\n"
	"   ";

extern rpOneOfSeveral sWeb_RecSet_Section_Radio_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
extern void sWeb_RecSet_Section_Radio_Set(void *theTaskDataPtr, rpOneOfSeveral theValue,
		char *theNamePtr, Signed16Ptr theIndexValuesPtr);
static rpRadioGroupInfo PgRecSetting_RadioGroup_5 = {
	"sWeb_RecSet_Section_Radio",
	(void*)sWeb_RecSet_Section_Radio_Get,
	(void*)sWeb_RecSet_Section_Radio_Set,
	eRpVarType_Complex,
	eRpVarType_Complex
};

static char PgRecSetting_Item_60[] =
	"   ";

static rpRadioButtonFormItem PgRecSetting_Item_61 = {
	&PgRecSetting_RadioGroup_5,
	"0",
	0,
	(char *) 0
};

extern char *sWeb_RecSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgRecSetting_Item_62 = {
	"11",
	(void*)sWeb_RecSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgRecSetting_Item_63[] =
	"  " C_xTD C_xTR "<TR >\n"
	"   <TD nowrap" C_COLSPAN "2 class=\"itemContent\">\n"
	"   ";

static rpRadioButtonFormItem PgRecSetting_Item_64 = {
	&PgRecSetting_RadioGroup_5,
	"1",
	1,
	(char *) 0
};

extern char *sWeb_RecSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgRecSetting_Item_65 = {
	"12",
	(void*)sWeb_RecSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgRecSetting_Item_66[] =
	"  " C_xTD C_xTR "<TR >\n"
	"   <TD nowrap" C_COLSPAN "2 class=\"itemContent\">\n"
	"   ";

static rpRadioButtonFormItem PgRecSetting_Item_67 = {
	&PgRecSetting_RadioGroup_5,
	"2",
	2,
	(char *) 0
};

extern char *sWeb_RecSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgRecSetting_Item_68 = {
	"13",
	(void*)sWeb_RecSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgRecSetting_Item_69[] =
	"  " C_xTD C_xTR C_oTR_ALIGN_CENTER ">\n"
	"  <TD nowrap class=\"itemTitle\">";

extern char *sWeb_RecSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgRecSetting_Item_70 = {
	"18",
	(void*)sWeb_RecSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgRecSetting_Item_71[] =
	C_xTD "<TD nowrap" C_COLSPAN "2 class=\"itemContent\">\n"
	"   "
	C_NBSP;

extern char *sWeb_RecSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgRecSetting_Item_72 = {
	"19",
	(void*)sWeb_RecSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgRecSetting_Item_73[] =
	C_NBSP "\n"
	"  " C_xTD C_xTR C_oTR_ALIGN_CENTER ">\n"
	"  <TD nowrap" C_ROWSPAN "3 class=\"itemTitle\">";

extern char *sWeb_RecSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgRecSetting_Item_74 = {
	"14",
	(void*)sWeb_RecSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgRecSetting_Item_75[] =
	C_xTD "<TD nowrap" C_COLSPAN "2 class=\"itemContent\">\n"
	"   ";

extern rpOneOfSeveral sWeb_RecSet_Sensitivity_Radio_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
extern void sWeb_RecSet_Sensitivity_Radio_Set(void *theTaskDataPtr, rpOneOfSeveral theValue,
		char *theNamePtr, Signed16Ptr theIndexValuesPtr);
static rpRadioGroupInfo PgRecSetting_RadioGroup_6 = {
	"sWeb_RecSet_Sensitivity_Radio",
	(void*)sWeb_RecSet_Sensitivity_Radio_Get,
	(void*)sWeb_RecSet_Sensitivity_Radio_Set,
	eRpVarType_Complex,
	eRpVarType_Complex
};

static char PgRecSetting_Item_76[] =
	"   ";

static rpRadioButtonFormItem PgRecSetting_Item_77 = {
	&PgRecSetting_RadioGroup_6,
	"0",
	0,
	(char *) 0
};

extern char *sWeb_RecSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgRecSetting_Item_78 = {
	"15",
	(void*)sWeb_RecSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgRecSetting_Item_79[] =
	"  " C_xTD C_xTR "<TR >\n"
	"   <TD nowrap" C_COLSPAN "2 class=\"itemContent\">\n"
	"   ";

static rpRadioButtonFormItem PgRecSetting_Item_80 = {
	&PgRecSetting_RadioGroup_6,
	"1",
	1,
	(char *) 0
};

extern char *sWeb_RecSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgRecSetting_Item_81 = {
	"16",
	(void*)sWeb_RecSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgRecSetting_Item_82[] =
	"  " C_xTD C_xTR "<TR >\n"
	"   <TD nowrap" C_COLSPAN "2 class=\"itemContent\">\n"
	"   ";

static rpRadioButtonFormItem PgRecSetting_Item_83 = {
	&PgRecSetting_RadioGroup_6,
	"2",
	2,
	(char *) 0
};

extern char *sWeb_RecSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgRecSetting_Item_84 = {
	"17",
	(void*)sWeb_RecSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgRecSetting_Item_85[] =
	"  " C_xTD C_xTR C_oTR_ALIGN_RIGHT ">\n"
	"      " C_oTD_COLSPAN "\"3\">\n"
	"      ";

static char PgRecSetting_Item_86[] =
	" onClick=\"return formCheck(this.form)\"";

static rpButtonFormItem PgRecSetting_Item_87 = {
	"Apply",
	(char *) PgRecSetting_Item_86
};

static char PgRecSetting_Item_88[] =
	"      " C_xTD C_xTR C_xTABLE C_xFORM
	C_xBODY_xHTML;



static rpItem PgRecSetting_Form_1_Items[] = { 
	{ eRpItemType_FormHiddenText, (void*)&PgRecSetting_Item_16 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgRecSetting_Item_25 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgRecSetting_Item_28 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgRecSetting_Item_34 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgRecSetting_Item_37 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgRecSetting_Item_43 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgRecSetting_Item_46 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgRecSetting_Item_52 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgRecSetting_Item_55 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgRecSetting_Item_61 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgRecSetting_Item_64 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgRecSetting_Item_67 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgRecSetting_Item_77 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgRecSetting_Item_80 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgRecSetting_Item_83 }, 
	{ eRpItemType_FormSubmit, (void*)&PgRecSetting_Item_87 }, 
	{ eRpItemType_LastItemInList } 
};

extern void sWeb_RecSet_Apply(void *theTaskDataPtr, Signed16Ptr theIndexValuesPtr);
static rpObjectExtension PgRecSetting_Form_1_ObjectExtension = {
	sWeb_RecSet_Apply,
	&PgRecSetting,
	(rpObjectDescriptionPtr) 0,
	0,
	kRpObjFlags_None,
	(char *) 0
};

rpObjectDescription PgRecSetting_Form_1 = {
	"/Forms/sWeb_RecSet",
	PgRecSetting_Form_1_Items,
	&PgRecSetting_Form_1_ObjectExtension,
	(Unsigned32) 0,
	kRpPageAccess_Realm8,
	eRpDataTypeForm,
	eRpObjectTypeDynamic
};

static rpItem PgRecSetting_Items[] = { 
	{ eRpItemType_DataZeroTerminated, (void*)&PgRecSetting_Item_1 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgRecSetting_Item_2 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgRecSetting_Item_3 }, 
	{ eRpItemType_DisplayText, (void*)&PgRecSetting_Item_4 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgRecSetting_Item_5 }, 
	{ eRpItemType_FormHeader, (void*)&PgRecSetting_Form_1 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgRecSetting_Item_7 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgRecSetting_Item_8 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgRecSetting_Item_9 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgRecSetting_Item_10 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgRecSetting_Item_11 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgRecSetting_Item_12 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgRecSetting_Item_13 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgRecSetting_Item_14 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgRecSetting_Item_15 }, 
	{ eRpItemType_FormHiddenText, (void*)&PgRecSetting_Item_16 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgRecSetting_Item_17 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgRecSetting_Item_18 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgRecSetting_Item_19 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgRecSetting_Item_20 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgRecSetting_Item_21 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgRecSetting_Item_22 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgRecSetting_Item_23 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgRecSetting_Item_24 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgRecSetting_Item_25 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgRecSetting_Item_26 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgRecSetting_Item_27 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgRecSetting_Item_28 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgRecSetting_Item_29 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgRecSetting_Item_30 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgRecSetting_Item_31 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgRecSetting_Item_32 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgRecSetting_Item_33 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgRecSetting_Item_34 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgRecSetting_Item_35 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgRecSetting_Item_36 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgRecSetting_Item_37 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgRecSetting_Item_38 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgRecSetting_Item_39 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgRecSetting_Item_40 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgRecSetting_Item_41 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgRecSetting_Item_42 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgRecSetting_Item_43 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgRecSetting_Item_44 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgRecSetting_Item_45 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgRecSetting_Item_46 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgRecSetting_Item_47 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgRecSetting_Item_48 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgRecSetting_Item_49 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgRecSetting_Item_50 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgRecSetting_Item_51 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgRecSetting_Item_52 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgRecSetting_Item_53 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgRecSetting_Item_54 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgRecSetting_Item_55 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgRecSetting_Item_56 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgRecSetting_Item_57 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgRecSetting_Item_58 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgRecSetting_Item_59 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgRecSetting_Item_60 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgRecSetting_Item_61 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgRecSetting_Item_62 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgRecSetting_Item_63 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgRecSetting_Item_64 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgRecSetting_Item_65 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgRecSetting_Item_66 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgRecSetting_Item_67 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgRecSetting_Item_68 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgRecSetting_Item_69 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgRecSetting_Item_70 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgRecSetting_Item_71 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgRecSetting_Item_72 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgRecSetting_Item_73 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgRecSetting_Item_74 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgRecSetting_Item_75 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgRecSetting_Item_76 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgRecSetting_Item_77 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgRecSetting_Item_78 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgRecSetting_Item_79 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgRecSetting_Item_80 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgRecSetting_Item_81 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgRecSetting_Item_82 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgRecSetting_Item_83 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgRecSetting_Item_84 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgRecSetting_Item_85 }, 
	{ eRpItemType_FormSubmit, (void*)&PgRecSetting_Item_87 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgRecSetting_Item_88 }, 
	{ eRpItemType_LastItemInList } 
};


rpObjectDescription PgRecSetting = {
	"/html/RecSetting.html",
	PgRecSetting_Items,
	(rpObjectExtensionPtr) 0,
	(Unsigned32) 0,
	kRpPageAccess_Realm7,
	eRpDataTypeHtml,
	eRpObjectTypeDynamic
};


/* *********************************************** */
/* *    Built from "html\DisplaySetting.html"    * */
/* *********************************************** */

extern rpObjectDescription PgDisplaySetting;

static char PgDisplaySetting_Item_1[] =
	C_oHTML C_oHEAD "\n"
	C_oMETA " http-equiv=\"Content-Type\"" C_CONTENT "\"text/html; ";

extern char *sWeb_Menu_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgDisplaySetting_Item_2 = {
	"69",
	(void*)sWeb_Menu_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgDisplaySetting_Item_3[] =
	"\">\n"
	C_oTITLE "Display Setting" C_xTITLE "<LINK REL=stylesheet "
	"TYPE=\"text/css\" HREF=\"web_style.css\">\n"
	C_xHEAD "<script src=\"jsComm.js\"></script>\n"
	"<SCRIPT language=\"javascript\">\n";

extern char *sWeb_DisplaySet_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpTextDisplayItem PgDisplaySetting_Item_4 = {
	(void*)sWeb_DisplaySet_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII,
	20
};

static char PgDisplaySetting_Item_5[] =
	"\n"
	"function formCheck(f)\n"
	"{\n"
	"\tif(!checkAccessRight())\n"
	"\t\treturn false;\n"
	"\treturn true;\n"
	"}\n"
	"\n"
	"</SCRIPT>\n"
	"\n"
	C_oBODY " text=\"#000000\" bgcolor=\"ffffff\" link=\"#0000CD\">\n"
	"\n";

static char PgDisplaySetting_Item_7[] =
	"\n"
	"\n"
	"<LAYER style=\"position : absolute; top : -50px\">\n"
	C_oDIV_ " style=\"position : absolute; top : -50px\">\n"
	"<!-- <INPUT type=\"radio\" style=\"background : #000000\"> -->\n"
	C_xDIV "</LAYER>\n"
	C_oBR C_oP C_oFONT_SIZE "\"3\">"
	C_oB;

extern char *sWeb_DisplaySet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgDisplaySetting_Item_8 = {
	"1",
	(void*)sWeb_DisplaySet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgDisplaySetting_Item_9[] =
	C_xB C_xFONT "\n"
	C_oHR C_oBR C_oTABLE_CELLPADDING "\"0\"" C_BORDER "\"0\" "
	"class=\"pageTitle\">\n"
	" <TR" C_BGCOLOR "B4CDCD\"" C_ALIGN_CENTER ">\n"
	"  <TD nowrap class=pageTitle>";

extern char *sWeb_DisplaySet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgDisplaySetting_Item_10 = {
	"2",
	(void*)sWeb_DisplaySet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgDisplaySetting_Item_11[] =
	C_xTD "<TD nowrap" C_COLSPAN "2 class=pageTitle >";

extern char *sWeb_DisplaySet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgDisplaySetting_Item_12 = {
	"3",
	(void*)sWeb_DisplaySet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgDisplaySetting_Item_13[] =
	C_xTD C_xTR C_oTR_ALIGN_CENTER ">\n"
	"  <TD nowrap class=\"itemTitle\">";

extern char *sWeb_DisplaySet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgDisplaySetting_Item_14 = {
	"4",
	(void*)sWeb_DisplaySet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgDisplaySetting_Item_15[] =
	C_xTD "<TD nowrap" C_COLSPAN "2 class=\"itemContent\">\n"
	"\t";

extern char *sWeb_DisplaySet_AutoChannel_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
extern void sWeb_DisplaySet_AutoChannel_Set(void *theTaskDataPtr, char *theValuePtr,
		char *theNamePtr, Signed16Ptr theIndexValuesPtr);
static rpTextFormItem PgDisplaySetting_Item_16 = {
	"sWeb_DisplaySet_AutoChannel",
	(void*)sWeb_DisplaySet_AutoChannel_Get,
	(void*)sWeb_DisplaySet_AutoChannel_Set,
	eRpVarType_Complex,
	eRpVarType_Complex,
	eRpTextType_ASCII,
	2,
	2,
	(char *) 0
};

static char PgDisplaySetting_Item_17[] =
	"    "
	C_NBSP;

extern char *sWeb_DisplaySet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgDisplaySetting_Item_18 = {
	"5",
	(void*)sWeb_DisplaySet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgDisplaySetting_Item_19[] =
	C_NBSP C_NBSP "\n"
	"  " C_xTD C_xTR C_oTR_ALIGN_CENTER ">\n"
	"  <TD nowrap" C_ROWSPAN "6 class=\"itemTitle\">";

extern char *sWeb_DisplaySet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgDisplaySetting_Item_20 = {
	"6",
	(void*)sWeb_DisplaySet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgDisplaySetting_Item_21[] =
	C_xTD "<TD nowrap" C_ROWSPAN "2 class=\"itemTitle\">";

extern char *sWeb_DisplaySet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgDisplaySetting_Item_22 = {
	"7",
	(void*)sWeb_DisplaySet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgDisplaySetting_Item_23[] =
	C_xTD "<TD nowrap class=\"itemContent\">\n"
	"   ";

extern rpOneOfSeveral sWeb_DisplaySet_OSDTime_Radio_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
extern void sWeb_DisplaySet_OSDTime_Radio_Set(void *theTaskDataPtr, rpOneOfSeveral theValue,
		char *theNamePtr, Signed16Ptr theIndexValuesPtr);
static rpRadioGroupInfo PgDisplaySetting_RadioGroup_1 = {
	"sWeb_DisplaySet_OSDTime_Radio",
	(void*)sWeb_DisplaySet_OSDTime_Radio_Get,
	(void*)sWeb_DisplaySet_OSDTime_Radio_Set,
	eRpVarType_Complex,
	eRpVarType_Complex
};

static char PgDisplaySetting_Item_24[] =
	"   ";

static rpRadioButtonFormItem PgDisplaySetting_Item_25 = {
	&PgDisplaySetting_RadioGroup_1,
	"0",
	0,
	(char *) 0
};

static char PgDisplaySetting_Item_26[] =
	"   ";

extern char *sWeb_DisplaySet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgDisplaySetting_Item_27 = {
	"8",
	(void*)sWeb_DisplaySet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgDisplaySetting_Item_28[] =
	C_NBSP C_NBSP "\n"
	"  " C_xTD C_xTR "<TR >\n"
	"   <TD nowrap class=\"itemContent\">\n"
	"   ";

static rpRadioButtonFormItem PgDisplaySetting_Item_29 = {
	&PgDisplaySetting_RadioGroup_1,
	"1",
	1,
	(char *) 0
};

static char PgDisplaySetting_Item_30[] =
	"   ";

extern char *sWeb_DisplaySet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgDisplaySetting_Item_31 = {
	"9",
	(void*)sWeb_DisplaySet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgDisplaySetting_Item_32[] =
	C_NBSP C_NBSP "\n"
	"  " C_xTD C_xTR C_oTR_ALIGN_CENTER ">\n"
	"  <TD nowrap" C_ROWSPAN "2 class=\"itemTitle\">";

extern char *sWeb_DisplaySet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgDisplaySetting_Item_33 = {
	"10",
	(void*)sWeb_DisplaySet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgDisplaySetting_Item_34[] =
	C_xTD "<TD nowrap class=\"itemContent\">\n"
	"   ";

extern rpOneOfSeveral sWeb_DisplaySet_OSDChel_Radio_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
extern void sWeb_DisplaySet_OSDChel_Radio_Set(void *theTaskDataPtr, rpOneOfSeveral theValue,
		char *theNamePtr, Signed16Ptr theIndexValuesPtr);
static rpRadioGroupInfo PgDisplaySetting_RadioGroup_2 = {
	"sWeb_DisplaySet_OSDChel_Radio",
	(void*)sWeb_DisplaySet_OSDChel_Radio_Get,
	(void*)sWeb_DisplaySet_OSDChel_Radio_Set,
	eRpVarType_Complex,
	eRpVarType_Complex
};

static char PgDisplaySetting_Item_35[] =
	"   ";

static rpRadioButtonFormItem PgDisplaySetting_Item_36 = {
	&PgDisplaySetting_RadioGroup_2,
	"0",
	0,
	(char *) 0
};

static char PgDisplaySetting_Item_37[] =
	"   ";

extern char *sWeb_DisplaySet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgDisplaySetting_Item_38 = {
	"8",
	(void*)sWeb_DisplaySet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgDisplaySetting_Item_39[] =
	C_NBSP C_NBSP "\n"
	"  " C_xTD C_xTR "<TR >\n"
	"   <TD nowrap class=\"itemContent\">\n"
	"   ";

static rpRadioButtonFormItem PgDisplaySetting_Item_40 = {
	&PgDisplaySetting_RadioGroup_2,
	"1",
	1,
	(char *) 0
};

static char PgDisplaySetting_Item_41[] =
	"   ";

extern char *sWeb_DisplaySet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgDisplaySetting_Item_42 = {
	"9",
	(void*)sWeb_DisplaySet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgDisplaySetting_Item_43[] =
	C_NBSP C_NBSP "\n"
	"  " C_xTD C_xTR C_oTR_ALIGN_CENTER ">\n"
	"  <TD nowrap" C_ROWSPAN "2 class=\"itemTitle\">";

extern char *sWeb_DisplaySet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgDisplaySetting_Item_44 = {
	"11",
	(void*)sWeb_DisplaySet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgDisplaySetting_Item_45[] =
	C_xTD "<TD nowrap class=\"itemContent\">\n"
	"   ";

extern rpOneOfSeveral sWeb_DisplaySet_OSDCard_Radio_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
extern void sWeb_DisplaySet_OSDCard_Radio_Set(void *theTaskDataPtr, rpOneOfSeveral theValue,
		char *theNamePtr, Signed16Ptr theIndexValuesPtr);
static rpRadioGroupInfo PgDisplaySetting_RadioGroup_3 = {
	"sWeb_DisplaySet_OSDCard_Radio",
	(void*)sWeb_DisplaySet_OSDCard_Radio_Get,
	(void*)sWeb_DisplaySet_OSDCard_Radio_Set,
	eRpVarType_Complex,
	eRpVarType_Complex
};

static char PgDisplaySetting_Item_46[] =
	"   ";

static rpRadioButtonFormItem PgDisplaySetting_Item_47 = {
	&PgDisplaySetting_RadioGroup_3,
	"0",
	0,
	(char *) 0
};

static char PgDisplaySetting_Item_48[] =
	"   ";

extern char *sWeb_DisplaySet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgDisplaySetting_Item_49 = {
	"8",
	(void*)sWeb_DisplaySet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgDisplaySetting_Item_50[] =
	C_NBSP C_NBSP "\n"
	"  " C_xTD C_xTR "<TR >\n"
	"   <TD nowrap class=\"itemContent\">\n"
	"   ";

static rpRadioButtonFormItem PgDisplaySetting_Item_51 = {
	&PgDisplaySetting_RadioGroup_3,
	"1",
	1,
	(char *) 0
};

static char PgDisplaySetting_Item_52[] =
	"   ";

extern char *sWeb_DisplaySet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgDisplaySetting_Item_53 = {
	"9",
	(void*)sWeb_DisplaySet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgDisplaySetting_Item_54[] =
	C_NBSP C_NBSP "\n"
	"  " C_xTD C_xTR C_oTR_ALIGN_RIGHT ">\n"
	"      " C_oTD_COLSPAN "\"3\">\n"
	"      ";

static char PgDisplaySetting_Item_55[] =
	" onClick=\"return formCheck(this.form)\"";

static rpButtonFormItem PgDisplaySetting_Item_56 = {
	"Apply",
	(char *) PgDisplaySetting_Item_55
};

static char PgDisplaySetting_Item_57[] =
	"      " C_xTD C_xTR C_xTABLE C_xFORM
	C_xBODY_xHTML;



static rpItem PgDisplaySetting_Form_1_Items[] = { 
	{ eRpItemType_FormAsciiText, (void*)&PgDisplaySetting_Item_16 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgDisplaySetting_Item_25 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgDisplaySetting_Item_29 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgDisplaySetting_Item_36 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgDisplaySetting_Item_40 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgDisplaySetting_Item_47 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgDisplaySetting_Item_51 }, 
	{ eRpItemType_FormSubmit, (void*)&PgDisplaySetting_Item_56 }, 
	{ eRpItemType_LastItemInList } 
};

extern void sWeb_DisplaySet_Apply(void *theTaskDataPtr, Signed16Ptr theIndexValuesPtr);
static rpObjectExtension PgDisplaySetting_Form_1_ObjectExtension = {
	sWeb_DisplaySet_Apply,
	&PgDisplaySetting,
	(rpObjectDescriptionPtr) 0,
	0,
	kRpObjFlags_None,
	(char *) 0
};

rpObjectDescription PgDisplaySetting_Form_1 = {
	"/Forms/sWeb_DisplaySet",
	PgDisplaySetting_Form_1_Items,
	&PgDisplaySetting_Form_1_ObjectExtension,
	(Unsigned32) 0,
	kRpPageAccess_Realm8,
	eRpDataTypeForm,
	eRpObjectTypeDynamic
};

static rpItem PgDisplaySetting_Items[] = { 
	{ eRpItemType_DataZeroTerminated, (void*)&PgDisplaySetting_Item_1 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgDisplaySetting_Item_2 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgDisplaySetting_Item_3 }, 
	{ eRpItemType_DisplayText, (void*)&PgDisplaySetting_Item_4 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgDisplaySetting_Item_5 }, 
	{ eRpItemType_FormHeader, (void*)&PgDisplaySetting_Form_1 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgDisplaySetting_Item_7 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgDisplaySetting_Item_8 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgDisplaySetting_Item_9 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgDisplaySetting_Item_10 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgDisplaySetting_Item_11 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgDisplaySetting_Item_12 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgDisplaySetting_Item_13 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgDisplaySetting_Item_14 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgDisplaySetting_Item_15 }, 
	{ eRpItemType_FormAsciiText, (void*)&PgDisplaySetting_Item_16 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgDisplaySetting_Item_17 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgDisplaySetting_Item_18 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgDisplaySetting_Item_19 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgDisplaySetting_Item_20 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgDisplaySetting_Item_21 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgDisplaySetting_Item_22 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgDisplaySetting_Item_23 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgDisplaySetting_Item_24 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgDisplaySetting_Item_25 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgDisplaySetting_Item_26 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgDisplaySetting_Item_27 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgDisplaySetting_Item_28 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgDisplaySetting_Item_29 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgDisplaySetting_Item_30 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgDisplaySetting_Item_31 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgDisplaySetting_Item_32 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgDisplaySetting_Item_33 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgDisplaySetting_Item_34 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgDisplaySetting_Item_35 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgDisplaySetting_Item_36 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgDisplaySetting_Item_37 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgDisplaySetting_Item_38 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgDisplaySetting_Item_39 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgDisplaySetting_Item_40 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgDisplaySetting_Item_41 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgDisplaySetting_Item_42 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgDisplaySetting_Item_43 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgDisplaySetting_Item_44 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgDisplaySetting_Item_45 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgDisplaySetting_Item_46 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgDisplaySetting_Item_47 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgDisplaySetting_Item_48 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgDisplaySetting_Item_49 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgDisplaySetting_Item_50 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgDisplaySetting_Item_51 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgDisplaySetting_Item_52 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgDisplaySetting_Item_53 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgDisplaySetting_Item_54 }, 
	{ eRpItemType_FormSubmit, (void*)&PgDisplaySetting_Item_56 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgDisplaySetting_Item_57 }, 
	{ eRpItemType_LastItemInList } 
};


rpObjectDescription PgDisplaySetting = {
	"/html/DisplaySetting.html",
	PgDisplaySetting_Items,
	(rpObjectExtensionPtr) 0,
	(Unsigned32) 0,
	kRpPageAccess_Realm7,
	eRpDataTypeHtml,
	eRpObjectTypeDynamic
};


/* ********************************************* */
/* *    Built from "html\ImageSetting.html"    * */
/* ********************************************* */

extern rpObjectDescription PgImageSetting;

static char PgImageSetting_Item_1[] =
	C_oHTML C_oHEAD "\n"
	C_oMETA " http-equiv=\"Content-Type\"" C_CONTENT "\"text/html; ";

extern char *sWeb_Menu_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgImageSetting_Item_2 = {
	"69",
	(void*)sWeb_Menu_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgImageSetting_Item_3[] =
	"\">\n"
	C_oTITLE "Image Setting" C_xTITLE "<LINK REL=stylesheet "
	"TYPE=\"text/css\" HREF=\"web_style.css\">\n"
	C_xHEAD "<script src=\"jsComm.js\"></script>\n"
	"<SCRIPT language=\"javascript\">\n";

extern char *sWeb_ImgSet_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpTextDisplayItem PgImageSetting_Item_4 = {
	(void*)sWeb_ImgSet_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII,
	20
};

static char PgImageSetting_Item_5[] =
	"\n"
	"function formCheck(f)\n"
	"{\n"
	"\tif(!checkAccessRight())\n"
	"\t\treturn false;\n"
	"\treturn true;\n"
	"}\n"
	"\n"
	"</SCRIPT>\n"
	"\n"
	C_oBODY " text=\"#000000\" bgcolor=\"ffffff\" link=\"#0000CD\">\n"
	"\n";

static char PgImageSetting_Item_7[] =
	"\n"
	"\n"
	"<LAYER style=\"position : absolute; top : -50px\">\n"
	C_oDIV_ " style=\"position : absolute; top : -50px\">\n"
	"<!-- <INPUT type=\"radio\" style=\"background : #000000\"> -->\n"
	C_xDIV "</LAYER>\n"
	C_oBR C_oP C_oFONT_SIZE "\"3\">"
	C_oB;

extern char *sWeb_ImgSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgImageSetting_Item_8 = {
	"1",
	(void*)sWeb_ImgSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgImageSetting_Item_9[] =
	C_xB C_xFONT "\n"
	C_oHR C_oBR C_oTABLE_CELLPADDING "\"0\"" C_BORDER "\"0\" "
	"class=\"pageTitle\">\n"
	" <TR" C_BGCOLOR "B4CDCD\"" C_ALIGN_CENTER ">\n"
	"  <TD nowrap class=pageTitle>";

extern char *sWeb_ImgSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgImageSetting_Item_10 = {
	"2",
	(void*)sWeb_ImgSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgImageSetting_Item_11[] =
	C_xTD "<TD nowrap class=pageTitle >";

extern char *sWeb_ImgSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgImageSetting_Item_12 = {
	"3",
	(void*)sWeb_ImgSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgImageSetting_Item_13[] =
	C_xTD C_xTR C_oTR_ALIGN_CENTER ">\n"
	"  <TD nowrap" C_ROWSPAN "3 class=\"itemTitle\">";

extern char *sWeb_ImgSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgImageSetting_Item_14 = {
	"4",
	(void*)sWeb_ImgSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgImageSetting_Item_15[] =
	C_xTD "<TD nowrap class=\"itemContent\">\n"
	"   ";

extern rpOneOfSeveral sWeb_ImgSet_Quality_Radio_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
extern void sWeb_ImgSet_Quality_Radio_Set(void *theTaskDataPtr, rpOneOfSeveral theValue,
		char *theNamePtr, Signed16Ptr theIndexValuesPtr);
static rpRadioGroupInfo PgImageSetting_RadioGroup_1 = {
	"sWeb_ImgSet_Quality_Radio",
	(void*)sWeb_ImgSet_Quality_Radio_Get,
	(void*)sWeb_ImgSet_Quality_Radio_Set,
	eRpVarType_Complex,
	eRpVarType_Complex
};

static char PgImageSetting_Item_16[] =
	"   ";

static rpRadioButtonFormItem PgImageSetting_Item_17 = {
	&PgImageSetting_RadioGroup_1,
	"0",
	0,
	(char *) 0
};

extern char *sWeb_ImgSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgImageSetting_Item_18 = {
	"5",
	(void*)sWeb_ImgSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgImageSetting_Item_19[] =
	C_NBSP C_NBSP "\n"
	"  " C_xTD C_xTR "<TR >\n"
	"   <TD nowrap class=\"itemContent\">\n"
	"   ";

static rpRadioButtonFormItem PgImageSetting_Item_20 = {
	&PgImageSetting_RadioGroup_1,
	"1",
	1,
	(char *) 0
};

extern char *sWeb_ImgSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgImageSetting_Item_21 = {
	"6",
	(void*)sWeb_ImgSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgImageSetting_Item_22[] =
	C_NBSP C_NBSP "\n"
	"  " C_xTD C_xTR "<TR >\n"
	"   <TD nowrap class=\"itemContent\">\n"
	"   ";

static rpRadioButtonFormItem PgImageSetting_Item_23 = {
	&PgImageSetting_RadioGroup_1,
	"2",
	2,
	(char *) 0
};

extern char *sWeb_ImgSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgImageSetting_Item_24 = {
	"7",
	(void*)sWeb_ImgSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgImageSetting_Item_25[] =
	C_NBSP C_NBSP "\n"
	"  " C_xTD C_xTR C_oTR_ALIGN_CENTER ">\n"
	"  <TD nowrap" C_ROWSPAN "3 class=\"itemTitle\">";

extern char *sWeb_ImgSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgImageSetting_Item_26 = {
	"8",
	(void*)sWeb_ImgSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgImageSetting_Item_27[] =
	C_xTD "<TD nowrap class=\"itemContent\">\n"
	"   ";

extern rpOneOfSeveral sWeb_ImgSet_Resolution_Radio_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
extern void sWeb_ImgSet_Resolution_Radio_Set(void *theTaskDataPtr, rpOneOfSeveral theValue,
		char *theNamePtr, Signed16Ptr theIndexValuesPtr);
static rpRadioGroupInfo PgImageSetting_RadioGroup_2 = {
	"sWeb_ImgSet_Resolution_Radio",
	(void*)sWeb_ImgSet_Resolution_Radio_Get,
	(void*)sWeb_ImgSet_Resolution_Radio_Set,
	eRpVarType_Complex,
	eRpVarType_Complex
};

static char PgImageSetting_Item_28[] =
	"   ";

static rpRadioButtonFormItem PgImageSetting_Item_29 = {
	&PgImageSetting_RadioGroup_2,
	"0",
	0,
	(char *) 0
};

extern char *sWeb_ImgSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgImageSetting_Item_30 = {
	"9",
	(void*)sWeb_ImgSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgImageSetting_Item_31[] =
	C_NBSP C_NBSP "\n"
	"  " C_xTD C_xTR "<TR >\n"
	"   <TD nowrap class=\"itemContent\">\n"
	"   ";

static rpRadioButtonFormItem PgImageSetting_Item_32 = {
	&PgImageSetting_RadioGroup_2,
	"1",
	1,
	(char *) 0
};

extern char *sWeb_ImgSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgImageSetting_Item_33 = {
	"10",
	(void*)sWeb_ImgSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgImageSetting_Item_34[] =
	C_NBSP C_NBSP "\n"
	"  " C_xTD C_xTR "<TR >\n"
	"   <TD nowrap class=\"itemContent\">\n"
	"   ";

static rpRadioButtonFormItem PgImageSetting_Item_35 = {
	&PgImageSetting_RadioGroup_2,
	"2",
	2,
	(char *) 0
};

extern char *sWeb_ImgSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgImageSetting_Item_36 = {
	"11",
	(void*)sWeb_ImgSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgImageSetting_Item_37[] =
	C_NBSP C_NBSP "\n"
	"  " C_xTD C_xTR C_oTR_ALIGN_CENTER ">\n"
	"  <TD nowrap" C_ROWSPAN "3 class=\"itemTitle\">";

extern char *sWeb_ImgSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgImageSetting_Item_38 = {
	"12",
	(void*)sWeb_ImgSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgImageSetting_Item_39[] =
	C_xTD "<TD nowrap class=\"itemContent\">\n"
	"   ";

extern rpOneOfSeveral sWeb_ImgSet_FrameRate_Radio_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
extern void sWeb_ImgSet_FrameRate_Radio_Set(void *theTaskDataPtr, rpOneOfSeveral theValue,
		char *theNamePtr, Signed16Ptr theIndexValuesPtr);
static rpRadioGroupInfo PgImageSetting_RadioGroup_3 = {
	"sWeb_ImgSet_FrameRate_Radio",
	(void*)sWeb_ImgSet_FrameRate_Radio_Get,
	(void*)sWeb_ImgSet_FrameRate_Radio_Set,
	eRpVarType_Complex,
	eRpVarType_Complex
};

static char PgImageSetting_Item_40[] =
	"   ";

static rpRadioButtonFormItem PgImageSetting_Item_41 = {
	&PgImageSetting_RadioGroup_3,
	"0",
	0,
	(char *) 0
};

extern char *sWeb_ImgSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgImageSetting_Item_42 = {
	"13",
	(void*)sWeb_ImgSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgImageSetting_Item_43[] =
	C_NBSP C_NBSP "\n"
	"  " C_xTD C_xTR "<TR >\n"
	"   <TD nowrap class=\"itemContent\">\n"
	"   ";

static rpRadioButtonFormItem PgImageSetting_Item_44 = {
	&PgImageSetting_RadioGroup_3,
	"1",
	1,
	(char *) 0
};

extern char *sWeb_ImgSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgImageSetting_Item_45 = {
	"14",
	(void*)sWeb_ImgSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgImageSetting_Item_46[] =
	C_NBSP C_NBSP "\n"
	"  " C_xTD C_xTR "<TR >\n"
	"   <TD nowrap class=\"itemContent\">\n"
	"   ";

static rpRadioButtonFormItem PgImageSetting_Item_47 = {
	&PgImageSetting_RadioGroup_3,
	"2",
	2,
	(char *) 0
};

extern char *sWeb_ImgSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr);
static rpNamedTextDisplayItem PgImageSetting_Item_48 = {
	"15",
	(void*)sWeb_ImgSet_Language_Get,
	eRpVarType_Complex,
	eRpTextType_ASCII_Extended,
	20
};

static char PgImageSetting_Item_49[] =
	C_NBSP C_NBSP "\n"
	"  " C_xTD C_xTR C_oTR_ALIGN_RIGHT ">\n"
	"      " C_oTD_COLSPAN "\"2\">\n"
	"      ";

static char PgImageSetting_Item_50[] =
	" onClick=\"return formCheck(this.form)\"";

static rpButtonFormItem PgImageSetting_Item_51 = {
	"Apply",
	(char *) PgImageSetting_Item_50
};

static char PgImageSetting_Item_52[] =
	"      " C_xTD C_xTR C_xTABLE C_xFORM
	C_xBODY_xHTML;



static rpItem PgImageSetting_Form_1_Items[] = { 
	{ eRpItemType_FormRadioButton, (void*)&PgImageSetting_Item_17 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgImageSetting_Item_20 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgImageSetting_Item_23 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgImageSetting_Item_29 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgImageSetting_Item_32 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgImageSetting_Item_35 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgImageSetting_Item_41 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgImageSetting_Item_44 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgImageSetting_Item_47 }, 
	{ eRpItemType_FormSubmit, (void*)&PgImageSetting_Item_51 }, 
	{ eRpItemType_LastItemInList } 
};

extern void sWeb_ImgSet_Apply(void *theTaskDataPtr, Signed16Ptr theIndexValuesPtr);
static rpObjectExtension PgImageSetting_Form_1_ObjectExtension = {
	sWeb_ImgSet_Apply,
	&PgImageSetting,
	(rpObjectDescriptionPtr) 0,
	0,
	kRpObjFlags_None,
	(char *) 0
};

rpObjectDescription PgImageSetting_Form_1 = {
	"/Forms/sWeb_ImgSet",
	PgImageSetting_Form_1_Items,
	&PgImageSetting_Form_1_ObjectExtension,
	(Unsigned32) 0,
	kRpPageAccess_Realm8,
	eRpDataTypeForm,
	eRpObjectTypeDynamic
};

static rpItem PgImageSetting_Items[] = { 
	{ eRpItemType_DataZeroTerminated, (void*)&PgImageSetting_Item_1 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgImageSetting_Item_2 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgImageSetting_Item_3 }, 
	{ eRpItemType_DisplayText, (void*)&PgImageSetting_Item_4 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgImageSetting_Item_5 }, 
	{ eRpItemType_FormHeader, (void*)&PgImageSetting_Form_1 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgImageSetting_Item_7 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgImageSetting_Item_8 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgImageSetting_Item_9 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgImageSetting_Item_10 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgImageSetting_Item_11 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgImageSetting_Item_12 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgImageSetting_Item_13 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgImageSetting_Item_14 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgImageSetting_Item_15 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgImageSetting_Item_16 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgImageSetting_Item_17 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgImageSetting_Item_18 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgImageSetting_Item_19 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgImageSetting_Item_20 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgImageSetting_Item_21 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgImageSetting_Item_22 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgImageSetting_Item_23 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgImageSetting_Item_24 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgImageSetting_Item_25 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgImageSetting_Item_26 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgImageSetting_Item_27 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgImageSetting_Item_28 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgImageSetting_Item_29 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgImageSetting_Item_30 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgImageSetting_Item_31 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgImageSetting_Item_32 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgImageSetting_Item_33 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgImageSetting_Item_34 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgImageSetting_Item_35 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgImageSetting_Item_36 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgImageSetting_Item_37 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgImageSetting_Item_38 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgImageSetting_Item_39 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgImageSetting_Item_40 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgImageSetting_Item_41 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgImageSetting_Item_42 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgImageSetting_Item_43 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgImageSetting_Item_44 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgImageSetting_Item_45 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgImageSetting_Item_46 }, 
	{ eRpItemType_FormRadioButton, (void*)&PgImageSetting_Item_47 }, 
	{ eRpItemType_NamedDisplayText, (void*)&PgImageSetting_Item_48 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgImageSetting_Item_49 }, 
	{ eRpItemType_FormSubmit, (void*)&PgImageSetting_Item_51 }, 
	{ eRpItemType_DataZeroTerminated, (void*)&PgImageSetting_Item_52 }, 
	{ eRpItemType_LastItemInList } 
};


rpObjectDescription PgImageSetting = {
	"/html/ImageSetting.html",
	PgImageSetting_Items,
	(rpObjectExtensionPtr) 0,
	(Unsigned32) 0,
	kRpPageAccess_Realm7,
	eRpDataTypeHtml,
	eRpObjectTypeDynamic
};

#endif	/* RomPagerServer */
