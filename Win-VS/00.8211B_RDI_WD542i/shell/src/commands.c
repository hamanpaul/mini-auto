/*	 
 *	commands.c
 *	the genie shell command part for ucosII
 *	under skyeye
 *
 *	Bugs report:	 ganganwen@163.com
 *	Last modified:	 2005-04-25 
 *  	changed by: wenjia
 */

#include	"general.h"
#include	"osapi.h"
#include	"commands.h"
#include	"shelltask.h"
#include	"fsapi.h"


command ShellComms[MAX_COMMAND_NUM];

FS_FILE *myfile;
char mybuffer[0x100];

/* cytsai: mass storage */
char CurDir[64]="\\";		//show current dir
char TargetVal[64] = "\\";  

/*CY 1023*/
int storageType = STORAGE_MEMORY_RAMDISK;
char CurDrive[64];
char CurPath[64];
char TargetPath[64];

INT8U DirExist(const char *DirName);
void RewindRoot(void);


void CreateTargetVal(const char *arg);
void CreateCurPath(void);
void CreateTargetPath(void);

INT8U DirExist(const char *DirName)
{
    	FS_DIR *dirp;
    	dirp = FS_OpenDir(DirName);
    	if (dirp) {
       		FS_CloseDir(dirp);
       		// _log("Open %s Exist!\n", DirName);
        	return 1;
    	} 
    	else {
       		// _log("The %s Dir is not exist!\n", DirName);
        	return 0;
    	}
} 

//--------------------------------------------
//#define CUR_DRI_MAX_LEN 32
//FS_DIR CurDir[CUR_DRI_MAX_LEN 32];
// struct FS_DIRENT *direntp;
/*********************************************************************
*
*             _write_file
*
  This routine demonstrates, how to create and write to a file
  using the file system.
*/

static void _write_file(const char *name, const char *txt) 
{
  	int x;
  
  	/* create file */
  	myfile = FS_FOpen(name,"w");
  	if (myfile) {
    		/* write to file */
    		x = FS_FWrite(txt,1,strlen(txt),myfile);
    		/* all data written ? */
    		if (x!=(int)strlen(txt)) {
      			/* check, why not all data was written */
      			x = FS_FError(myfile);
      			sprintf(mybuffer,"Not all bytes written because of error %d.\n\n",x);
      			_error(mybuffer);
    		}
    		/* close file */
    		FS_FClose(myfile);
		_log("\n\n");
  	}
  	else {
    		sprintf(mybuffer,"Unable to create file %s\n\n",name);
    		_error(mybuffer);
  	}
}


/*********************************************************************
*
*             _dump_file
*
  This routine demonstrates, how to open and read from a file using 
  the file system.
*/

static void _dump_file(const char *name) 
{
  	int x;

  	/* open file */
  	myfile = FS_FOpen(name,"r");
  	if (myfile) {
    		/* read until EOF has been reached */
    		do {
      			x = FS_FRead(mybuffer,1,sizeof(mybuffer)-1,myfile);
      			mybuffer[x]=0;
      			if (x) {
        			_log(mybuffer);
      			}
    		} while (x);
    		/* check, if there is no more data, because of EOF */
    		x = FS_FError(myfile);
    		if (x!=FS_ERR_EOF) {
      			/* there was a problem during read operation */
      			sprintf(mybuffer,"Error %d during read operation.\n",x);
      			_error(mybuffer);
    		}
    		/* close file */
    		FS_FClose(myfile);
		_log("\n\n");
  	}
  	else {
    		sprintf(mybuffer,"Unable to open file %s.\n",name);
    		_error(mybuffer);
  	}
}


/*********************************************************************
*
*             _show_directory
*
  This routine demonstrates, how to read a directory.
*/

#if FS_POSIX_DIR_SUPPORT

