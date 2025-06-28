/*
**********************************************************************
*                          Micrium, Inc.
*                      949 Crestview Circle
*                     Weston,  FL 33327-1848
*
*                            uC/FS
*
*             (c) Copyright 2001 - 2003, Micrium, Inc.
*                      All rights reserved.
*
***********************************************************************

----------------------------------------------------------------------
File        : api_misc.c
Purpose     : Misc. API functions
----------------------------------------------------------------------
Known problems or limitations with current version
----------------------------------------------------------------------
None.
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*             #include Section
*
**********************************************************************
*/

#include "general.h"

#include "fsapi.h"
#include "fs_os.h"
#include "fs_fsl.h"
#include "fs_int.h"
#include "api_int.h"
#include "fs_clib.h"

#if FS_USE_FAT_FSL
#include "fs_fat.h"
#endif

extern void WDT_Reset_Count(void);


/*********************************************************************
*
*             #define constants
*
**********************************************************************
*/

#define FS_VALID_MODE_NUM (sizeof(_FS_valid_modes) / sizeof(_FS_mode_type))


/*********************************************************************
*
*             Local data types
*
**********************************************************************
*/

typedef struct
{
    FARCHARPTR mode;
    u8 mode_r;	// mode READ
    u8 mode_w;	// mode WRITE
    u8 mode_a;	// mode APPEND
    u8 mode_c;	// mode CREATE
    u8 mode_b;	// mode BINARY
}
_FS_mode_type;


/*********************************************************************
*
*             Local variables
*
**********************************************************************
*/

static const _FS_mode_type _FS_valid_modes[] =
{
    //       READ  WRITE  APPEND  CREATE  BINARY
    { "r"   ,  1,    0,     0,       0,     0 },
    { "w"   ,  0,    1,     0,       1,     0 },
    { "a"   ,  0,    1,     1,       1,     0 },
    { "rb"  ,  1,    0,     0,       0,     1 },
    { "wb"  ,  0,    1,     0,       1,     1 },
    { "ab"  ,  0,    1,     1,       1,     1 },
    { "r+"  ,  1,    1,     0,       0,     0 },
    { "w+"  ,  1,    1,     0,       1,     0 },
    { "w-"  ,  0,    1,     0,       1,     0 }, //Lucian: New function. No scan files in directory.
    { "a+"  ,  1,    1,     1,       1,     0 },
    { "r+b" ,  1,    1,     0,       0,     1 },
    { "rb+" ,  1,    1,     0,       0,     1 },
    { "w+b" ,  1,    1,     0,       1,     1 },
    { "wb+" ,  1,    1,     0,       1,     1 },
    { "a+b" ,  1,    1,     1,       1,     1 },
    { "ab+" ,  1,    1,     1,       1,     1 }
};

static const unsigned int _FS_maxopen = FS_MAXOPEN;
static FS_FILE _FS_filehandle[FS_MAXOPEN];

/*********************************************************************
*
*             External function prototype
*
**********************************************************************
*/
/*********************************************************************
*
*             Global functions
*
**********************************************************************
*/

/*********************************************************************
*
*             FS__find_fsl
*
  Description:
  FS internal function. Find correct index in the device information
  table referred by FS__pDevInfo for a given fully qualified name.

  Parameters:
  pFullName   - Fully qualified name.
  		Input "device-name:unit:\\directory\\directory\\file-name"
		e.g. "ram:0:\\dir1\\dir2",
		     "ram:0:\\dir1\\dir2\\test.txt"
  pFilename   - Address of a pointer, which is modified to point to
                the file name part of pFullName.
		Output "unit:\\directory\\directory\\file-name"
		e.g. "0:\\dir1\\dir2",
		     "0:\\dir1\\dir2\\test.txt"

  Return value:
  <0          - Unable to find the device.
  >=0         - Index of the device in the device information table.
  		Index of device-name.
  		e.g. 0
*/

int FS__find_fsl(const char *pFullName, FARCHARPTR *pFileName)
{
    int idx, i, j, m;
    FARCHARPTR s;

    // Find correct FSL (device:unit:name)
    s = (FARCHARPTR)FS__CLIB_strchr(pFullName, ':');
    if (s)
    {
        // Scan for device name
        idx = 0;
        m = (int)((u32)(s) - (u32)(pFullName));
        while (1)
        {
            WDT_Reset_Count();
            j = FS__CLIB_strlen(FS__pDevInfo[idx].devname);
            if (m > j)
            {
                // cytsai: might be (m - j) spaces
                j = m;
            }
            i = FS__CLIB_strncmp(FS__pDevInfo[idx].devname, pFullName, j);
            idx++;
            if (idx >= (int)FS__maxdev)
            {
                break;  // End of device information table reached
            }
            if (i == 0)
            {
                break;  // Device found
            }
        }
        if (i == 0)
        {
            idx--;  // Correct index
        }
        else
        {
            return -1;  // Device not found
        }
        s++;
    }
    else
    {
        /* use 1st FSL as default */
        idx = 0;
        s = (FARCHARPTR) pFullName;
    }
    *pFileName = s;

    return idx;
}


