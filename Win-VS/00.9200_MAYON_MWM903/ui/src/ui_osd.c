/*

  Copyright (c) 2008 Mars Semiconductor Corp.

   Module Name:

   ui.c

   Abstract:

   The routines of user interface.

   Environment:

   ARM RealView Developer Suite

   Revision History:

   2010/12/27   Elsa Lee  Create

   */
#include "general.h"
#include "iduapi.h"
#include "uiapi.h"
#include "ui.h"
#include "ui_project.h"
/*
 *********************************************************************************************************
 *  Constant
 *********************************************************************************************************
 */
#define UI_OSD_MAX_W    60
#define UI_OSD_MAX_H    60
#define UI_ICON_MAX_W    100
#define UI_ICON_MAX_H    100


/*
 *********************************************************************************************************
 * Variables
 *********************************************************************************************************
 */
__align(4) u8 OSD_ICON[UI_OSD_MAX_W*UI_OSD_MAX_H];
__align(4) u8 OSDl_ICON[UI_ICON_MAX_W*UI_ICON_MAX_H];
u8 UI_OSD_4BIT_EN = 0;
/*
 *********************************************************************************************************
 * Extern Variables
 *********************************************************************************************************
 */

/*
 **********************************************************************************************************
 * External Functions
 **********************************************************************************************************
 */

/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */


/*
 *********************************************************************************************************
 * Function
 *********************************************************************************************************
 */


void uiMenuOSDShiftY(u16 osd_w , u16 *sy, u16 *ey , u8 shift_h, u8 buf_idx)
{
    u32 *addr;

    if (*sy < shift_h)
        return;

    //if (sysTVOutOnFlag)
    //    addr = (u32 *)GetTVOSDBufAdr(buf_idx);
    //else
        addr = (u32 *)uiGetOSDBufAdr(buf_idx);
    //word align
    osd_w >>= 2;

    //JYK
    if (UI_OSD_4BIT_EN)
        osd_w /= 2;

    memcpy(&addr[(*sy-shift_h)*osd_w], &addr[*sy*osd_w], ((*ey-*sy)*osd_w)*4);
    memset(&addr[(*ey - shift_h)*osd_w], 0, shift_h*osd_w*4);
    *sy = *sy - shift_h;
    *ey = *ey - shift_h;
}


/** Drop high-nibbles and merge low-nibbles into bytes. */
static void bytes_from_low_nibbles(u8 *addr, u16 n)
{
    //JYK
    #define _MERGE(a, b) (((b) << 4) & 0xF0) | ((a) & 0x0F)

    u8 *p, *q;
    u8 *start = addr;
    u8 *end = start + n;

    for (p=q=start; p<end; p+=2, q++)
        *q = _MERGE(*p, *(p+1));
}


static void uiMenuOSDPrintStr(u16 osd_w , u8 *font , u8 font_w , u8 font_h , u16 x_pos , u16 y_pos , u8 buf_idx)
{
    u32 i;
    u8  *pucDstAddr, *pucSrcAddr;

    if (font_w > (OsdBlkInfo[buf_idx].BlkEX - x_pos))
        return;
    if (font_h > (OsdBlkInfo[buf_idx].BlkEY - y_pos))
        return;

    pucDstAddr = (u8*)uiGetOSDBufAdr(buf_idx);
    pucSrcAddr = font;

    //JYK
    if (UI_OSD_4BIT_EN) {
        bytes_from_low_nibbles(font, font_w*font_h);
        osd_w /= 2;
        font_w /= 2;
        x_pos /= 2;
    }

    if((osd_w%4) !=0 )
        osd_w = ((osd_w - 1)/4)*4;

    pucDstAddr += y_pos * osd_w + x_pos;

    for (i=0; i<font_h; i++)
    {
        memcpy(pucDstAddr, pucSrcAddr, font_w);
        pucDstAddr += osd_w;
        pucSrcAddr += font_w;
    }
}