static void _show_directory(const char *name) 
{
  	FS_DIR *dirp;
 	struct FS_DIRENT *direntp;
		
	dirp = FS_OpenDir(name);
  	if (dirp) {
    		do {
      			direntp = FS_ReadDir(dirp);
      			if (direntp) {
       				sprintf(mybuffer,"%s\n",direntp->d_name);
        			_log(mybuffer);
      			}
    		} while (direntp);
    		FS_CloseDir(dirp);
		_log("\n");
  	}
  	else {
    		_error("Unable to open directory\n");
  	}
}

#endif /* FS_POSIX_DIR_SUPPORT */


/*********************************************************************
*
*             _show_free
*
  This routine demonstrates, how to read disk space information.
*/

static void _show_free(const char *device) {
  	FS_DISKFREE_T disk_data;
  	int x;

  	_log("Disk information of ");
  	_log(device);
  	_log("\n");
  	x = FS_IoCtl(device,FS_CMD_GET_DISKFREE,0,(void*) &disk_data);
  	if (x==0) {
    		sprintf(mybuffer,"total clusters     : %lu\navailable clusters : %lu\nsectors/cluster    : %u\nbytes per sector   : %u\n",
          		disk_data.total_clusters, disk_data.avail_clusters, disk_data.sectors_per_cluster, disk_data.bytes_per_sector);
    	_log(mybuffer);
  	} 
  	else {
    		_error("Invalid drive specified\n\n");
  	}
}

/*********************************************************************
*
*             InitPath
*
  This routine init shell
*/
INT8U InitPath()
{
	/*CY 1023*/
	switch (storageType)
	{
		case STORAGE_MEMORY_RAMDISK:
			strcpy((char*)CurDrive, "ram:0:");
			strcpy((char*)CurPath, "ram:0:\\");
			strcpy((char*)TargetPath, "ram:0:\\"); 
			break;
			
		case STORAGE_MEMORY_SD_MMC:
			strcpy((char*)CurDrive, "sdmmc:0:");
			strcpy((char*)CurPath, "sdmmc:0:\\");
			strcpy((char*)TargetPath, "sdmmc:0:\\"); 
			break;
			
		case STORAGE_MEMORY_SMC_NAND:
			strcpy((char*)CurDrive, "smc:0:");
			strcpy((char*)CurPath, "smc:0:\\");
			strcpy((char*)TargetPath, "smc:0:\\"); 
			break;
	}
	
	return 0;
}	

/*********************************************************************
*
*             InitCommands
*
  This routine init shell
*/
void InitCmd(void)
{
    	INT8U TmpCnt = MAX_COMMAND_NUM;
    	while (TmpCnt--)
    	{
        	ShellComms[TmpCnt].CommandFunc = NULL;
        	ShellComms[TmpCnt].name = NULL;
        	ShellComms[TmpCnt].num  = 0;
    	}
}

INT8U InitCommands()
{
    	InitCmd();
	ShellComms[0].num = 0;
	ShellComms[0].name = "ls";
	ShellComms[0].CommandFunc = lsFunc;

	ShellComms[1].num = 1;
	ShellComms[1].name = "rm";
	ShellComms[1].CommandFunc = rmFunc;

	ShellComms[2].num = 2;
	ShellComms[2].name = "write";
	ShellComms[2].CommandFunc = writeFunc;

	ShellComms[3].num = 3;
	ShellComms[3].name = "cat";
	ShellComms[3].CommandFunc = catFunc;

	ShellComms[4].num = 4;
	ShellComms[4].name = "format";
	ShellComms[4].CommandFunc = formatFunc;

	ShellComms[5].num = 5;
	ShellComms[5].name = "mkdir";
	ShellComms[5].CommandFunc = mkdirFunc;

	ShellComms[6].num = 6;
	ShellComms[6].name = "rmdir";
	ShellComms[6].CommandFunc = rmdirFunc;

	ShellComms[7].num = 7;
	ShellComms[7].name = "free";
	ShellComms[7].CommandFunc = freeFunc;

	ShellComms[8].num = 8;
	ShellComms[8].name = "help";
	ShellComms[8].CommandFunc = helpFunc;

	ShellComms[9].num = 9;
	ShellComms[9].name = "cd";
	ShellComms[9].CommandFunc = cdFunc;

	ShellComms[10].num = 10;
	ShellComms[10].name = "pwd";
	ShellComms[10].CommandFunc = pwdFunc;


    	ShellComms[11].num = 11;
	ShellComms[11].name = "exit";
	ShellComms[11].CommandFunc = ExitFunc;
        
   	ShellComms[12].num = 12;
	ShellComms[12].name = "show";
	ShellComms[12].CommandFunc = ShowFunc;

	/* cytsai: mass storage */
	ShellComms[13].num = 13;
	ShellComms[13].name = "drive";
	ShellComms[13].CommandFunc = driveFunc;
	
	return 0;
}