/*********************************************************************
*
*             FS_FOpen
*
  Description:
  API function. Open an existing file or create a new one.

  Parameters:
  pFileName   - Fully qualified file name.
  pMode       - Mode for opening the file.

  Return value:
  ==0         - Unable to open the file.
  !=0         - Address of an FS_FILE data structure.
*/

FS_FILE *FS_FOpen(const char *pFileName, const char *pMode)
{
    FARCHARPTR s;
    u32 i;
    int result, idx, j, c;

    // Find correct FSL  (device:unit:name)
    idx = FS__find_fsl(pFileName, &s);
    if (idx < 0)
    {
        ERRD(FS_DEVICE_FIND_ERR);
        return 0;  // Device not found
    }

    if(!FS__pDevInfo[idx].fs_ptr->fsl_fopen)
    {
        ERRD(FS_FUNC_PRT_ASSIGN_ERR);
        return 0;
    }

    //  Find next free entry in _FS_filehandle
    FS_X_OS_LockFileHandle();
    i = 0;
    while (1)
    {
        if (i >= _FS_maxopen)
        {
            ERRD(FS_FILE_OVER_OPEN_ERR);
            break;  // No free entry found.
        }
        if (!_FS_filehandle[i].inuse)
        {
            break;  // Unused entry found
        }
        i++;
    }
    if (i < _FS_maxopen)
    {
        // Check for valid mode string and set flags in file handle
        j = 0;
        while (1)
        {
            if (j >= FS_VALID_MODE_NUM)
            {
                break;  // Not in list of valid modes
            }
            c = FS__CLIB_strcmp(pMode, _FS_valid_modes[j].mode);
            if (c == 0)
            {
                break;  // Mode found in list
            }
            j++;
        }
        if (j < FS_VALID_MODE_NUM)
        {
            // Set mode flags according to the mode string
            _FS_filehandle[i].mode_r = _FS_valid_modes[j].mode_r;
            _FS_filehandle[i].mode_w = _FS_valid_modes[j].mode_w;
            _FS_filehandle[i].mode_a = _FS_valid_modes[j].mode_a;
            _FS_filehandle[i].mode_c = _FS_valid_modes[j].mode_c;
            _FS_filehandle[i].mode_b = _FS_valid_modes[j].mode_b;
        }
        else
        {
            FS_X_OS_UnlockFileHandle();
            return 0;
        }
        _FS_filehandle[i].dev_index = idx;
        // Execute the FSL function
        result = (FS__pDevInfo[idx].fs_ptr->fsl_fopen)(s, pMode, &_FS_filehandle[i]);
        FS_X_OS_UnlockFileHandle();
        if(result < 0)
            return NULL;
        return &_FS_filehandle[i];
    }
    FS_X_OS_UnlockFileHandle();
    return NULL;
}


/*********************************************************************
*
*             FS_FClose
*
  Description:
  API function. Close a file referred by pFile.

  Parameters:
  pFile       - Pointer to a FS_FILE data structure.

  Return value:
  None.
*/

int FS_FClose(FS_FILE *pFile)
{
    int result;

    if(!pFile)
    {
        ERRD(FS_PARAM_PTR_EXIST_ERR);
        return -1;  // No pointer to a FS_FILE structure
    }

    if(!pFile->inuse)
    {
        ERRD(FS_FILE_PTR_USE_ERR);
        return -1;	// The FS_FILE structure is not in use
    }

    if (pFile->dev_index < 0)
    {
        ERRD(FS_DEVICE_FIND_ERR);
        return -1;
    }

    if(!FS__pDevInfo[pFile->dev_index].fs_ptr->fsl_fclose)
    {
        ERRD(FS_FUNC_PRT_ASSIGN_ERR);
        return -1;
    }

    FS_X_OS_LockFileHandle();
    result = (FS__pDevInfo[pFile->dev_index].fs_ptr->fsl_fclose)(pFile);
    /*CY 0907*/
    FS_X_OS_UnlockFileHandle();
    return result;
}

/*********************************************************************
*
*             FS_FRead
*
  Description:
  API function. Read data from a file.

  Parameters:
  pData       - Pointer to a data buffer for storing data transferred
                from file.
  Size        - Size of an element to be transferred from file to data
                buffer
  N           - Number of elements to be transferred from the file.
  pFile       - Pointer to a FS_FILE data structure.

  Return value:
  Number of elements read.
*/

