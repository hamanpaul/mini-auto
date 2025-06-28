/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	usbclass.c

Abstract:

   	USB class routine.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#include "general.h"

#include "task.h"
#include "board.h"
#include "usb.h"
#include "usbdev.h"
#include "usbdesc.h"
#include "usbclass.h"
#include "usbmsc.h"


/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

s32 usbClassCfg(void);
s32 usbClassInit(void);

extern s32 usbMscInit(void);
extern s32 usbVcInit(void);

/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */

/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */

u8 usbClass = USB_MASS_STORAGE_CLASS;

extern USB_DEV_SETTING usbDevSetting;

//extern USB_DEV_DESC usb_msc_dev_desc;
//extern USB_DEV_QUAL_DESC usb_msc_dev_qual_desc;
//extern USB_MSC_CONFIGURATION_DESC usb_msc_configuration_desc;
//extern USB_DEV_DESC usb_vc_dev_desc;
//extern USB_DEV_QUAL_DESC usb_vc_dev_qual_desc;
//extern USB_VC_CONFIGURATION_DESC usb_vc_configuration_desc;
extern u8 usb_str_desc0[];
extern u8 usb_str_desc1[];
extern u8 usb_str_desc2[];
extern u8 usb_str_desc3[];

/*
 *********************************************************************************************************
 * Function body
 *********************************************************************************************************
 */

/*

Routine Description:

	Configure USB Class.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbClassCfg(void)
{
    usbClass = USB_MASS_STORAGE_CLASS;

    memset((void *)&usbDevSetting, 0, sizeof(USB_DEV_SETTING));
    switch (usbClass)
    {
    case USB_MASS_STORAGE_CLASS:
        /* set parameters of mass storage class */
        usbDevSetting.numCfg = USB_MSC_DEV_bNumConfigurations;
        usbDevSetting.numIf = USB_MSC_CFG_bNumInterfaces;
        usbDevSetting.numEp = USB_MSC_IF0_ALT0_bNumEndpoints;
        usbDevSetting.cfgNum = USB_MSC_CFG_bConfigurationValue;
        usbDevSetting.ifNum = USB_MSC_IF0_ALT0_bInterfaceNumber;
        usbDevSetting.altSet[usbDevSetting.ifNum] = USB_MSC_IF0_ALT0_bAlternateSetting;
        usbDevSetting.devStat = USB_DEV_STAT_SELF_POWERED | USB_DEV_STAT_REMOTE_WAKEUP;
        break;

    case USB_VIDEO_CLASS:
        /* set parameters of video class */
        usbDevSetting.numCfg = USB_VC_DEV_bNumConfigurations;
        usbDevSetting.numIf = USB_VC_CFG_bNumInterfaces;
        usbDevSetting.numEp = USB_VC_IF0_ALT0_bNumEndpoints;
        usbDevSetting.cfgNum = USB_VC_CFG_bConfigurationValue;
        usbDevSetting.ifNum = USB_VC_IF0_ALT0_bInterfaceNumber;
        usbDevSetting.altSet[usbDevSetting.ifNum] = USB_VC_IF0_ALT0_bAlternateSetting;
        usbDevSetting.devStat = USB_DEV_STAT_SELF_POWERED | USB_DEV_STAT_REMOTE_WAKEUP;
        break;

    default:
        /* error class */
        return 0;
    }

    return 1;
}

/*

Routine Description:

	Initialize USB Class.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbClassInit(void)
{
    switch (usbClass)
    {
    case USB_MASS_STORAGE_CLASS:
        /* initialize mass storage class */
        usbMscInit();
        break;

    case USB_VIDEO_CLASS:
        /* initialize video class */
        //usbVcInit();
        break;

    default:
        /* error class */
        return 0;
    }

    return 1;
}

/*

Routine Description:

	USB class get device descriptor.

Arguments:

	pData - Descriptor data.
	pSize - Descriptor size.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbClassGetDevDesc(u32* pData, u32* pSize)
{
    switch (usbClass)
    {
    case USB_MASS_STORAGE_CLASS:
        /* mass storage class */
      //  *pData = (u32) &usb_msc_dev_desc;
      //  *pSize = sizeof(USB_DEV_DESC);
        break;

    case USB_VIDEO_CLASS:
        /* video class */
       // *pData = (u32) &usb_vc_dev_desc;
       // *pSize = sizeof(USB_DEV_DESC);
        break;

    default:
        /* error class */
        return 0;
    }

    return 1;
}