INT8U driveFunc(INT8U argc,char **argv)
{
	INT8U DriveLen;
	
	if(argc != 1)
	{
		_error("please input command as:drive drivename");
		_error("\n\n");
		return 1;
	}
	
	 DriveLen = strlen(argv[0]);
	 strncpy(&CurDrive[0], argv[0], DriveLen);
	 RewindRoot();
	 CreateTargetVal("\\\0");
	 CreateTargetPath();
	 if (DirExist(TargetPath))
	 {
	 	//FS_IoCtl(CurDrive, FS_CMD_CHK_DSKCHANGE, 0, 0); /* check CurDrive periodically */
		_log("drive ");
		_log(argv[0]);
		_log(" successfully changed\n\n");
	}
	else
	{
		_error("drive changed failure! please check the drive name\n");
	}
	
	return 0;
}

INT8U ShowFunc(INT8U argc,char **argv)
{

    	_log("\n=========  TargetPath =  ");
	_log(TargetPath);
	_log("========\n");
    	return 0;
}

INT8U ExitFunc(INT8U argc,char **argv)
{
    	exit(0);    
    	return 0;
}


INT8U pwdFunc(INT8U argc,char **argv)
{
	if (argc != 0)
	{
		_error("PWD is single arg\n");
		return 1;
	}
	/* cytsai: mass storage */
	CreateCurPath();
	_log(CurPath);
	_log("\n");
	return 0;
}

void RewindRoot(void)
{
    	/* cytsai: mass storage */
    	CurDir[0] = '\\';
    	CurDir[1] = '\0';
}