int FS_FRead(FS_FILE *pFile, void *pData, u32 dataSize, u32 *pReadSize)
{
    u32 result;

    if (!pFile)
    {
        ERRD(FS_PARAM_PTR_EXIST_ERR);
        return -1;  // No pointer to a FS_FILE structure
    }

    if (!pFile->mode_r)
    {
        // File open mode does not allow read ops
        pFile->error = FS_ERR_WRITEONLY;
        return -1;
    }

    if (pFile->dev_index < 0)
    {
        ERRD(FS_DEVICE_FIND_ERR);
        return -1;
    }

    if(!FS__pDevInfo[pFile->dev_index].fs_ptr->fsl_fread)
    {
        ERRD(FS_FUNC_PRT_ASSIGN_ERR);
        return -1;
    }

    // Execute the FSL function
    FS_X_OS_LockFileOp(pFile);
    result = (FS__pDevInfo[pFile->dev_index].fs_ptr->fsl_fread)(pFile, pData, dataSize, pReadSize);
    FS_X_OS_UnlockFileOp(pFile);
    return result;
}

/*********************************************************************
*
*             FS_FWrite
*
  Description:
  API function. Write data to a file.

  Parameters:
  pData       - Pointer to a data to be written to a file.
  Size        - Size of an element to be transferred.
  N           - Number of elements to be transferred to the file.
  pFile       - Pointer to a FS_FILE data structure.

  Return value:
  Number of elements written.
*/

int FS_FWrite(FS_FILE *pFile, const void *pData, u32 dataSize, u32 *pWriteSize)
{
    int i;

    if (!pFile)
    {
        ERRD(FS_PARAM_PTR_EXIST_ERR);
        return -1; // No pointer to a FS_FILE structure
    }

#if FS_DEBUG_ENA
    gpioDebugIRQ(1);
#endif

    FS_X_OS_LockFileOp(pFile);
    if (!pFile->mode_w)
    {
        // Open mode does now allow write access
        pFile->error = FS_ERR_READONLY;
        FS_X_OS_UnlockFileOp(pFile);
        return -1;
    }
    i = 0;
    if(pFile->dev_index < 0)
    {
        ERRD(FS_DEVICE_FIND_ERR);
        FS_X_OS_UnlockFileOp(pFile);
        return -1;
    }

    if(!FS__pDevInfo[pFile->dev_index].fs_ptr->fsl_fwrite)
    {
        ERRD(FS_FUNC_PRT_ASSIGN_ERR);
        FS_X_OS_UnlockFileOp(pFile);
        return -1;
    }
    // Execute the FSL function
    i = (FS__pDevInfo[pFile->dev_index].fs_ptr->fsl_fwrite)(pFile, pData, dataSize, pWriteSize);
    FS_X_OS_UnlockFileOp(pFile);

#if FS_DEBUG_ENA
    gpioDebugIRQ(0);
#endif

    return i;
}


/*********************************************************************
*
*             FS_Remove
*
  Description:
  API function. Remove a file.
  There is no real 'delete' function in the FSL, but the FSL's 'open'
  function can delete a file.

  Parameters:
  pFileName   - Fully qualified file name.

  Return value:
  ==0         - File has been removed.
  ==-1        - An error has occured.
*/

int FS_Remove(FS_DIR *pDir, char *pFileName, FS_DeleteCondition *pCondition)
{
	switch(pCondition->DeleteMode)
	{
		case FS_E_DELETE_TYPE_AUTO:
		case FS_E_DELETE_TYPE_ORDER:
			return FSFATFileDelete(pDir, pFileName, pCondition);
		case FS_E_DELETE_TYPE_DIR:
			return FSFATDirDelete(pDir);
		default: return 0;
	}
}

/*********************************************************************
*
*             FS_IoCtl
*
  Description:
  API function. Execute device command.

  Parameters:
  pDevName    - Fully qualified directory name.
  Cmd         - Command to be executed.
  Aux         - Parameter depending on command.
  pBuffer     - Pointer to a buffer used for the command.

  Return value:
  Command specific. In general a negative value means an error.
*/

int FS_IoCtl(const char *pDevName, s32 Cmd, s32 Aux, void *pBuffer)
{
    int idx, unit;
    FARCHARPTR s;
    FARCHARPTR t;

    idx = FS__find_fsl(pDevName, &s);
    if (idx < 0)
    {
        ERRD(FS_DEVICE_FIND_ERR);
        return -1;  // Device not found
    }

    if(!FS__pDevInfo[idx].fs_ptr->fsl_ioctl)
    {
        ERRD(FS_FUNC_PRT_ASSIGN_ERR);
        return -1;
    }

    t = FS__CLIB_strchr(s, ':');  // Find correct unit (unit:name)
    if (t)
    {
        unit = FS__CLIB_atoi(s);  // Scan for unit number
    }
    else
    {
        unit = 0;  // Use 1st unit as default
    }

    // delay for unknown bug on eBELL SD Card Format
    OSTimeDly(1);
    // Execute the FSL function
    return (FS__pDevInfo[idx].fs_ptr->fsl_ioctl)(idx, unit, Cmd, Aux, pBuffer);
}