static u16 _uiGetOsdWidth(void)
{
#if OSD_BUFFER_CHANGE_SUPPORT
    return OSDDispWidth[1];

#else
    if(sysTVOutOnFlag)
        return TVOSD_SizeX;
    else
        return OSD_Width;
#endif

}

static void _uiOSDDrawStr(u16 osd_w, UI_MULT_LAN_STR* strInfo, u8 *strLib, u16 x_pos, u16 y_pos, u8 font_w, u8 font_h, u8 buf_idx, u8 str_color, u8 bg_color)
{
    u32 i, j;
    u8 *ptr, *pucFont, err;
    u32 unTotalSize;

    unTotalSize = font_w*font_h;
    OSSemPend(uiOSDSemEvt, OS_IPC_WAIT_FOREVER, &err);
    for (i=0; i<strInfo->Str_len;i++)
    {
        ptr = strLib+strInfo->StrIndex[i]*unTotalSize;
        pucFont = &OSD_ICON[0];
        for (j = 0; j < unTotalSize; j++)
        {
            if (*ptr == 0x3f)
                *pucFont = bg_color;
            else
                *pucFont = str_color;
            ptr++;
            pucFont++;
        }
        uiMenuOSDPrintStr(osd_w , OSD_ICON , font_w , font_h , x_pos , y_pos , buf_idx);
        x_pos+=font_w;
    }
    OSSemPost(uiOSDSemEvt);
}

/*
 *********************************************************************************************************
 * Called by UI in other file
 *********************************************************************************************************
 */

void _uiOSDIconColor(u16 osd_w , u8 *icon, u16 icon_w , u16 icon_h , u16 x_pos , u16 y_pos , u16 buf_idx)
{
    u32 *addr;
    u32 *data;
    u32 i,j;
    u32 sx, ex, tmp_sx, tmp_icon_x;

    data = (u32 *)icon;
    addr = (u32 *)uiGetOSDBufAdr(buf_idx);

    if (x_pos > OsdBlkInfo[buf_idx].BlkEX)
    {
        //DEBUG_RED("Icon x %d large than window x %d\n", x_pos, OsdBlkInfo[buf_idx].BlkEX);
        return;
    }
    if (y_pos > OsdBlkInfo[buf_idx].BlkEY)
    {
        //DEBUG_RED("Icon y %d large than window y %d\n", y_pos, OsdBlkInfo[buf_idx].BlkEY);
        return;
    }

    if (icon_w > (OsdBlkInfo[buf_idx].BlkEX - x_pos))
    {
        //DEBUG_RED("Icon x %d Out of Range %d\n", icon_w, (OsdBlkInfo[buf_idx].BlkEX - x_pos));
        icon_w = (OsdBlkInfo[buf_idx].BlkEX - x_pos);
    }
    if (icon_h > (OsdBlkInfo[buf_idx].BlkEY - y_pos))
    {
        //DEBUG_RED("Icon y %d Out of Range %d\n", icon_h, (OsdBlkInfo[buf_idx].BlkEY- y_pos));
        icon_h = (OsdBlkInfo[buf_idx].BlkEY - y_pos);
    }

    //JYK
    if (UI_OSD_4BIT_EN) {
        u8 *p = (u8*)uiGetOSDBufAdr(buf_idx);
        u8 *q = (u8*)icon;

        bytes_from_low_nibbles(icon, icon_w*icon_h);

        // byte align
        osd_w /= 2;
        icon_w /= 2;
        x_pos /= 2;

        p += y_pos * osd_w + x_pos;
        for (i=0; i<icon_h; i++) {
            memcpy(p, q, icon_w);
            p += osd_w;
            q += icon_w;
        }
        return;
    }

    //word align
    osd_w >>= 2;
    icon_w >>= 2;
    x_pos >>= 2;
    sx = x_pos;
    ex = icon_w + x_pos;
    tmp_sx = osd_w * y_pos;
    tmp_icon_x = 0;
    for (j = y_pos; j< (y_pos+icon_h); j++)
    {
        for (i = sx; i < ex; i++)
        {
            addr[tmp_sx+i] = data[tmp_icon_x +(i-sx)];
        }
        tmp_sx += osd_w;
        tmp_icon_x += icon_w;
    }
}