INT8U cdFunc(INT8U argc,char **argv)
{
	INT8U DirCurPos = 0;
    	INT8U CmdLen    = 0;
	INT8U TmpVal    = 0;
    	INT8U IsRoot    = 0;
    	INT8U IsBackup  = 0;
    	INT8U BackupLen = 0;
    	
	DirCurPos = strlen(CurDir);   
    	if (argc != 1) {
		_error("please input command as:cd directoryname ");
        	_error("\n cd \\ change to root\n cd .. \n cd \\MyDir\\Dir1\n");
		_error("\n\n");
		return 1;
	}
	if (strcmp(argv[0], "..") == 0)	{
		if (1 == DirCurPos) {
#if EN_DEBUG == 1
			_error("This is root dir\n");
#endif
			return 1;
		} else {
			TmpVal = DirCurPos;   
		        while (TmpVal--) {
			   	if (CurDir[TmpVal] == '\\') {
				   	if (TmpVal == 0) {
					   	RewindRoot();
				   	} else {
				      		CurDir[TmpVal] = '\0';   
				   	}
#if EN_DEBUG  == 1
                   			_log("\n CurDir Len = %d\n", strlen(CurDir));
#endif
				   	break;    
               			} 
               			else {
                   			CurDir[TmpVal] = '\0';  
               			}
		   	}
		}
    	} else if (strcmp(argv[0], "\\") == 0) {   
		RewindRoot();   
	} else {
	    	if (strcmp(CurDir, "\\") == 0) {
            		IsRoot = 1;
            		if ('\\' == argv[0][0]) {
                		DirCurPos = 0;   
            		}
#if EN_DEBUG == 1
            		_log("\ncd start at root! DirCurPos = %d\n", DirCurPos);
#endif
		} 
		else {
            		if ('\\' == argv[0][0]) { 
                		IsBackup  = 1;
                		BackupLen = DirCurPos + 1;
                		strncpy(&TargetVal[0], &CurDir[0], BackupLen);
                		DirCurPos = 0;     
            		}
            		else {
		        	CurDir[DirCurPos] = '\\';
			    	DirCurPos++;
            		}
		}
        	CmdLen    = strlen(argv[0]);
        	strncpy(&CurDir[DirCurPos], argv[0], CmdLen + 1);  
#if EN_DEBUG == 1
        	_log("\nafter strncpy() CurDir= %s  DirCurPos = %d CmdLen = %d\n", CurDir, DirCurPos, CmdLen);
#endif
		
		CreateCurPath();
        	if (DirExist(CurPath) == 0)  
        	{
			_log(argv[0]);
            		_log("  Dir is not exist!\n");
            		if (1 == IsRoot) {
                		RewindRoot();               
            		} 
            		else {
                		if (1 == IsBackup) {
                    			strncpy(&CurDir[0], &TargetVal[0], BackupLen);
                		}
                		else {
                    			CurDir[DirCurPos - 1] = '\0';   
                		}
            		}
        	} 
        	else {
#if EN_DEBUG == 1
            		_log("Open %s Exist!\n", CurDir); 
#endif
        	}
	}
#if EN_DEBUG == 1
    	_log("\nCurDir = %s \n", CurDir);
#endif
	return 0;
}

INT8U lsFunc(INT8U argc,char **argv)
{			

	CreateCurPath();
	if(argc == 0)
    	{
        	_show_directory(CurPath);//_show_directory("");
    	} else {
        	_show_directory(argv[0]);
    	}	
	return 0;
}
		

INT8U rmFunc(INT8U argc,char **argv)
{

	if(argc != 1)
	{
		_error("please input command as:rm filename");
		_error("\n\n");
		return 1;
	}
    	CreateTargetVal(argv[0]);
    	CreateTargetPath();
    	_error("please input command as:rm filename2");
    	#if !FS_NEW_VERSION
    	if(!FS_Remove(TargetPath,0))
	{
		_log("remove ");
		_log(argv[0]);
		_log(" successful\n\n");
	}
	else
	{
		_error("remove failure!\n\n");
	}
	#endif
	return 0;
}

INT8U writeFunc(INT8U argc,char **argv)
{
	if(argc != 2)
	{
		_error("please input command as:create filename data");
		_error("\n\n");
		return 1;
	}
    	CreateTargetVal(argv[0]);
	CreateTargetPath();
	//_write_file(argv[0], argv[1]);
    	_write_file(TargetPath, argv[1]);

	return 0;
}


//CurDir + arg = TargetParam
void CreateTargetVal(const char *arg)
{
    	INT8U CurDirLen = 0;
    	//INT8U ArgIsStartRoot = 0;

    	CurDirLen = strlen(CurDir);
    	if ('\\' == arg[0])
  	{
     		//   ArgIsStartRoot  = 1;
        	strncpy(&TargetVal[0], arg, strlen(arg) + 1);
    	} 
    	else 
    	{
        	if (1 == CurDirLen)  
        	{
        		strncpy(&TargetVal[CurDirLen], arg, strlen(arg) + 1);
        	} 
        	else 
        	{
            		strncpy(TargetVal, CurDir, CurDirLen + 1);
            		TargetVal[CurDirLen++] = '\\';
#if EN_DEBUG == 1
            		_log("\n %s\n", TargetVal);
#endif
            		strncpy(&TargetVal[CurDirLen], arg, strlen(arg) + 1);
        	}
    	}
    	
#if EN_DEBUG == 1
    	_log("\nTargetVal = %s\n", TargetVal);
#endif
}