/*********************************************************************
*
*             FS_FSeek
*
  Description:
  API function. Set current position of a file pointer.
  FS_fseek does not support to position the fp behind end of a file.

  Parameters:
  pFile       - Pointer to a FS_FILE data structure.
  Offset      - Offset for setting the file pointer position.
  Whence      - Mode for positioning the file pointer.

  Return value:
  ==0         - File pointer has been positioned according to the
                parameters.
  ==-1        - An error has occured.
*/

int FS_FSeek(FS_FILE *pFile, s32 Offset, int Whence)
{
    s32 value;

    if (!pFile)
    {
        ERRD(FS_PARAM_PTR_EXIST_ERR);
        return -1;
    }

    pFile->error = FS_ERR_OK;	// Clear any previous error
    pFile->CurClust = 0;	// Invalidate current cluster
    if (Whence == FS_SEEK_SET)
    {
        if (Offset <= pFile->size)
        {
            pFile->filepos = Offset;
        }
        else
        {
            // New position would be behind EOF
            pFile->error = FS_ERR_INVALIDPAR;
            return -1;
        }
    }
    else if (Whence == FS_SEEK_CUR)
    {
        value = pFile->filepos + Offset;
        if (value <= pFile->size)
        {
            pFile->filepos += Offset;
        }
        else
        {
            // New position would be behind EOF
            pFile->error = FS_ERR_INVALIDPAR;
            return -1;
        }
    }
    else if (Whence == FS_SEEK_END)
    {
        // The file system does not support this
        pFile->error = FS_ERR_INVALIDPAR;
        return -1;
    }
    else
    {
        // Parameter 'Whence' is invalid
        pFile->error = FS_ERR_INVALIDPAR;
        return -1;
    }
    return 0;
}


/*********************************************************************
*
*             FS_FTell
*
  Description:
  API function. Return position of a file pointer.

  Parameters:
  pFile       - Pointer to a FS_FILE data structure.

  Return value:
  >=0         - Current position of the file pointer.
  ==-1        - An error has occured.
*/

int FS_FTell(FS_FILE *pFile)
{
    if (!pFile)
    {
        ERRD(FS_PARAM_PTR_EXIST_ERR);
        return -1;
    }
    return pFile->filepos;

}


/*********************************************************************
*
*             FS_FError
*
  Description:
  API function. Return error status of a file.

  Parameters:
  pFile       - Pointer to a FS_FILE data structure.

  Return value:
  ==FS_ERR_OK - No error.
  !=FS_ERR_OK - An error has occured.
*/

int FS_FError(FS_FILE *pFile)
{
    if (!pFile)
    {
        ERRD(FS_PARAM_PTR_EXIST_ERR);
        return -1;
    }
    return pFile->error;
}


/*********************************************************************
*
*             FS_ClearErr
*
  Description:
  API function. Clear error status of a file.

  Parameters:
  pFile       - Pointer to a FS_FILE data structure.

  Return value:
  None.
*/

void FS_ClearErr(FS_FILE *pFile)
{
    if (!pFile)
    {
        ERRD(FS_PARAM_PTR_EXIST_ERR);
        return;
    }
    pFile->error = FS_ERR_OK;
}


/*********************************************************************
*
*             FS_Init
*
  Description:
  API function. Start the file system.

  Parameters:
  None.

  Return value:
  ==0         - File system has been started.
  !=0         - An error has occured.
*/

int FS_Init(void)
{
    int x;

    x = FS_X_OS_Init();  /* Init the OS, e.g. create semaphores  */
#if FS_USE_FAT_FSL
    if (x == 0)
    {
    	FSFATClustSemEvt = OSSemCreate(1);
    	FS__fat_block_init(); /* Init the FAT layers memory pool */
    	FSMCacheBufInit();
    }
#endif
    return x;
}

/*********************************************************************
*
*             FS_Exit
*
  Description:
  API function. Stop the file system.

  Parameters:
  None.

  Return value:
  ==0         - File system has been stopped.
  !=0         - An error has occured.
*/

int FS_Exit(void)
{
#if FS_USE_FAT_FSL
	u8 err;
	FSFATClustSemEvt = OSSemDel(FSFATClustSemEvt, OS_DEL_ALWAYS, &err);
#endif
    return FS_X_OS_Exit();
}