void uiOSDIconColor(u16 osd_w , UI_MULT_ICON *iconInfo, u16 x_pos , u16 y_pos , u8 buf_idx, u8 bg_color, u8 alpha)
{
    u32 i;
    u8 *ptr;
    u8  *pucFont;
    u16 icon_h,icon_w,unTotalSize;
    u8 err;

    icon_h=iconInfo->Icon_h;
    icon_w=iconInfo->Icon_w;
    unTotalSize = icon_h* icon_w;
    ptr = iconInfo->IconAddr;
    OSSemPend(uiOSDSemEvt, OS_IPC_WAIT_FOREVER, &err);
    pucFont = &OSDl_ICON[0];
    if(alpha != alpha_OFF)
    {
        for (i=0; i<unTotalSize; i++)
        {
            if (*ptr == 0x3f)
                *pucFont = bg_color;
            else
                *pucFont = *ptr & ((alpha<<6) | 0x3F);
            ptr++;
            pucFont++;
        }
    }
    else
    {
        for (i=0; i<unTotalSize; i++)
        {
            *pucFont = *ptr ;
            ptr++;
            pucFont++;
        }
    }


    _uiOSDIconColor(osd_w , OSDl_ICON , icon_w , icon_h , x_pos , y_pos , buf_idx);
    OSSemPost(uiOSDSemEvt);
}

void uiOSDIconColorByXYChColor(OSD_ICONIDX icon_inx, u16 x_pos , u16 y_pos , u8 buf_idx, u8 bg_color, u8 alpha, u8 font_old_color, u8 font_new_color)
{
    u32 i;
    u8 *ptr;
    u8  *pucFont;
    u16 icon_h,icon_w,unTotalSize;
    u16 osd_w;
    UI_MULT_ICON *iconInfo;
    u8 err;

    osd_w = _uiGetOsdWidth();
    if (uiOsdGetIconInfo(icon_inx,&iconInfo) == 0)
        return;

    icon_h=iconInfo->Icon_h;
    icon_w=iconInfo->Icon_w;
    unTotalSize = icon_h* icon_w;
    ptr = iconInfo->IconAddr;
    OSSemPend(uiOSDSemEvt, OS_IPC_WAIT_FOREVER, &err);
    pucFont = &OSDl_ICON[0];
    for (i=0; i<unTotalSize; i++)
    {
        if (*ptr == 0x3f)
            *pucFont = bg_color;
        else if (*ptr == font_old_color)
            *pucFont = font_new_color;
        else
            *pucFont = *ptr & ((alpha<<6) | 0x3F);
        ptr++;
        pucFont++;
    }

    _uiOSDIconColor(osd_w , OSDl_ICON , icon_w , icon_h , x_pos , y_pos , buf_idx);
    OSSemPost(uiOSDSemEvt);
}

u8 uiOSDIcon(u16 osd_w, u16 icon_inx, u16 x_pos, u16 y_pos, u8 buf_idx, u8 bg_color, u8 alpha)
{
    UI_MULT_ICON *iconInfo;

    if (uiOsdGetIconInfo((OSD_ICONIDX)icon_inx,&iconInfo) == 0)
        return 0;

    uiOSDIconColor(osd_w , iconInfo,  x_pos , y_pos , buf_idx , bg_color, alpha);
    return 1;
}

u8 uiOSDIconColorByXY(OSD_ICONIDX icon_inx ,u16 x_pos, u16 y_pos, u8 buf_idx, u8 bg_color , u8 alpha)
{
#if defined(NEW_UI_ARCHITECTURE)
    return 0;
#else
    u16 osd_w = _uiGetOsdWidth();
    return uiOSDIcon(osd_w, icon_inx, x_pos, y_pos, buf_idx,bg_color, alpha);
#endif
}