//CurDrive + TargetVal = TargetPath
void CreateCurPath(void)
{
    	INT8U CurDriveLen = 0;	
	
	CurDriveLen = strlen(CurDrive);
	strncpy(&CurPath[0], &CurDrive[0], CurDriveLen);
	strncpy(&CurPath[CurDriveLen], &CurDir[0], strlen(CurDir) + 1);
}

//CurDrive + TargetVal = TargetPath
void CreateTargetPath(void)
{
    	INT8U CurDriveLen = 0;	
	
	CurDriveLen = strlen(CurDrive);
	strncpy(&TargetPath[0], &CurDrive[0], CurDriveLen);
	strncpy(&TargetPath[CurDriveLen], &TargetVal[0], strlen(TargetVal) + 1);
}
	
INT8U catFunc(INT8U argc,char **argv)
{
    	if(argc != 1)
	{
		_error("please input command as:cat filename");
		_error("\n\n");
		return 1;
	}
    	CreateTargetVal(argv[0]);
    	CreateTargetPath();
    	_dump_file(TargetPath);
	return 0;
}

INT8U formatFunc(INT8U argc,char **argv)
{
	int x;

	/*CY 1023*/
    	switch (storageType)
    	{
    		case STORAGE_MEMORY_RAMDISK:
    			x = FS_IoCtl(CurDrive,FS_CMD_FORMAT_MEDIA,FS_MEDIA_RAM_16KB,0);
    			break;
    			
		case STORAGE_MEMORY_SD_MMC:  	
			x = FS_IoCtl(CurDrive,FS_CMD_FORMAT_MEDIA,FS_MEDIA_SD_512MB,0);
			break;
			
		case STORAGE_MEMORY_SMC_NAND:
    			x = FS_IoCtl(CurDrive,FS_CMD_FORMAT_MEDIA,FS_MEDIA_SMC_64MB,0);
    			break;
    	}		

    	if (x!=0) {
      		_error("Cannot format storage.\n\n");
	  	return 1;
	}
	else{
		_log("format successful\n\n");
		return 0;
	}
}

INT8U mkdirFunc(INT8U argc,char **argv)
{
	if(argc != 1)
	{
		_error("please input command as:mkdir filename");
		_error("\n\n");
		return 1;
	}
    	CreateTargetVal(argv[0]);
    	CreateTargetPath();
	//if(!FS_MkDir(argv[0]))	
    	if(!FS_MkDir(TargetPath))
	{
		_log("make directory ");
		_log(argv[0]);
		_log(" successful\n\n");
	}
	else
	{
		_error("mkdir failure! please check disk full or unformated\n");
	}
	return 0;
}

INT8U rmdirFunc(INT8U argc,char **argv)
{
	if(argc != 1)
	{
		_error("please input command as:mkdir filename");
		_error("\n\n");
		return 1;
	}
    	CreateTargetVal(argv[0]);
    	CreateTargetPath();
    	if(!FS_RmDir(TargetPath,1))
	{
		_log("remove ");
		_log(argv[0]);
		_log(" successful\n\n");
	}
	else
	{
		_error("remove failure! please check the directory name\n");
	}
	return 0;
}

INT8U freeFunc(INT8U argc,char **argv)
{
	/* cytsai: mass storage */
	_show_free(CurDrive);
	_log("\n");
	return 0;
}

INT8U helpFunc(INT8U argc,char **argv)
{
    	ShowHelp();
	return 0;
}

void ShowHelp(void)
{
	/* cytsai: mass storage */
	_log("\nformat - format disk\nmkdir  - make directory\nwrite  - write file\nrmdir  - remove directory\nls  \
   - list file/directory\ncat    - read file\nrm     - remove file\nfree   - show free space\ncd  \
   - change directory\nshow   - show TargetPath\npwd    - show current directory\ndrive  - change drive\nexit   - end\n");
}