/*

Routine Description:

	USB class get configuration descriptor.

Arguments:

	cfgNum - Configuration number.
	pData - Descriptor data.
	pSize - Descriptor size.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbClassGetCfgDesc(u8 cfgNum, u32* pData, u32* pSize)
{
    switch (usbClass)
    {
    case USB_MASS_STORAGE_CLASS:
        /* mass storage class */
        if (cfgNum < USB_MSC_DEV_bNumConfigurations)
        {
      //      *pData = (u32) &usb_msc_configuration_desc;
       //     *pSize = sizeof(USB_MSC_CONFIGURATION_DESC);
        }
        else
        {
            /* error configuration number */
            return 0;
        }
        break;

    case USB_VIDEO_CLASS:
        /* video class */
        if (cfgNum < USB_VC_DEV_bNumConfigurations)
        {
       //     *pData = (u32) &usb_vc_configuration_desc;
        //    *pSize = sizeof(USB_VC_CONFIGURATION_DESC);
        }
        else
        {
            /* error configuration number */
            return 0;
        }
        break;

    default:
        /* error class */
        return 0;
    }

    return 1;
}

/*

Routine Description:

	USB class get string descriptor.

Arguments:

	strNum - String number.
	pData - Descriptor data.
	pSize - Descriptor size.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbClassGetStrDesc(u8 strNum, u32* pData, u32* pSize)
{
    switch (usbClass)
    {
    case USB_MASS_STORAGE_CLASS:
    case USB_VIDEO_CLASS:
        /* mass storage class and video class */
        switch (strNum)
        {
        case 0:
            *pData = (u32) usb_str_desc0;
            *pSize = USB_STR0_bLength;
            break;

        case 1:
            *pData = (u32) usb_str_desc1;
            *pSize = USB_STR1_bLength;
            break;

        case 2:
            *pData = (u32) usb_str_desc2;
            *pSize = USB_STR2_bLength;
            break;

        case 3:
            *pData = (u32) usb_str_desc3;
            *pSize = USB_STR3_bLength;
            break;

        default:
            /* error string number */
            return 0 ;
        }
        break;

    default:
        /* error class */
        return 0;
    }

    return 1;
}

/*

Routine Description:

	USB class get device qualifier descriptor. For USB revision 2.0 only.

Arguments:

	pData - Descriptor data.
	pSize - Descriptor size.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbClassGetDevQualDesc(u32* pData, u32* pSize)
{
    /* TBD: check if high-speed device */
    switch (usbClass)
    {
    case USB_MASS_STORAGE_CLASS:
        /* mass storage class */
      //  *pData = (u32) &usb_msc_dev_qual_desc;
     //   *pSize = sizeof(USB_DEV_QUAL_DESC);
        break;

    case USB_VIDEO_CLASS:
        /* video class */
  //      *pData = (u32) &usb_vc_dev_qual_desc;
  //      *pSize = sizeof(USB_DEV_QUAL_DESC);
        break;

    default:
        /* error class */
        return 0;
    }

    return 1;
}

/*

Routine Description:

	USB class get other speed configuration descriptor. For USB revision 2.0 only.

Arguments:

	cfgNum - Configuration number.
	pData - Descriptor data.
	pSize - Descriptor size.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbClassGetOtherSpeedCfgDesc(u8 cfgNum, u32* pData, u32* pSize)
{
    /* TBD: check if high-speed device */
    switch (usbClass)
    {
    case USB_MASS_STORAGE_CLASS:
        /* mass storage class */
        if (cfgNum < USB_MSC_DEV_QUAL_bNumConfigurations)
        {
            /* TBD: set to other speed configuration */
        //    *pData = (u32) &usb_msc_configuration_desc;
         //   *pSize = sizeof(USB_MSC_CONFIGURATION_DESC);
        }
        else
        {
            /* error configuration number */
            return 0;
        }
        break;

    case USB_VIDEO_CLASS:
        /* video class */
        if (cfgNum < USB_VC_DEV_QUAL_bNumConfigurations)
        {
            /* TBD: set to other speed configuration */
       //     *pData = (u32) &usb_vc_configuration_desc;
       //     *pSize = sizeof(USB_VC_CONFIGURATION_DESC);
        }
        else
        {
            /* error configuration number */
            return 0;
        }
        break;

    default:
        /* error class */
        return 0;
    }

    return 1;
}