u8 uiOSDIconColorByX(OSD_ICONIDX icon_inx ,u16 x_pos,  u8 buf_idx, u8 bg_color , u8 alpha)
{
    u16 icon_h, y_pos, osd_w;
    u32 OSD_SY, OSD_EY;
    UI_MULT_ICON *iconInfo;

    osd_w = _uiGetOsdWidth();

    if(iduOSDGetYStartEnd(buf_idx, &OSD_SY, &OSD_EY) == 0)
    {
        DEBUG_UI("osdGetYStartEnd Fail buf %d\n", buf_idx);
        return 0;
    }

    if (uiOsdGetIconInfo(icon_inx,&iconInfo) == 0)
        return 0;
    icon_h = iconInfo->Icon_h;

    y_pos = (OSD_EY - OSD_SY - icon_h)/2;
    uiOSDIcon(osd_w, icon_inx, x_pos, y_pos, buf_idx, bg_color, alpha);
    return 1;
}

u8 uiOSDIconColorByY(OSD_ICONIDX icon_inx , u16 y_pos, u8 buf_idx, u8 bg_color , u8 alpha)
{
    u16 width, x_pos, osd_w;
    UI_MULT_ICON *iconInfo;

    osd_w = _uiGetOsdWidth();
    if (uiOsdGetIconInfo(icon_inx,&iconInfo) == 0)
        return 0;
    width = iconInfo->Icon_w;

    x_pos = (osd_w - width)/2;

    while((x_pos%4) != 0)
        x_pos--;
    uiOSDIcon(osd_w, icon_inx, x_pos, y_pos, buf_idx, bg_color, alpha);
    return 1;
}

u8 uiOSDMultiLanguageStr(u16 osd_w, u8 str_inx, u16 x_pos, u16 y_pos, u8 buf_idx, u8 str_color, u8 bg_color)
{
    u8 *strLib;
    u8 font_w, font_h;
    UI_MULT_LAN_STR *strInfo;

    if (uiOsdGetStrLib((MSG_SRTIDX)str_inx, &strLib, &font_w, &font_h, &strInfo) == 0)
        return 0;

    _uiOSDDrawStr(osd_w, strInfo, strLib, x_pos, y_pos, font_w, font_h, buf_idx, str_color, bg_color);
    return 1;
}

u8 uiOSDMultiLanStrByLanguage(u16 osd_w, u8 str_inx, u16 x_pos, u16 y_pos, u8 buf_idx, u8 str_color, u8 bg_color, u8 language)
{
    u8 *strLib;
    u8 font_w, font_h;
    UI_MULT_LAN_STR *strInfo;

    if (uiOsdGetStrLibByLanguage((MSG_SRTIDX)str_inx, &strLib, &font_w, &font_h, &strInfo, language) == 0)
        return 0;

    _uiOSDDrawStr(osd_w, strInfo, strLib, x_pos, y_pos, font_w, font_h, buf_idx, str_color, bg_color);
    return 1;

}

u8 uiOSDMultiLanguageStrByXY(MSG_SRTIDX str_inx, u16 x_pos, u16 y_pos, u8 buf_idx, u8 str_color, u8 bg_color)
{
    u16 osd_w;
    osd_w = _uiGetOsdWidth();
    return uiOSDMultiLanguageStr(osd_w, str_inx, x_pos, y_pos, buf_idx, str_color, bg_color);
}

u8 uiOSDMultiLanguageStrByX(MSG_SRTIDX str_inx, u16 x_pos, u8 buf_idx, u8 str_color, u8 bg_color)
{
    u16 y_pos, osd_w;
    u32 OSD_SY, OSD_EY;
    u8 *strLib;
    u8 font_w, font_h;
    UI_MULT_LAN_STR *strInfo;

    osd_w = _uiGetOsdWidth();
    if(uiOsdGetStrLib(str_inx, &strLib, &font_w, &font_h, &strInfo) == 0)
        return 0;

    if(iduOSDGetYStartEnd(buf_idx, &OSD_SY, &OSD_EY) == 0)
    {
        DEBUG_UI("osdGetYStartEnd Fail buf %d\n", buf_idx);
        return 0;
    }

    y_pos = (OSD_EY - OSD_SY - font_h)/2;
    _uiOSDDrawStr(osd_w, strInfo, strLib, x_pos, y_pos, font_w, font_h, buf_idx, str_color, bg_color);
    return 1;
}

