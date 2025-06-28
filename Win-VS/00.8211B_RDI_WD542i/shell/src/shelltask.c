/*	 
*	shelltask.c
*	the genie shell task for ucosII
*	under skyeye
*
*	Bugs report:	 Yang Ye  ( yangye@163.net )
*	Last modified:	 2005-05-6 
*  	changed by: wenjia
*/

#include	"general.h"
#include	"osapi.h"
#include	"commands.h"
#include	"shelltask.h"
#include	"fsapi.h"
#include 	"ramdiskapi.h"
#include	"sdcapi.h"
#include	"smcapi.h"


extern int storageType;
extern command ShellComms[MAX_COMMAND_NUM];
extern void CommRxIntEn(INT8U ch);
extern char CurDir[64];
char *argv[10];
INT8U argc;

void shelltask(void *pParam)
{
	INT8U i=0,num;		/* i is the pointer of commandbuf */
	char ch;
	INT8U (*Func)(INT8U argc,char **argv);
	char CommandBuf[MaxLenComBuf+1];	/*store '\0'*/
	InitPath(); /*CY 1023*/
	InitCommands();
	CommandBuf[0] = '\0';
	
	_log("\n\r************************************************************\n");
	_log("\n\r*		 Welcom to genie shell\n");
	_log("\n\r*		   Modified by: David Tsai\n");
	_log("\n\r*	  use \"help\" command for help information\n");
	_log("\n\r************************************************************\n\n");
	
	/*To be done: Login & Password*/

	switch (storageType) /*CY 1023*/
	{
		case STORAGE_MEMORY_RAMDISK:
			ramDiskInit();
			ramDiskInit1();
			break;
			
		case STORAGE_MEMORY_SD_MMC:
			sdcInit();	
			sdcMount();
			break;
			
		case STORAGE_MEMORY_SMC_NAND:
			smcInit();	
			smcMount();
	}		

    	ShowHelp();

	switch (storageType) /*CY 1023*/
	{
		case STORAGE_MEMORY_RAMDISK:
			_log("RAMDISK:");
			break;
			
		case STORAGE_MEMORY_SD_MMC:
			_log("SDMMC:");
			break;
			
		case STORAGE_MEMORY_SMC_NAND:
			_log("SMC:");
			break;
	}		
		
	_log(CurDir);
	_log(">");
    
	for(;;){
		do
		{		
		        ch = getchar();
		}while(!((ch>='0'&&ch<='9')||(ch>='a'&&ch<='z')||(ch>='A'&&ch<='Z')||(ch=='.')||(ch==' ')||(ch==':')||(ch=='\\')||(ch=='-')||(ch=='/')||(ch==0xa)||(ch==0xd)||(ch=='\b')||(ch==',')));
		
		switch(ch)
		{
		case 0xa:	
		case 0xd:				//enter
			if (i==0)
			{      						//commandbuf is null,begin a new line
				switch (storageType) /*CY 1023*/
				{
					case STORAGE_MEMORY_RAMDISK:
						_log("\nRAMDISK:");
						break;
						
					case STORAGE_MEMORY_SD_MMC:
						_log("\nSDMMC:");
						break;
						
					case STORAGE_MEMORY_SMC_NAND:
						_log("\nSMC:");
						break;
				}		

				_log(CurDir);
				_log(">");
			}
			else{
				if(CommandBuf[i-1]==' ') 
					i--;				//get rid of the end space
				CommandBuf[i] = '\0';
				num = CommandAnalys(CommandBuf);	//analys the argv in the commandbuf
				if(num==ERRORCOMMAND)
				{	//error or none exist command
					i = 0;
					CommandBuf[i] = '\0';
					_log("error command!\n\n\r");
					
					switch (storageType) /*CY 1023*/
					{
						case STORAGE_MEMORY_RAMDISK:			
							_log("RAMDISK:");
							break;
							
							
						case STORAGE_MEMORY_SD_MMC:					
							_log("SDMMC:");
							break;
							
						case STORAGE_MEMORY_SMC_NAND:				
							_log("SMC:");
							break;
					}		
				
					_log(CurDir);
					_log(">");
				}
				else{
					Func = ShellComms[num].CommandFunc;	//call corresponding CommandFunc
					Func(argc,argv);
					i = 0;
					CommandBuf[i] = '\0';
					
					switch (storageType)	/*CY 1023*/
					{
						case STORAGE_MEMORY_RAMDISK:
							_log("RAMDISK:");	/* cytsai: 0315 */
							break;
							
						case STORAGE_MEMORY_SD_MMC:				
							_log("SDMMC:");
							break;
							
						case STORAGE_MEMORY_SMC_NAND:
                                        		_log("SMC:");
                                        		break;
                                        }		
			
					_log(CurDir);
					_log(">");
				}
			}
			break;
			
		case '\b':				//backspace
			if ( i==0 ){	//has backed to first one
				//do nothing
			}
			else{
				i--;			//pointer back once
#if 0
				// cytsai: eliminate echo
				OS_ENTER_CRITICAL();
				putchar('\b');		//cursor back once
				putchar(' ');	      //earse last char in screen
				putchar('\b');		//cursor back again
				OS_ENTER_CRITICAL();
#endif
			}
			break;
			
		case ' ':               //don't allow continuous or begin space(' ')
			if((CommandBuf[i-1] == ' ')||(i==0)||(i>MaxLenComBuf)){
				//do nothing
			}
			else
			{
				CommandBuf[i] = ch;
				i++;
#if 0				
				// cytsai: eliminate echo
				OS_ENTER_CRITICAL();
				putchar(ch);  //display and store ch
				OS_ENTER_CRITICAL();
#endif				
			}
			break;
			
		default:				//normal key
			if (i>MaxLenComBuf){	//the buf reached MAX 
				//do nothing
			}			
			else{
				CommandBuf[i] = ch;
				i++;
#if 0				
				// cytsai: eliminate echo
				OS_ENTER_CRITICAL();
				putchar(ch);  //display and store ch
				OS_ENTER_CRITICAL();
#endif
			}
			break;
		}  //switch
	}//for(;;)
}

INT8U CommandAnalys(char *Buf)
{
	INT8U i;
	INT8U pointer;
	INT8U num;
	char name[20];		//command name length <20
	
	argc = 0;              //argc is global
	pointer = 0;
	num = 0;
	_log("\n\r");
	
	while((Buf[pointer]!=' ') && (Buf[pointer]!='\0') && pointer<20 ){
		name[pointer]=Buf[pointer];
		pointer++;
	}
	name[pointer] = '\0';	//now got the command name, and pointer is to the first space in the Buf
	
	for(i=0;i<MAX_COMMAND_NUM;i++){
		if(!strcmp(name,ShellComms[i].name)){
			num = i;
			break;
		}				//find the command number
	}					
	//not find it
	if (i==MAX_COMMAND_NUM) return ERRORCOMMAND;
	
	while(Buf[pointer]!='\0'){
		if(Buf[pointer]==' '){
			if(argc>0){
				Buf[pointer] = '\0';			//end of last argv
			}
			pointer++;
			argv[argc] = &Buf[pointer];			//add a parameter for every space
			argc++;
		}
		else{
			pointer++;
		}
	}//while
	
	return num;
}