u8 uiOSDMultiLanguageStrByY(MSG_SRTIDX str_inx, u16 y_pos, u8 buf_idx, u8 str_color, u8 bg_color)
{
    u16 width, x_pos, osd_w;
    u8 *strLib;
    u8 font_w, font_h;
    UI_MULT_LAN_STR *strInfo;

    osd_w = _uiGetOsdWidth();
    if(uiOsdGetStrLib(str_inx, &strLib, &font_w, &font_h, &strInfo) == 0)
        return 0;

    width = strInfo->Str_len*font_w;
    x_pos = (osd_w - width)/2;
    while((x_pos%4) != 0)
        x_pos--;
    _uiOSDDrawStr(osd_w, strInfo, strLib, x_pos, y_pos, font_w, font_h, buf_idx, str_color, bg_color);
    return 1;
}

u8 uiOSDMultiLanguageStrCenter(MSG_SRTIDX str_inx, u8 buf_idx, u8 str_color, u8 bg_color)
{
#if (UI_PREVIEW_OSD == 0)
    return 0;
#else
    u16 width, x_pos, y_pos, osd_w;
    u32 OSD_SY, OSD_EY;
    u8 *strLib;
    u8 font_w, font_h;
    UI_MULT_LAN_STR *strInfo;


    osd_w = _uiGetOsdWidth();
    if(uiOsdGetStrLib(str_inx, &strLib, &font_w, &font_h, &strInfo) == 0)
        return 0;
    if(iduOSDGetYStartEnd(buf_idx, &OSD_SY, &OSD_EY) == 0)
    {
        DEBUG_UI("osdGetYStartEnd Fail buf %d\n", buf_idx);
        return 0;
    }
    y_pos = (OSD_EY - OSD_SY - font_h)/2;
    width = strInfo->Str_len*font_w;
    x_pos = (osd_w - width)/2;
    while((x_pos%4) != 0)
        x_pos--;

    _uiOSDDrawStr(osd_w, strInfo, strLib, x_pos, y_pos, font_w, font_h, buf_idx, str_color, bg_color);
    return 1;
#endif
}

/*
 *********************************************************************************************************
 * Called by other moudle
 *********************************************************************************************************
 */
u32 uiGetOSDBufAdr(u8 buf_idx)
{
    //printf("OSD %d Addr %x\n",buf_idx, OsdBlkInfo[buf_idx].BlkAddr);
    return((u32)(OsdBlkInfo[buf_idx].BlkAddr));
}

void uiClearOSDBuf(u8 blk_idx)
{
    u32 i, j, BlkW, BlkH;
    u32 DispW;
    u32 * addr;

    addr = (u32 *)uiGetOSDBufAdr(blk_idx);
    BlkW = OsdBlkInfo[blk_idx].BlkEX - OsdBlkInfo[blk_idx].BlkSX;
    BlkH = OsdBlkInfo[blk_idx].BlkEY - OsdBlkInfo[blk_idx].BlkSY;
    //JYK
    DispW = OSDDispWidth[sysTVOutOnFlag];
    if (UI_OSD_4BIT_EN) {
        DispW /= 2;
        BlkW /= 2;
    }

    for (i = 0; i < BlkH; i++)
    {
        for (j = 0; j < BlkW; j+=4)
        {
            *addr = 0;
            addr++;
        }
        addr += (DispW-BlkW)>>2;
    }
}

void uiMenuOSDReset(void)
{
    u32 i, OSD_SIZE;
    u32 * addr;

#if IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE
    return;
#endif

    if(sysTVOutOnFlag)
        iduTVOSDDisable_All();
    else
        iduOSDDisable_All();
    
    addr = (u32*)OSD_buf;
    OSD_SIZE = OSDDispWidth[sysTVOutOnFlag]*OSDDispHeight[sysTVOutOnFlag];
    for(i=0; i< OSD_SIZE; i+=4)
    {
        *addr = 0;
        addr++;
    }
#if( (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1019A)||(CHIP_OPTION == CHIP_A1025A) || (CHIP_OPTION == CHIP_A1021A) )
    addr = (u32*)OSD_buf1;
    for(i=0; i< OSD_buf1_Size; i+=4)
    {
        *addr = 0;
        addr++;
    }
#endif
    memset(&OsdBlkInfo, 0, sizeof(OsdBlkInfo));
    if (sysTVOutOnFlag)
    {
        iduTVOSDClear();
    }
    else
    {
        iduOSDClear();
    }
}

void uiMenuOSDFrame(u16 osd_w , u16 icon_w , u16 icon_h , u16 x_pos , u16 y_pos , u8 buf_idx, u32 data)
{
    u32 *addr;
    u32 i,j;

    //DEBUG_UI(" %d %d %d %d %d %d %x\n",osd_w,icon_w,icon_h,x_pos,y_pos,buf_idx,data);
    addr = (u32 *)uiGetOSDBufAdr(buf_idx);

    if (x_pos > OsdBlkInfo[buf_idx].BlkEX)
    {
        //DEBUG_UI("Frame x %d large than window x %d\n", x_pos, OsdBlkInfo[buf_idx].BlkEX);
        return;
    }
    if (y_pos > OsdBlkInfo[buf_idx].BlkEY)
    {
        //DEBUG_UI("Frame y %d large than window y %d\n", x_pos, OsdBlkInfo[buf_idx].BlkEX);
        return;
    }

    if (icon_w > (OsdBlkInfo[buf_idx].BlkEX - x_pos))
    {
        //DEBUG_UI("Frame x %d Out of Range %d\n", icon_w, (OsdBlkInfo[buf_idx].BlkEX - x_pos));
        icon_w = (OsdBlkInfo[buf_idx].BlkEX - x_pos);
    }
    if (icon_h > (OsdBlkInfo[buf_idx].BlkEY - y_pos))
    {
        //DEBUG_UI("Frame y %d Out of Range %d\n", icon_h, (OsdBlkInfo[buf_idx].BlkEY- y_pos));
        icon_h = (OsdBlkInfo[buf_idx].BlkEY - y_pos);
    }

    // JYK
    if (UI_OSD_4BIT_EN)
    {
        u8 *addr = (u8*)uiGetOSDBufAdr(buf_idx);

        // byte align
        osd_w /= 2;
        icon_w /= 2;
        x_pos /= 2;

        addr += y_pos * osd_w + x_pos;
        for (i=0; i<icon_h; i++) {
            memset(addr, (char)(data&0xFF), icon_w);
            addr += osd_w;
        }
        return;
    }

    //word align
    osd_w >>= 2;
    icon_w >>= 2;
    x_pos >>= 2;

    addr += y_pos * osd_w + x_pos;
    for (i = 0; i < icon_h; i++)
    {
        for (j = 0; j < icon_w; j++)
            *(addr+j) = data;
        addr += osd_w;
    }
}


void uiOSDDrawRectangle(u16 osd_w , u16 icon_w , u16 icon_h , u16 x_pos , u16 y_pos , u8 buf_idx, u32 color, u8 thick)
{
    /*left*/
    uiMenuOSDFrame(osd_w, thick, icon_h+thick, x_pos, y_pos,buf_idx, color);
    /*right*/
    uiMenuOSDFrame(osd_w, thick, icon_h+thick, x_pos+icon_w, y_pos, buf_idx, color);
    /*up*/
    uiMenuOSDFrame(osd_w, icon_w+thick, thick, x_pos, y_pos, buf_idx, color);
    /*down*/
    uiMenuOSDFrame(osd_w, icon_w+thick, thick, x_pos, y_pos+icon_h, buf_idx, color);


}
/*

Routine Description:

    Draw OSD string by colors.

Arguments:

    string - string content to show on OSD
    x_pos - start position of x-axis to print out
    y_pos - start position of y-axis to print out
    buf_idx - osd buffer index to show
    str_color - string color to show
    bg_color - background color

Return Value:

    None.

*/

u8 uiOSDASCIIStringByColor(u8 *string, u16 x_pos , u16 y_pos, u8 buf_idx , u8 str_color, u8 bg_color)   // XY
{
#if (UI_PREVIEW_OSD == 0)
    return 0;
#else
    u8  *strLib, *ptr, *pucFont;
    u8  font_w, font_h;
    u32 unTotalSize, len, index, i;
    u16 osd_w;
    UI_MULT_LAN_STR *strInfo;
    u8  err;
    
    if (uiOsdGetStrLibByLanguage(MSG_ASCII_STR, &strLib, &font_w, &font_h, &strInfo, UI_MULT_LANU_EN) == 0)
        return 0;

    len = strlen((char *)string);
    unTotalSize = font_w*font_h;
    osd_w = _uiGetOsdWidth();

    OSSemPend(uiOSDSemEvt, OS_IPC_WAIT_FOREVER, &err);
    for (index=0; index<len; index++)
    {
        ptr = strLib+(*string - 32)*unTotalSize;
        pucFont = &OSD_ICON[0];
        for (i=0; i<unTotalSize; i++)
        {
            if (*ptr == 0x3f)
                *pucFont = bg_color;
            else if (*ptr != 0xC1)
                *pucFont = str_color;
            else
                *pucFont = *ptr;
            ptr++;
            pucFont++;
        }
        uiMenuOSDPrintStr(osd_w , OSD_ICON , font_w , font_h , x_pos , y_pos , buf_idx);
        x_pos+=font_w;
        string++;
    }
    OSSemPost(uiOSDSemEvt);
    return 1;
#endif
}


u8 uiOSDASCIIStringByColorX(u8 *string, u16 x_pos , u8 buf_idx , u8 str_color, u8 bg_color)
{
    u8  *strLib, *ptr, *pucFont;
    u8  font_w, font_h;
    u32 unTotalSize, len, index, i;
    u16 osd_w, y_pos;
    u32 OSD_SY, OSD_EY;
    UI_MULT_LAN_STR *strInfo;
    u8  err;


    if (uiOsdGetStrLibByLanguage(MSG_ASCII_STR, &strLib, &font_w, &font_h, &strInfo, UI_MULT_LANU_EN) == 0)
        return 0;

    len = strlen((char *)string);
    unTotalSize = font_w*font_h;
    osd_w = _uiGetOsdWidth();

    if(iduOSDGetYStartEnd(buf_idx, &OSD_SY, &OSD_EY) == 0)
    {
        DEBUG_UI("osdGetYStartEnd Fail buf %d\n", buf_idx);
        return 0;
    }

    y_pos = (OSD_EY - OSD_SY - font_h)/2;

    OSSemPend(uiOSDSemEvt, OS_IPC_WAIT_FOREVER, &err);
    for (index=0; index<len; index++)
    {
        ptr = strLib+(*string - 32)*unTotalSize;
        pucFont = &OSD_ICON[0];
        for (i=0; i<unTotalSize; i++)
        {
            if (*ptr == 0x3f)
                *pucFont = bg_color;
            else if (*ptr != 0xC1)
                *pucFont = str_color;
            else
                *pucFont = *ptr;
            ptr++;
            pucFont++;
        }
        uiMenuOSDPrintStr(osd_w , OSD_ICON , font_w , font_h , x_pos , y_pos , buf_idx);
        x_pos+=font_w;
        string++;
    }
    OSSemPost(uiOSDSemEvt);
    return 1;
}

u8 uiOSDASCIIStringByColorY(u8 *string, u16 y_pos, u8 buf_idx , u8 str_color, u8 bg_color)
{
    u8  *strLib, *ptr, *pucFont;
    u8  font_w, font_h;
    u32 unTotalSize, len, index, i;
    u16 osd_w, x_pos, width;
    UI_MULT_LAN_STR *strInfo;
    u8  err;


    if (uiOsdGetStrLibByLanguage(MSG_ASCII_STR, &strLib, &font_w, &font_h, &strInfo, UI_MULT_LANU_EN) == 0)
        return 0;

    len = strlen((char *)string);
    unTotalSize = font_w*font_h;
    osd_w = _uiGetOsdWidth();

    width = len*font_w;
    x_pos = (osd_w - width)/2;
    while((x_pos%4) != 0)
        x_pos--;

    OSSemPend(uiOSDSemEvt, OS_IPC_WAIT_FOREVER, &err);
    for (index=0; index<len; index++)
    {
        ptr = strLib+(*string - 32)*unTotalSize;
        pucFont = &OSD_ICON[0];
        for (i=0; i<unTotalSize; i++)
        {
            if (*ptr == 0x3f)
                *pucFont = bg_color;
            else if (*ptr != 0xC1)
                *pucFont = str_color;
            else
                *pucFont = *ptr;
            ptr++;
            pucFont++;
        }
        uiMenuOSDPrintStr(osd_w , OSD_ICON , font_w , font_h , x_pos , y_pos , buf_idx);
        x_pos+=font_w;
        string++;
    }
    OSSemPost(uiOSDSemEvt);
    return 1;
}

u8 uiOSDASCIIStringByColorCenter(u8 *string, u8 buf_idx , u8 str_color, u8 bg_color)
{
    u8  *strLib, *ptr, *pucFont;
    u8  font_w, font_h;
    u32 unTotalSize, len, index, i;
    u16 osd_w, x_pos, y_pos, width;
    u32 OSD_SY, OSD_EY;
    UI_MULT_LAN_STR *strInfo;
    u8  err;


    if (uiOsdGetStrLibByLanguage(MSG_ASCII_STR, &strLib, &font_w, &font_h, &strInfo, UI_MULT_LANU_EN) == 0)
        return 0;

    len = strlen((char *)string);
    unTotalSize = font_w*font_h;
    osd_w = _uiGetOsdWidth();

    if(iduOSDGetYStartEnd(buf_idx, &OSD_SY, &OSD_EY) == 0)
    {
        DEBUG_UI("osdGetYStartEnd Fail buf %d\n", buf_idx);
        return 0;
    }

    y_pos = (OSD_EY - OSD_SY - font_h)/2;
    width = len*font_w;
    x_pos = (osd_w - width)/2;
    while((x_pos%4) != 0)
        x_pos--;

    OSSemPend(uiOSDSemEvt, OS_IPC_WAIT_FOREVER, &err);
    for (index=0; index<len; index++)
    {
        ptr = strLib+(*string - 32)*unTotalSize;
        pucFont = &OSD_ICON[0];
        for (i=0; i<unTotalSize; i++)
        {
            if (*ptr == 0x3f)
                *pucFont = bg_color;
            else if (*ptr != 0xC1)
                *pucFont = str_color;
            else
                *pucFont = *ptr;
            ptr++;
            pucFont++;
        }
        uiMenuOSDPrintStr(osd_w , OSD_ICON , font_w , font_h , x_pos , y_pos , buf_idx);
        x_pos+=font_w;
        string++;
    }
    OSSemPost(uiOSDSemEvt);
    return 1;
}

void uiOsdDisable(u8 osd)
{
    if (sysTVOutOnFlag)
        iduTVOSDDisable(osd);
    else
        iduOSDDisable(osd);
}

void uiOsdEnable(u8 osd)
{
    if (sysTVOutOnFlag)
        iduTVOSDEnable(osd);
    else
        iduOSDEnable(osd);
}

void uiOsdDisableAll(void)
{
    u8  i;
    if (sysTVOutOnFlag)
    {
        for(i = 0; i < IDU_OSD_MAX_NUM; i++)
            iduTVOSDDisable(i);
    }
    else
        iduOSDDisable_All();
}

void uiOsdEnableAll(void)
{
    u8  i;

    if (sysTVOutOnFlag)
    {
        for(i = 0; i < IDU_OSD_MAX_NUM; i++)
            iduTVOSDEnable(i);
    }
    else
        iduOSDEnable_All();
}


