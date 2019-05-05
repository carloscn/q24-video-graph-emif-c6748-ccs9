/*========================================================*
 * 文件模块说明:
 * lcdFun.c 通用界面模块的功能函数
 * 文件版本: V1.0.0
 * 开发人员: Rock
 * 创建时间: 2013-01-01 
 * Copyright(c) 2013-2015  Rock-Embed Limited Co.
 *========================================================*
 * 程序修改记录：
 * <版本号> <修改日期>, <修改人员>: <修改功能概述>
 *      V1.0.0           2013-01-01              Rock                        创建
 *========================================================*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "LcdMacro.h"

//==============================================================================
// 检查坐标点是否超出范围                       **** 本函数仅供内部调用 ****
inline int Lcd_Pub_CheckPosi( uint16 *pwPosi, uint16 wSize, int iErrCode )
{
    int iRetCode = 0;

    if( *pwPosi >=wSize )             // 坐标越限
    {
        iRetCode = iErrCode;
        *pwPosi = wSize - 1;
    }

    return iRetCode;
}

//==============================================================================
//功能:24位色转为16位色,内部函数                      **** 本函数仅供内部调用 ****
//参数:uint32 dwRGB,24位颜色值
//返回:16位颜色值
inline uint16 Lcd_Pub_RGBto16B( uint32 dwRGB )
{
    uint16 R,G,B,L;

    R=(dwRGB>>16)&0xff;
    G=(dwRGB>>8 )&0xff;
    B=(dwRGB    )&0xff;
    L=(dwRGB>>24);

    if( R>=0xf8 )
        R=0xf8;
    else if( R&0x04 )
        R=(R&0xf8)+0x08;
    else
        R=R&0xf8;

    if( G>=0xf8 )
        G=0xf8;
    else if( G&0x04 )
        G=(G&0xf8)+0x08;
    else
        G=G&0xf8;

    if( B>=0xf8 )
        B=0xf8;
    else if( B&0x04 )
        B=(B&0xf8)+0x08;
    else
        B=B&0xf8;

    if( L >0x7f )
        L=1;
    else
        L=0;
    return( (R<<8)+(G<<3)+(B>>2) + L);
}


//==============================================================================
//功能:把RGB颜色值转换为所用LCD支持的颜色值
//返回：转换后的颜色值
//参数：dwRGB，24位彩色值
uint16 Lcd_Pub_RGBtoColor( uint32 dwRGB )
{
    return( Lcd_Pub_RGBto16B(dwRGB) );

}

//==============================================================================
//功能:把16位或8位颜色值转换为24位颜色值---内部函数
//返回：转换后的颜色值
//参数：wColor，24位彩色值
//==============================================================================
uint32 Lcd_Pub_ColortoRGB( uint16 wColor )
{
    uint32 L;
    if(wColor &1)
        L = 0xFF000000;
    else
        L = 0;
    return(L+((wColor&0xF800)<<8)+((wColor&0x07C0)<<5)+((wColor&0x001F)<<3));


}


//==============================================================================
//功能:由虚拟界面取得相应物理界面,即界面变量的下标  **** 本函数仅供内部调用 ****
//参数:uint8 byScreen,虚拟界面编号
//返回:界面物理序号
//==============================================================================
uint8 Lcd_Pub_GetScreenNo( uint8 byScreen )
{
    switch( byScreen )
    {
    case CN_SCR_ALARM:  return(0);      break;
    case CN_SCR_RUN:    return(0);      break;
    case CN_SCR_MENU:   return(1);      break;
    case CN_SCR_USER:   return(1);      break;
    default:            return(0);      break;
    }
    return( 0 );
}

//==============================================================================
//功能:设置全局指针g_tCVsr指向byScreen界面的设置    **** 本函数仅供内部调用 ****
//参数：byScreen，无符号字符，虚拟界面编号。
//返回:无
//==============================================================================
void Lcd_Pub_GetCurrentSet( uint8 byScreen )
{
    uint16 wSno;

    wSno=Lcd_Pub_GetScreenNo( byScreen );
    g_tCVsr=&g_tVsr[wSno];
}


//==============================================================================
//功能:根据虚拟界面号取得相应的显存地址.            **** 本函数仅供内部调用 ****
//参数:uint8 byScreen,虚拟界面编号
//返回:显存地址
//==============================================================================
void *Lcd_Pub_GetScreenAddr( uint8 byScreen )
{
    return( g_tVsr[Lcd_Pub_GetScreenNo( byScreen)].pwbyLCDMemory );
}

//==============================================================================
//功能：使用系统缺省字体(8*16点阵)和当前系统设置显示一个英文字符。  ****内部使用
//返回：正确显示时返回0；x坐标超出范围返回-2，y坐标超出范围返回-1。
//参数：byScreen，无符号字符，虚拟界面编号。
//x、y，无符号短整数，显示字符的坐标位置。
//wAttrib，显示附加属性((含义见显示附件属性说明))。
//byAscii 无符号字符，所要显示字符的ASCII编码。
//横向显示时，X:左-->右,Y:上-->下,坐标为左上角坐标
//竖向显示时, X:左-->右,Y:下-->上,坐标为左下角坐标.
//==============================================================================
inline uint16 Lcd_Pub_Ascii( uint8 byScreen, uint16 x, uint16 y,  uint8 byAscii, bool_t bTR )
{
    uint8 i,j;
    uint8    byLine, *pbyFontLib;

    Lcd_Pub_GetCurrentSet( byScreen );
    pbyFontLib = &g_tCVsr->pbyLibAscii[ byAscii*g_tCVsr->wASCIIFontHeight];     // 字模首地址

    if( 0 ==g_tDspAttrib.style )
    {// 正常显示
        if( 0 ==g_tDspAttrib.Rotate )
        {// 横向显示
			if( (x+g_tCVsr->wASCIIFontWidth ) >CN_LCD_SIZE_X ) return(-2);  // 列越界
            if( (y+g_tCVsr->wASCIIFontHeight) >CN_LCD_SIZE_Y ) return(-1);  // 行越界

            for( j=0; j<g_tCVsr->wASCIIFontHeight; j++ )
            {
                byLine = pbyFontLib[j];                         // 取字模的当前行
                for( i=0; i<g_tCVsr->wASCIIFontWidth; i++ )
                {
                    if( (byLine<<i) & 0x80 ) Lcd_Pub_PixelFT( x+i, y+j );
                    else if( false ==bTR   ) Lcd_Pub_PixelBK( x+i, y+j );   // 非透明则显示背景
                }
            } // end for( j )
        }
		else
        {// 竖向显示，即字符逆时钟旋转90度
			if( (x+g_tCVsr->wASCIIFontHeight) >CN_LCD_SIZE_X )  return(-2); // 列越界
            if( (y+1) < g_tCVsr->wASCIIFontWidth )              return(-1); // 行越界

            for (j=0; j<g_tCVsr->wASCIIFontHeight; j++ )
            {
                byLine = pbyFontLib[j];                         // 取字模的当前行
                for( i=0;i <g_tCVsr->wASCIIFontWidth; i++ )
                {
                    if( (byLine<<i) & 0x80 ) Lcd_Pub_PixelFT( x+j, y-i );
                    else if( false ==bTR   ) Lcd_Pub_PixelBK( x+j, y-i );   // 非透明则显示背景
                }
            } // end for( j )
        }// end if( Rotate ) ... else ...
    }
	else
    { // 突出显示，即字符与背景颜色对调。
        if( 0 ==g_tDspAttrib.Rotate )
        {// 横向显示
            if( (x+g_tCVsr->wASCIIFontWidth ) >CN_LCD_SIZE_X ) return(-2);  // 列越界
            if( (y+g_tCVsr->wASCIIFontHeight) >CN_LCD_SIZE_Y ) return(-1);  // 行越界

            for( j=0; j<g_tCVsr->wASCIIFontHeight; j++ )
            {
                byLine = pbyFontLib[j];                         // 取字模的当前行
                for( i=0; i<g_tCVsr->wASCIIFontWidth; i++ )
                {
                    if( (byLine<<i) & 0x80 ) Lcd_Pub_PixelBK( x+i, y+j );
                    else if( false ==bTR   ) Lcd_Pub_PixelFT( x+i, y+j );   // 非透明则显示背景
                }
            } // end for( j )
        }else
        {// 竖向显示，即字符逆时钟旋转90度
            if( (x+g_tCVsr->wASCIIFontHeight) >CN_LCD_SIZE_X )  return(-2); // 列越界
            if( (y+1) < g_tCVsr->wASCIIFontWidth )              return(-1); // 行越界

            for (j=0; j<g_tCVsr->wASCIIFontHeight; j++ )
            {
                byLine = pbyFontLib[j];                         // 取字模的当前行
                for( i=0;i <g_tCVsr->wASCIIFontWidth; i++ )
                {
                    if( (byLine<<i) & 0x80 ) Lcd_Pub_PixelBK( x+j, y-i );
                    else if( false ==bTR   ) Lcd_Pub_PixelFT( x+j, y-i );   // 非透明则显示背景
                }
            } // end for( j )
        }// end if( Rotate ) ... else ...
    }// end if( style ) ... else ...

    return( 0 );
}

//==============================================================================
//功能：使用系统缺省字体和当前系统设置显示一个汉字。
//注意：本函数仅提供驱动程序内部调用，为提高效率，不判断坐标是否越界。
//参数：byScreen: 无符号字符，虚拟界面编号。
//      x、y:     无符号短整数，显示字符的坐标位置。
//                横向显示时，X:左-->右,Y:上-->下,坐标为左上角坐标
//                竖向显示时, X:左-->右,Y:下-->上,坐标为左下角坐标.
//      wAttrib:  显示附加属性(含义见显示附件属性说明)。
//      byAscii:  所要显示字符的ASCII编码。
//返回：无
//==============================================================================
void Lcd_Pub_GB2312( uint8 byScreen, uint16 x, uint16 y,  uint16 wCharGB , bool_t bTR )
{
    uint8    i,j;
    uint16    offset, wLine, *pwFontLib;

    Lcd_Pub_GetCurrentSet( byScreen );
    offset    = ( (wCharGB>>8)-0xA1 ) * 94 + (wCharGB&0xFF) -0xA1;
    pwFontLib = &g_tCVsr->pwLibHZ[ offset * g_tCVsr->wGBFontHeight];    // 字模首地址

    if( 0 ==g_tDspAttrib.style )
    {// 正常显示
        if( 0 ==g_tDspAttrib.Rotate )
        {// 横向显示
            for( j=0; j<g_tCVsr->wGBFontHeight; j++ )
            {
                wLine = pwFontLib[j];                       // 取字模的当前行
                wLine = (wLine<<8) + (wLine>>8);

                for( i=0;i <g_tCVsr->wGBFontWidth; i++ )
                {
                    if( (wLine<<i) & 0x8000 ) Lcd_Pub_PixelFT( x+i, y+j );
                    else if( false ==bTR    ) Lcd_Pub_PixelBK( x+i, y+j );  // 非透明则显示背景

                }
            } // end for( j )
        }else
        {// 竖向显示，即字符逆时钟旋转90度
            for( j=0; j<g_tCVsr->wGBFontHeight; j++ )
            {
//                wLine = pwFontLib[j];                       // 取字模的当前行
                wLine = pwFontLib[j];                       // 取字模的当前行
                wLine = (wLine<<8) + (wLine>>8);
                for( i=0;i <g_tCVsr->wGBFontWidth; i++ )
                {
                    if( (wLine<<i) & 0x8000 ) Lcd_Pub_PixelFT( x+j, y-i );
                    else if( false ==bTR    ) Lcd_Pub_PixelBK( x+j, y-i );  // 非透明则显示背景
                }
            } // end for( j )
        }// end if( Rotate ) ... else ...
    }else
    { // 突出显示，即字符与背景颜色对调。
        if( 0 ==g_tDspAttrib.Rotate )
        {// 横向显示
            for( j=0; j<g_tCVsr->wGBFontHeight; j++ )
            {
//                wLine = pwFontLib[j];                       // 取字模的当前行
                wLine = pwFontLib[j];                       // 取字模的当前行
                wLine = (wLine<<8) + (wLine>>8);
                for( i=0;i <g_tCVsr->wGBFontWidth; i++ )
                {
                    if( (wLine<<i) & 0x8000 ) Lcd_Pub_PixelBK( x+i, y+j );
                    else if( false ==bTR    ) Lcd_Pub_PixelFT( x+i, y+j );  // 非透明则显示背景
                }
            } // end for( j )
        }else
        {// 竖向显示，即字符逆时钟旋转90度
            for( j=0; j<g_tCVsr->wGBFontHeight; j++ )
            {
//                wLine = pwFontLib[j];                       // 取字模的当前行
                wLine = pwFontLib[j];                       // 取字模的当前行
                wLine = (wLine<<8) + (wLine>>8);
                for( i=0;i <g_tCVsr->wGBFontWidth; i++ )
                {
                    if( (wLine<<i) & 0x8000 ) Lcd_Pub_PixelBK( x+j, y-i );
                    else if( false ==bTR    ) Lcd_Pub_PixelFT( x+j, y-i );  // 非透明则显示背景
                }
             } // end for( j )
        }// end if( Rotate ) ... else ...
    }// end if( style ) ... else ...

    return;
}



//==============================================================================
//返回：无。
//参数：byScreen，虚拟界面编号。
//dwRGB，无符号长整数，背景色的RGB值。
//设置系统当前显示所用的背景色。
//==============================================================================
void Lcd_SetBackColor ( uint8 byScreen, uint32 dwRGB )
{
    g_tVsr[Lcd_Pub_GetScreenNo( byScreen)].wBackColor=Lcd_Pub_RGBtoColor( dwRGB);
}


//==============================================================================
//提取指定界面的当前字符色。
//参数：byScreen，虚拟界面编号。
//返回：24位彩色值。
uint32 Lcd_GetFontColor ( uint8 byScreen)
{
    return( Lcd_Pub_ColortoRGB(g_tVsr[Lcd_Pub_GetScreenNo( byScreen)].wFontColor));
}


//==============================================================================
//功能：设置系统当前显示所用的字符颜色。
//参数：byScreen，虚拟界面编号。
//      dwRGB，无符号长整数，字符颜色的RGB值。
//返回：无。
void Lcd_SetFontColor ( uint8 byScreen, uint32 dwRGB )
{
    g_tVsr[Lcd_Pub_GetScreenNo( byScreen)].wFontColor=Lcd_Pub_RGBtoColor( dwRGB);
}


//==============================================================================
//功能：清除原有数据并用缺省背景色填充整个屏幕。
//参数：byScreen，无符号字符，虚拟界面编号。
//返回：无。
void Lcd_Clear( uint8 byScreen )
{
    uint32 i;
    uint32 *buf = Lcd_Pub_GetScreenAddr(byScreen);
    uint32 color = g_tVsr[Lcd_Pub_GetScreenNo( byScreen)].wBackColor;
    color = color + (color<<16);
    for(i=0; i < CN_DISPLAYMEMORYSIZE/4; i++)
        buf[i] = color;
}

//==============================================================================
//返回：正确显示时返回0；x坐标超出范围返回-2，y坐标超出范围返回-1。
//参数：byScreen，无符号字符，虚拟界面编号。
//xStart、yStart，无符号短整数，直线的起始坐标。
//xEnd，无符号短整数，直线的末端坐标。
//wWidth，无符号短整数，直线的宽度。
//功能：使用系统当前设置画一水平线。
//==============================================================================
int Lcd_LineH( uint8 byScreen, uint16 wXStart, uint16 wXEnd, uint16 wYStart, uint16 wWidth )
{
    uint16    wLoopX, wLoopY, wYEnd;
    int     iRetCode;

    wYEnd = wYStart + wWidth;           // 宽度
    iRetCode = Lcd_Pub_CheckPosi( &wXStart, CN_LCD_SIZE_X, -2 );
    iRetCode = Lcd_Pub_CheckPosi( &wXEnd,   CN_LCD_SIZE_X, -2 );
    iRetCode = Lcd_Pub_CheckPosi( &wYStart, CN_LCD_SIZE_Y, -1 );
    iRetCode = Lcd_Pub_CheckPosi( &wYEnd,   CN_LCD_SIZE_Y, -1 );
    if( wXEnd < wXStart )   M_GuiSwap ( wXEnd, wXStart );

    Lcd_Pub_GetCurrentSet( byScreen );
    for( wLoopX=wXStart; wLoopX<wXEnd; wLoopX++ )
    {
        for( wLoopY=wYStart; wLoopY<wYEnd; wLoopY++ )
        {
            Lcd_Pub_PixelFT( wLoopX, wLoopY );
        }
    }

    return( iRetCode );
}

//==============================================================================
//返回：正确显示时返回0；x坐标超出范围返回-2，y坐标超出范围返回-1。
//参数：byScreen，无符号字符，虚拟界面编号。
//      xStart、yStart，无符号短整数，直线的起始坐标。
//      xEnd，无符号短整数，直线的末端坐标。
//      wWidth，无符号短整数，直线的宽度。
//功能：使用系统当前设置画一垂直线。
//==============================================================================
int Lcd_LineV( uint8 byScreen, uint16 wYStart, uint16 wYEnd, uint16 wXStart, uint16 wWidth )
{
    uint16    wLoopX, wLoopY, wXEnd;
    int     iRetCode;

    wXEnd = wXStart + wWidth;           // 宽度
    iRetCode = Lcd_Pub_CheckPosi( &wXStart, CN_LCD_SIZE_X, -2 );
    iRetCode = Lcd_Pub_CheckPosi( &wXEnd,   CN_LCD_SIZE_X, -2 );
    iRetCode = Lcd_Pub_CheckPosi( &wYStart, CN_LCD_SIZE_Y, -1 );
    iRetCode = Lcd_Pub_CheckPosi( &wYEnd,   CN_LCD_SIZE_Y, -1 );
    if( wYEnd < wYStart )   M_GuiSwap ( wYEnd, wYStart );

    Lcd_Pub_GetCurrentSet(byScreen);
    for( wLoopX=wXStart; wLoopX<wXEnd; wLoopX++ )
    {
        for( wLoopY=wYStart; wLoopY<wYEnd; wLoopY++ )
        {
            Lcd_Pub_PixelFT( wLoopX, wLoopY );
        }
    }

    return( iRetCode );
}

//==============================================================================
//返回：正确显示时返回0；x坐标超出范围返回-2，y坐标超出范围返回-1。
//参数：byScreen，无符号字符，虚拟界面编号。
//xStart、yStart，无符号短整数，直线的起始坐标。
//xEnd、yEnd，无符号短整数，直线的末端坐标。
//wWidth，无符号短整数，直线的宽度。
//功能：使用系统当前设置画一直线。
//==============================================================================
int Lcd_Line(  uint8 byScreen, uint16 wXstart, uint16 wYstart, uint16 wXend, uint16 wYend, uint16 wWidth )
{
    uint16 wDX;
    uint16 wDY;
    uint16 wSignY;
    uint16 wSignX;
    uint16 wDecision;
    uint16 wCurx, wCury, wNextx, wNexty, wPY, wPX;
    uint8 byErr=0;

    byErr = Lcd_Pub_CheckPosi( &wXstart, CN_LCD_SIZE_X, -2 );
    byErr = Lcd_Pub_CheckPosi( &wXend,   CN_LCD_SIZE_X, -2 );
    byErr = Lcd_Pub_CheckPosi( &wYstart, CN_LCD_SIZE_Y, -1 );
    byErr = Lcd_Pub_CheckPosi( &wYend,   CN_LCD_SIZE_Y, -1 );

    if( wYstart == wYend )
    {
        Lcd_LineH( byScreen,wXstart, wXend, wYstart, wWidth );
        return( byErr );
    }
    if( wXstart == wXend )
    {
        Lcd_LineV( byScreen,wYstart, wYend, wXstart, wWidth );
        return( byErr );
    }

    Lcd_Pub_GetCurrentSet(byScreen);
    wDX = abs(wXstart - wXend);
    wDY = abs(wYstart - wYend);
    if (((wDX >= wDY && (wXstart > wXend)) || \
        ((wDY > wDX) && (wYstart > wYend))))
    {
        M_GuiSwap(wXend, wXstart);
        M_GuiSwap(wYend, wYstart);
    }
    wSignY = (wYend - wYstart) / wDY;
    wSignX = (wXend - wXstart) / wDX;

    if (wDX >= wDY)
    {
        wCurx = wXstart;
        wCury = wYstart;
        wNextx = wXend;
        wNexty = wYend;
        wDecision = (wDX >> 1);
        for (;wCurx <= wNextx;)
        {
            if (wDecision >= wDX)
            {
                wDecision -= wDX;
                wCury += wSignY;
                wNexty -= wSignY;
            }
            for(wPY = wCury - wWidth / 2; \
                wPY <= wCury + wWidth / 2; wPY++)
            {
                Lcd_Pub_PixelFT(wCurx, wPY);
            }

            for(wPY = wNexty - wWidth / 2;
                wPY <= wNexty + wWidth / 2; wPY++)
            {
                Lcd_Pub_PixelFT(wNextx, wPY);
            }
            wCurx++;
            wNextx--;
            wDecision += wDY;
        }
    }
    else
    {
        wCurx = wXstart;
        wCury = wYstart;
        wNextx = wXend;
        wNexty = wYend;
        wDecision = (wDY >> 1);
        for (;wCury <= wNexty;)
        {

            if (wDecision >= wDY)
            {
                wDecision -= wDY;
                wCurx += wSignX;
                wNextx -= wSignX;
            }
            for (wPX = wCurx - wWidth / 2;
                 wPX <= wCurx + wWidth / 2; wPX++)
            {
                Lcd_Pub_PixelFT(wPX, wCury);
            }

            for (wPX = wNextx - wWidth / 2;
                 wPX <= wNextx + wWidth / 2; wPX++)
            {
                Lcd_Pub_PixelFT(wPX, wNexty);
            }
            wCury++;
            wNexty--;
            wDecision += wDX;
        }
    }
    return(byErr);
}

//==============================================================================
//功能:设置系统使用的默认字体,所有字符和字符串显示函数均按此设置进行显示
//返回:正确设置,返回1,参数错误,返回-1;
//参数:byScreen 无符号字符，虚拟界面编号
//     wGB,汉字字体,可选参数为CN_GB12Bit,CN_GB14Bit,CN_GB16Bit
//     wASCII,字符字体,可选参数为 CN_ASCII88,CN_ASCII126,CN_ASCII148,CN_ASCII168
//==============================================================================
uint16 Lcd_SetFont( uint8 byScreen,uint16 wFontGB,uint16 wFontASCII )
{
    uint16 wSno;
    int     iRet = 1;

    wSno=Lcd_Pub_GetScreenNo( byScreen );
    switch( wFontGB )
    {
    /*
    case CN_GB12BIT:
        g_tVsr[wSno].wGBFontWidth=14;
        g_tVsr[wSno].wGBFontHeight=12;
        g_tVsr[wSno].pwLibHZ=g_awLibHZ12_12[0];
        break;

    case CN_GB14BIT:
        g_tVsr[wSno].wGBFontWidth=14;
        g_tVsr[wSno].wGBFontHeight=14;
        g_tVsr[wSno].pwLibHZ=g_awLibHZ14_14[0];
        break;
*/
    case CN_GB16BIT:
        g_tVsr[wSno].wGBFontWidth=16;
        g_tVsr[wSno].wGBFontHeight=16;
        g_tVsr[wSno].pwLibHZ=g_awLibHZ16_16[0];
        break;

    default:    iRet = -1;      break;
    }

    switch( wFontASCII )
    {
    /*
    case CN_ASCII88:
        g_tVsr[wSno].wASCIIFontWidth=8;
        g_tVsr[wSno].wASCIIFontHeight=8;
        g_tVsr[wSno].pbyLibAscii=g_abyASCII88[0];
        break;

    case CN_ASCII126:
        g_tVsr[wSno].wASCIIFontWidth=7;
        g_tVsr[wSno].wASCIIFontHeight=12;
        g_tVsr[wSno].pbyLibAscii=g_abyASCII126[0];
        break;

    case CN_ASCII148:
        g_tVsr[wSno].wASCIIFontWidth=8;
        g_tVsr[wSno].wASCIIFontHeight=16;
        g_tVsr[wSno].pbyLibAscii=g_abyASCII168[0];
        break;
*/
    case CN_ASCII168:
        g_tVsr[wSno].wASCIIFontWidth=8;
        g_tVsr[wSno].wASCIIFontHeight=16;
        g_tVsr[wSno].pbyLibAscii=g_byASCII168[0];
        break;

    default:    iRet = -1;      break;
    }

    return( iRet );
}


//==============================================================================
//返回：正确显示时返回0；x坐标超出范围返回-2，y坐标超出范围返回-1。
//参数：byScreen，无符号字符，虚拟界面编号。
//x、y，无符号短整数，显示字符的坐标位置。
//wAttrib，显示附加属性((含义见显示附件属性说明))。
//byAscii 无符号字符，所要显示字符的ASCII编码。
//功能：使用系统缺省字体(8*16点阵)和当前系统设置显示一个英文字符。
//横向显示时，X:左-->右,Y:上-->下,坐标为左上角坐标
//竖向显示时, X:左-->右,Y:下-->上,坐标为左下角坐标.
//==============================================================================
uint16 Lcd_Ascii( uint8 byScreen, uint16 x, uint16 y,  uint8 byAscii )
{
	return Lcd_Pub_Ascii( byScreen, x, y, byAscii, false );
}

//==============================================================================
//返回：正确显示时返回0；x坐标超出范围返回-2，y坐标超出范围返回-1。
//参数：byScreen，无符号字符，虚拟界面编号。
//x、y，无符号短整数，显示字符的坐标位置。
//wAttrib，显示附加属性((含义见显示附件属性说明))。
//byAscii 无符号字符，所要显示字符的ASCII编码。
//功能：使用系统缺省字体(8*16点阵)和当前系统设置显示一个英文字符。透明显示
//横向显示时，X:左-->右,Y:上-->下,坐标为左上角坐标
//竖向显示时, X:左-->右,Y:下-->上,坐标为左下角坐标.
//==============================================================================
uint16 Lcd_AsciiTr( uint8 byScreen, uint16 x, uint16 y, uint8 byAscii )
{
    return Lcd_Pub_Ascii( byScreen, x, y, byAscii, true );
}

//==============================================================================
//返回：无
//参数：byScreen，无符号字符，虚拟界面编号。
//x、y，无符号短整数，显示字符的坐标位置。
//wAttrib，显示附加属性(含义见显示附件属性说明)。
//byAscii，所要显示字符的ASCII编码。
//功能：使用系统缺省字体和当前系统设置显示一个汉字。
//注意：本函数仅提供驱动程序内部调用，为提高效率，不判断坐标是否越界。
//横向显示时，X:左-->右,Y:上-->下,坐标为左上角坐标
//竖向显示时, X:左-->右,Y:下-->上,坐标为左下角坐标.
//==============================================================================
void Lcd_GB2312( uint8 byScreen, uint16 x, uint16 y,  uint16 wCharGB )
{
    Lcd_Pub_GB2312( byScreen, x, y, wCharGB, false );
    return;
}


//==============================================================================
//返回：无
//参数：byScreen，无符号字符，虚拟界面编号。
//x、y，无符号短整数，显示字符的坐标位置。
//wAttrib，显示附加属性(含义见显示附件属性说明)。
//byAscii，所要显示字符的ASCII编码。
//功能：使用系统缺省字体和当前系统设置显示一个汉字。透明显示
//注意：本函数仅提供驱动程序内部调用，为提高效率，不判断坐标是否越界。
//横向显示时，X:左-->右,Y:上-->下,坐标为左上角坐标
//竖向显示时, X:左-->右,Y:下-->上,坐标为左下角坐标.
//==============================================================================
void Lcd_GB2312Tr( uint8 byScreen, uint16 x, uint16 y,  uint16 GB )
{
    Lcd_Pub_GB2312( byScreen, x, y, GB, true );

}

//==============================================================================
//返回：正确显示时返回0；x坐标超出范围返回-2，y坐标超出范围返回-1。
//参数：byScreen，无符号字符，虚拟界面编号。
//x、y，无符号短整数，显示字符的坐标位置。
//wAttrib，显示附加属性(含义见显示附件属性说明)。
//pcString，字符串指针，以NULL结束。
//功能：使用系统缺省字体和当前系统设置显示字符串；函数根据字符编码自动识别是英文还是中文字符。
//如果字符串中间包含换行符号(ASCII数值为0x0A)，则进行换行显示。下一行的横坐标为x，
//而纵坐标则为当前行的纵坐标、
//缺省字体点阵高度(如为8*16点阵则为16)以及行距三者的和。
//横向显示时，X:左-->右,Y:上-->下,坐标为左上角坐标
//竖向显示时, X:左-->右,Y:下-->上,坐标为左下角坐标.
//==============================================================================
uint16 Lcd_Str( uint8 byScreen, uint16 x, uint16 y, uint8 *pcString )
{
    uint16 charindex,i,j,err,bakx,baky,LineOver;

    bakx=x;
    baky=y;
    Lcd_Pub_GetCurrentSet(byScreen);

    charindex = 0;
    if((g_tDspAttrib.StringDirect==0)&&(g_tDspAttrib.Rotate==0))
    {// 字符不旋转，字符串横向,坐标为字符串左上角坐标
        if( (y+g_tCVsr->wGBFontWidth) >CN_LCD_SIZE_Y ) return(-1);

        while( pcString[charindex]!=0x0 )
        {
            // 若坐标越界则返回错误信息
            if( (y+g_tCVsr->wGBFontHeight) >CN_LCD_SIZE_Y ) return(-2);

            if( pcString[charindex]>0x7f )
            {// 显示汉字
                if( (x+g_tCVsr->wGBFontWidth) <= CN_LCD_SIZE_X )
                {
                    Lcd_GB2312( byScreen,x,y,
                                (uint16)((pcString[charindex]<<8)+pcString[charindex+1]) );
                    charindex += 2;
                    x += g_tCVsr->wGBFontWidth;
                }
				else
                {// 横坐标不足一个字
                    if(g_tDspAttrib.style==0)
                    {
                        for( j=0;j<g_tCVsr->wGBFontHeight;j++ )
                        {//擦除本行剩余部分
                            for(i=x;i<CN_LCD_SIZE_X;i++)
                            {
                                Lcd_Pub_PixelBK(i,y+j);
                            }
                        }
                    }
					else
                    {
                        for (j=0;j<g_tCVsr->wGBFontHeight;j++)
                        {//擦除本行剩余部分
                            for(i=x;i<CN_LCD_SIZE_X;i++)
                            {
                                Lcd_Pub_PixelFT(i,y+j);
                            }
                        }
                    }
                    err=-1;

                    for( LineOver=0; LineOver<100; LineOver++ )
                    {//搜索换行符
                        charindex++;
                        if( pcString[charindex]==0x0a )
                        {//找到换行符，换行。
                            x = bakx;
                            y += g_tCVsr->wGBFontHeight+g_tDspAttrib.rowSpace;
                            charindex++;
                            break;
                        }
                        if(pcString[charindex]==0x0) return(err);
                    }
                    //LineOver>=100表明没有搜索到换行符，返回错误。
                    if( LineOver>=100 )  return(err);
                }
            }
			else
            { // 显示字符
                if(pcString[charindex]==0x0a)
                {
                    x=bakx;
                    y+=g_tCVsr->wGBFontHeight+g_tDspAttrib.rowSpace;
                    charindex++;
                }
                else
                {
                    if(pcString[charindex]<0x10)
                    {//小于0x10，不处理
                        charindex++;
                        continue;
                    }
                    if(x>CN_LCD_SIZE_X-g_tCVsr->wASCIIFontWidth)
                    /*横坐标不足一个字符*/
                    {
                        if(g_tDspAttrib.style==0)
                        {
                            for (j=0;j<g_tCVsr->wGBFontHeight;j++)
                            {//擦除本行剩余部分
                                for(i=x;i<CN_LCD_SIZE_X;i++)
                                {
                                    Lcd_Pub_PixelBK(i,y+j);
                                }
                            }
                        }else
                        {
                            for (j=0;j<g_tCVsr->wGBFontHeight;j++)
                            {//擦除本行剩余部分
                                for(i=x;i<CN_LCD_SIZE_X;i++)
                                {
                                    Lcd_Pub_PixelFT(i,y+j);
                                }
                            }
                        }
                        err=-1;
                        for(LineOver=0;LineOver<100;LineOver++)
                        {//搜索换行符
                            charindex++;
                            if(pcString[charindex]==0x0a)
                            {//找到换行符，换行。
                                x=bakx;
                                y+=g_tCVsr->wGBFontHeight+g_tDspAttrib.rowSpace;
                                charindex++;
                                break;
                            }
                            if(pcString[charindex]==0x0)
                                return(err);
                        }
                        //LineOver>=100表明没有搜索到换行符，返回错误。
                        if( LineOver>=100 )   return( err );
                    }
					else
                    {
						if(g_tDspAttrib.style==0)
                        {
                            for (j=0;j<(g_tCVsr->wGBFontHeight-g_tCVsr->wASCIIFontHeight);j++)
                            {//下对齐，擦除汉字高出字符的部分
                                for(i=0;i<g_tCVsr->wASCIIFontWidth;i++)
                                {
                                    Lcd_Pub_PixelBK(x+i,y+j);
                                }
                            }
                        }
                        else
                        {// 反显
                            for (j=0;j<(g_tCVsr->wGBFontHeight-g_tCVsr->wASCIIFontHeight);j++)
                            {//下对齐，擦除汉字高出字符的部分
                                for(i=0;i<g_tCVsr->wASCIIFontWidth;i++)
                                {
                                    Lcd_Pub_PixelFT(x+i,y+j);
                                }
                            }
                        }
                        Lcd_Ascii(byScreen, x,(uint16)(y+g_tCVsr->wGBFontHeight-g_tCVsr->wASCIIFontHeight),\
                                    pcString[charindex]);
                        charindex++;x+=g_tCVsr->wASCIIFontWidth;
                    }
                }
            }
        }
    }else if((g_tDspAttrib.StringDirect==0)&&(g_tDspAttrib.Rotate==1))
    {   //字符旋转，字符串横向,坐标为字符串左上角坐标
        if(x>CN_LCD_SIZE_X-g_tCVsr->wGBFontHeight)return(-1);   //起始X坐标越界，直接返回
        while(pcString[charindex]!=0x0)
        {
            /*若坐标越界则返回错误信息*/
            if(pcString[charindex]>0x7f)
            {/*显示汉字*/
                if( (y+g_tCVsr->wGBFontWidth)>=CN_LCD_SIZE_Y ) return(-1);
                if( (x+g_tCVsr->wGBFontWidth)> CN_LCD_SIZE_X )
                { // 横坐标可以显示一个字
                    Lcd_GB2312( byScreen, x,(uint16)(y+g_tCVsr->wGBFontWidth-1),
                            (uint16)(pcString[charindex]*256+pcString[charindex+1]) );
                    charindex+=2;x+=g_tCVsr->wGBFontWidth;
                }else
                {  /*横坐标不足一个字*/
                    if(g_tDspAttrib.style==0)
                        for (j=0;j<g_tCVsr->wGBFontWidth;j++)
                        {//擦除本行剩余部分
                            for(i=x;i<CN_LCD_SIZE_X;i++)
                            {
                                Lcd_Pub_PixelBK(i,y+j);
                            }
                        }
                    else
                        for (j=0;j<g_tCVsr->wGBFontWidth;j++)
                        {//擦除本行剩余部分
                            for(i=x;i<CN_LCD_SIZE_X;i++)
                            {
                                Lcd_Pub_PixelFT(i,y+j);
                            }
                        }
                    err=-1;
                    for(LineOver=0;LineOver<100;LineOver++)
                    {//搜索换行符
                        charindex++;
                        if(pcString[charindex]==0x0a)
                        {//找到换行符，换行。
                            x=bakx;
                            y+=g_tCVsr->wGBFontWidth+g_tDspAttrib.rowSpace;
                            charindex++;
                            break;
                        }
                        if(pcString[charindex]==0x0)
                            return(err);
                    }
                    //LineOver>=100表明没有搜索到换行符，返回错误。
                    if( LineOver>=100 )   return( err );
                }
            }
            else
            { /*显示字符*/
                if(pcString[charindex]==0x0a)
               {
                    x=bakx;
                    y+=g_tCVsr->wGBFontWidth+g_tDspAttrib.rowSpace;
                    charindex++;

                }else if(pcString[charindex]<0x10)
                {//小于0x10，不处理
                    charindex++;
                    continue;

                }else if( (x+g_tCVsr->wGBFontWidth) < CN_LCD_SIZE_X )
                {// 横坐标可以显示一个字符
                    if(g_tDspAttrib.style==0)
                        for (j=0;j<(g_tCVsr->wGBFontWidth-g_tCVsr->wASCIIFontWidth);j++)
                        {//下对齐，擦除汉字高出字符的部分
                            for(i=0;i<g_tCVsr->wASCIIFontHeight;i++)
                            {
                                Lcd_Pub_PixelBK(x+i,y+j);
                            }
                        }
                    else
                        for (j=0;j<(g_tCVsr->wGBFontWidth-g_tCVsr->wASCIIFontWidth);j++)
                        {//下对齐，擦除汉字高出字符的部分
                            for(i=0;i<g_tCVsr->wASCIIFontHeight;i++)
                            {
                                Lcd_Pub_PixelFT(x+i,y+j);
                            }
                        }
                    Lcd_Ascii(byScreen, x,(uint16)(y+g_tCVsr->wGBFontWidth-1), pcString[charindex]);
                    charindex++;x+=g_tCVsr->wASCIIFontHeight;

                }else
                { // 横坐标不足一个字符
                    if(g_tDspAttrib.style==0)
                        for (j=0;j<g_tCVsr->wGBFontWidth;j++)
                        {//擦除本行剩余部分
                            for(i=x;i<CN_LCD_SIZE_X;i++)
                            {
                                Lcd_Pub_PixelBK(i,y+j);
                            }
                        }
                    else
                        for (j=0;j<g_tCVsr->wGBFontWidth;j++)
                        {//擦除本行剩余部分
                            for(i=x;i<CN_LCD_SIZE_X;i++)
                            {
                                Lcd_Pub_PixelFT(i,y+j);
                            }
                        }

                    err=-1;
                    for(LineOver=0;LineOver<100;LineOver++)
                    {//搜索换行符
                        charindex++;
                        if(pcString[charindex]==0x0a)
                        {//找到换行符，换行。
                            x=bakx;
                            y+=g_tCVsr->wGBFontWidth+g_tDspAttrib.rowSpace;
                            charindex++;
                            break;
                        }
                        if(pcString[charindex]==0x0)    return(err);
                    }
                    //LineOver>=100表明没有搜索到换行符，返回错误。
                    if( LineOver>=100 )   return( err );
                }
            } // end if( >0x7F ) else( .. )
        } // end while( )
    }
	else if((g_tDspAttrib.StringDirect==1)&&(g_tDspAttrib.Rotate==0))
    {   //字符不旋转，字符串竖向（自上至下）,坐标为字符串右上角坐标
        if(y>CN_LCD_SIZE_Y-g_tCVsr->wGBFontHeight)return(-2);   //起始Y坐标越界，直接返回
        while(pcString[charindex]!=0x0)
        {
            /*若坐标越界则返回错误信息*/
//          if(x<g_tCVsr->wGBFontWidth-1) return(-2);
            if(pcString[charindex]>0x7f)
            {  /*显示汉字*/
                if(y>CN_LCD_SIZE_Y-g_tCVsr->wGBFontHeight)
                /*纵坐标不足一个字*/
                {
                    if(g_tDspAttrib.style==0)
                        for (j=0;j<g_tCVsr->wGBFontWidth;j++)
                        {//擦除本行剩余部分
                            for(i=y;i<CN_LCD_SIZE_Y;i++)
                            {
                                Lcd_Pub_PixelBK(x-j,i);
                            }
                        }
                    else
                        for (j=0;j<g_tCVsr->wGBFontWidth;j++)
                        {//擦除本行剩余部分
                            for(i=y;i<CN_LCD_SIZE_Y;i++)
                            {
                                Lcd_Pub_PixelFT(x-j,i);
                            }
                        }
                    err=-1;
                    for(LineOver=0;LineOver<100;LineOver++)
                    {//搜索换行符
                        charindex++;
                        if(pcString[charindex]==0x0a)
                        {//找到换行符，换行。
                            y=baky;
                            x-=g_tCVsr->wGBFontWidth+g_tDspAttrib.rowSpace;
                            charindex++;
                            break;
                        }
                        if(pcString[charindex]==0x0)
                            return(err);
                    }
                    //LineOver>=100表明没有搜索到换行符，返回错误。
                    if( LineOver>=100 )   return(err);
                }else
                {
                    Lcd_GB2312(byScreen, (uint16)(x-g_tCVsr->wGBFontWidth+1),y,
                            (uint16)(pcString[charindex]*256+pcString[charindex+1]) );
                    charindex+=2;y+=g_tCVsr->wGBFontHeight;
                }
            }
            else
            /*显示字符*/
            {
                if(pcString[charindex]==0x0a)
                {
                    y=baky;
                    x-=g_tCVsr->wGBFontWidth+g_tDspAttrib.rowSpace;
                    charindex++;
                }
                else
                {
                    if(pcString[charindex]<0x10)
                    {//小于0x10，不处理
                        charindex++;
                        continue;
                    }
                    if(y>CN_LCD_SIZE_Y-g_tCVsr->wASCIIFontHeight)
                    /*纵坐标不足一个字符*/
                    {
                        if(g_tDspAttrib.style==0)
                            for (j=0;j<g_tCVsr->wGBFontWidth;j++)
                            {//擦除本行剩余部分
                                for(i=y;i<CN_LCD_SIZE_Y;i++)
                                {
                                    Lcd_Pub_PixelBK(x-j,i);
                                }
                            }
                        else
                            for (j=0;j<g_tCVsr->wGBFontWidth;j++)
                            {//擦除本行剩余部分
                                for(i=y;i<CN_LCD_SIZE_Y;i++)
                                {
                                    Lcd_Pub_PixelFT(x-j,i);
                                }
                            }
                        err=-1;
                        for(LineOver=0;LineOver<100;LineOver++)
                        {//搜索换行符
                            charindex++;
                            if(pcString[charindex]==0x0a)
                            {//找到换行符，换行。
                                y=baky;
                                x-=g_tCVsr->wGBFontWidth+g_tDspAttrib.rowSpace;
                                charindex++;
                                break;
                            }
                            if(pcString[charindex]==0x0)
                                return(err);
                        }
                        //LineOver>=100表明没有搜索到换行符，返回错误。
                        if( LineOver>=100 )   return( err );
                    }else
                    {
                        if(g_tDspAttrib.style==0)
                            for (j=0;j<g_tCVsr->wASCIIFontHeight;j++)
                            {//下对齐，擦除汉字高出字符的部分
                                for(i=0;i<(g_tCVsr->wGBFontWidth-g_tCVsr->wASCIIFontWidth);i++)
                                {
                                    Lcd_Pub_PixelBK(x-i,y+j);
                                }
                            }
                        else
                            for (j=0;j<g_tCVsr->wASCIIFontHeight;j++)
                            {//下对齐，擦除汉字高出字符的部分
                                for(i=0;i<(g_tCVsr->wGBFontWidth-g_tCVsr->wASCIIFontWidth);i++)
                                {
                                    Lcd_Pub_PixelFT(x-i,y+j);
                                }
                            }
                        Lcd_Ascii(byScreen, (uint16)(x-g_tCVsr->wGBFontWidth+1),y, pcString[charindex]);
                        charindex++;
                        y+=g_tCVsr->wASCIIFontHeight;
                    }
                }
            }
        }
    }
	else if((g_tDspAttrib.StringDirect==1)&&(g_tDspAttrib.Rotate==1))
    {   //字符旋转，字符串竖向（自下至上）,坐标为字符串左下角坐标
        if(y<g_tCVsr->wASCIIFontWidth-1)return(-2);   //起始Y坐标越界，直接返回
        while(pcString[charindex]!=0x0)
        {
            /*若坐标越界则返回错误信息*/
            if(x>CN_LCD_SIZE_X-g_tCVsr->wGBFontHeight) return(-2);
            if(pcString[charindex]>0x7f)
            /*显示汉字*/
            {
                if(y<g_tCVsr->wGBFontWidth-1)
                /*纵坐标不足一个字*/
                {
                    if(g_tDspAttrib.style==0)
                        for (j=0;j<=y;j++)
                        {//擦除本行剩余部分
                            for(i=0;i<g_tCVsr->wGBFontHeight;i++)
                            {
                                Lcd_Pub_PixelBK(x+i,j);
                            }
                        }
                    else
                        for (j=0;j<=y;j++)
                        {//擦除本行剩余部分
                            for(i=0;i<g_tCVsr->wGBFontHeight;i++)
                            {
                                Lcd_Pub_PixelFT(x+i,j);
                            }
                        }
                    err=-1;
                    for(LineOver=0;LineOver<100;LineOver++)
                    {//搜索换行符
                        charindex++;
                        if(pcString[charindex]==0x0a)
                        {//找到换行符，换行。
                            y=baky;
                            x+=g_tCVsr->wGBFontHeight+g_tDspAttrib.rowSpace;
                            charindex++;
                            break;
                        }
                        if(pcString[charindex]==0x0)
                            return(err);
                    }
                    //LineOver>=100表明没有搜索到换行符，返回错误。
                    if( LineOver>=100 )   return( err );

                }else
                {
                    Lcd_GB2312(byScreen,x,y, (uint16)(pcString[charindex]*256+pcString[charindex+1]) );
                    charindex+=2;y-=g_tCVsr->wGBFontWidth;
                }
            }
            else
            /*显示字符*/
            {
                if(pcString[charindex]==0x0a)
                {
                    y=baky;
                    x+=g_tCVsr->wGBFontHeight+g_tDspAttrib.rowSpace;
                    charindex++;
                }
                else
                {
                    if(pcString[charindex]<0x10)
                    {//小于0x10，不处理
                        charindex++;
                        continue;
                    }
                    if(y<g_tCVsr->wASCIIFontWidth-1)
                    /*纵坐标不足一个字符*/
                    {
                        if(g_tDspAttrib.style==0)
                            for (j=0;j<=y;j++)
                            {//擦除本行剩余部分
                                for(i=0;i<g_tCVsr->wGBFontHeight;i++)
                                {
                                    Lcd_Pub_PixelBK(x+i,j);
                                }
                            }
                        else
                            for (j=0;j<=y;j++)
                            {//擦除本行剩余部分
                                for(i=0;i<g_tCVsr->wGBFontHeight;i++)
                                {
                                    Lcd_Pub_PixelFT(x+i,j);
                                }
                            }
                        err=-1;
                        for(LineOver=0;LineOver<100;LineOver++)
                        {//搜索换行符
                            charindex++;
                            if(pcString[charindex]==0x0a)
                            {//找到换行符，换行。
                                y=baky;
                                x+=g_tCVsr->wGBFontWidth+g_tDspAttrib.rowSpace;
                                charindex++;
                                break;
                            }
                            if(pcString[charindex]==0x0)
                                return(err);
                        }
                        //LineOver>=100表明没有搜索到换行符，返回错误。
                        if( LineOver>=100 )   return( err );
                    }else
                    {
                        if(g_tDspAttrib.style==0)
                            for (j=0;j<g_tCVsr->wASCIIFontWidth;j++)
                            {//下对齐，擦除汉字高出字符的部分
                                for(i=0;i<(g_tCVsr->wGBFontHeight-g_tCVsr->wASCIIFontHeight);i++)
                                {
                                    Lcd_Pub_PixelBK(x+i,y-j);
                                }
                            }
                        else
                            for (j=0;j<g_tCVsr->wASCIIFontWidth;j++)
                            {//下对齐，擦除汉字高出字符的部分
                                for(i=0;i<(g_tCVsr->wGBFontHeight-g_tCVsr->wASCIIFontHeight);i++)
                                {
                                    Lcd_Pub_PixelFT(x+i,y-j);
                                }
                            }
                        Lcd_Ascii(byScreen, (uint16)(x+g_tCVsr->wGBFontHeight-g_tCVsr->wASCIIFontHeight),y,\
                                    pcString[charindex]);
                        charindex++;y-=g_tCVsr->wASCIIFontWidth;
                    }
                }
            }
        }

    }
    return(0);
}

//==============================================================================
//返回：正确显示时返回0；x坐标超出范围返回-2，y坐标超出范围返回-1。
//参数：byScreen，无符号字符，虚拟界面编号。
//x、y，无符号短整数，显示字符的坐标位置。
//wAttrib，显示附加属性(含义见显示附件属性说明)。
//pcString，字符串指针，以NULL结束。
//功能：使用系统缺省字体和当前系统字体颜色显示字符串；函数根据字符编码自动识别是英文
//还是中文字符。
//对于非显示点(点阵值为0)，则不填充背景色(即透明显示，在原来显示基础上叠加)。
//如果字符串中间包含换行符号
//(ASCII数值为0x0A)，则进行换行显示。下一行的横坐标为x，而纵坐标则为当前行的纵坐标、
//缺省字体点阵高度
//(如为8*16点阵则为16)以及行距三者的和。
//==============================================================================
uint16 Lcd_StrTr ( uint8 byScreen, uint16 x, uint16 y,  uint8 *pcString )
{
    uint16 charindex, err,bakx,baky,LineOver;

    bakx=x;
    baky=y;
    Lcd_Pub_GetCurrentSet(byScreen);
    charindex=0;
    if((g_tDspAttrib.StringDirect==0)&&(g_tDspAttrib.Rotate==0))
    {   //字符不旋转，字符串横向,坐标为字符串左上角坐标
        if(x>CN_LCD_SIZE_X-g_tCVsr->wASCIIFontWidth)return(-1);   //起始X坐标越界，直接返回
        while(pcString[charindex]!=0x0)
        {
            /*若坐标越界则返回错误信息*/
            if(y>=CN_LCD_SIZE_Y-g_tCVsr->wGBFontHeight) return(-2);
            if(pcString[charindex]>0x7f)
            /*显示汉字*/
            {
                if(x>CN_LCD_SIZE_X-g_tCVsr->wGBFontWidth)
                /*横坐标不足一个字*/
                {
                    err=-1;
                    for(LineOver=0;LineOver<100;LineOver++)
                    {//搜索换行符
                        charindex++;
                        if(pcString[charindex]==0x0a)
                        {//找到换行符，换行。
                            x=bakx;
                            y+=g_tCVsr->wGBFontHeight+g_tDspAttrib.rowSpace;
                            charindex++;
                            break;
                        }
                        if(pcString[charindex]==0x0)
                            return(err);
                    }
                    //LineOver>=100表明没有搜索到换行符，返回错误。
                    if( LineOver>=100 )   return( err );
                }else
                {
                    Lcd_GB2312Tr(byScreen, x,y, (uint16)(pcString[charindex]*256+pcString[charindex+1]) );
                    charindex+=2;x+=g_tCVsr->wGBFontWidth;
                }
            }
            else
            {/*显示字符*/
                if(pcString[charindex]==0x0a)
                {
                    x=bakx;
                    y+=g_tCVsr->wGBFontHeight+g_tDspAttrib.rowSpace;
                    charindex++;
                }
                else
                {
                    if(pcString[charindex]<0x10)
                    {//小于0x10，不处理
                        charindex++;
                        continue;
                    }
                    if(x>CN_LCD_SIZE_X-g_tCVsr->wASCIIFontWidth)
                    /*横坐标不足一个字符*/
                    {
                        err=-1;
                        for(LineOver=0;LineOver<100;LineOver++)
                        {//搜索换行符
                            charindex++;
                            if(pcString[charindex]==0x0a)
                            {//找到换行符，换行。
                                x=bakx;
                                y+=g_tCVsr->wGBFontHeight+g_tDspAttrib.rowSpace;
                                charindex++;
                                break;
                            }
                            if(pcString[charindex]==0x0)
                                return(err);
                        }
                        //LineOver>=100表明没有搜索到换行符，返回错误。
                        if( LineOver>=100 )   return( err );
                    }else
                    {
                        Lcd_AsciiTr( byScreen,
                                     x,(uint16)(y+g_tCVsr->wGBFontHeight-g_tCVsr->wASCIIFontHeight),
                                     pcString[charindex]);
                        charindex++;x+=g_tCVsr->wASCIIFontWidth;
                    }
                }
            }
        }
    }else if((g_tDspAttrib.StringDirect==1)&&(g_tDspAttrib.Rotate==0))
    {   //字符不旋转，字符串竖向（自上至下）,坐标为字符串右上角坐标
        if(y>CN_LCD_SIZE_Y-g_tCVsr->wGBFontHeight)return(-2);   //起始Y坐标越界，直接返回
        while(pcString[charindex]!=0x0)
        {
            /*若坐标越界则返回错误信息*/
//          if(x<g_tCVsr->wGBFontWidth-1) return(-2);
            if(pcString[charindex]>0x7f)
            /*显示汉字*/
            {
                if(y>CN_LCD_SIZE_Y-g_tCVsr->wGBFontHeight)
                /*纵坐标不足一个字*/
                {
                    err=-1;
                    for(LineOver=0;LineOver<100;LineOver++)
                    {//搜索换行符
                        charindex++;
                        if(pcString[charindex]==0x0a)
                        {//找到换行符，换行。
                            y=baky;
                            x-=g_tCVsr->wGBFontWidth+g_tDspAttrib.rowSpace;
                            charindex++;
                            break;
                        }
                        if(pcString[charindex]==0x0)
                            return(err);
                    }
                    //LineOver>=100表明没有搜索到换行符，返回错误。
                    if( LineOver>=100 )   return( err );
                }else
                {
                    Lcd_GB2312Tr(byScreen, (uint16)(x-g_tCVsr->wGBFontWidth+1),y,\
                            (uint16)(pcString[charindex]*256+pcString[charindex+1]) );
                    charindex+=2;y+=g_tCVsr->wGBFontHeight;
                }
            }
            else
            /*显示字符*/
            {
                if(pcString[charindex]==0x0a)
                {
                    y=baky;
                    x-=g_tCVsr->wGBFontWidth+g_tDspAttrib.rowSpace;
                    charindex++;
                }
                else
                {
                    if(pcString[charindex]<0x10)
                    {//小于0x10，不处理
                        charindex++;
                        continue;
                    }
                    if(y>CN_LCD_SIZE_Y-g_tCVsr->wASCIIFontHeight)
                    /*纵坐标不足一个字符*/
                    {
                        err=-1;
                        for(LineOver=0;LineOver<100;LineOver++)
                        {//搜索换行符
                            charindex++;
                            if(pcString[charindex]==0x0a)
                            {//找到换行符，换行。
                                y=baky;
                                x-=g_tCVsr->wGBFontWidth+g_tDspAttrib.rowSpace;
                                charindex++;
                                break;
                            }
                            if(pcString[charindex]==0x0)
                                return(err);
                        }
                        //LineOver>=100表明没有搜索到换行符，返回错误。
                        if( LineOver>=100 )   return( err );
                    }else
                    {
                        Lcd_AsciiTr(byScreen, (uint16)(x-g_tCVsr->wGBFontWidth+1),y,pcString[charindex]);
                        charindex++;y+=g_tCVsr->wASCIIFontHeight;
                    }
                }
            }
        }
    }else if((g_tDspAttrib.StringDirect==0)&&(g_tDspAttrib.Rotate==1))
    {   //字符旋转，字符串横向,坐标为字符串左上角坐标
        if(x>CN_LCD_SIZE_X-g_tCVsr->wGBFontHeight)return(-1);   //起始X坐标越界，直接返回
        while(pcString[charindex]!=0x0)
        {
            /*若坐标越界则返回错误信息*/
            if(pcString[charindex]>0x7f)
            /*显示汉字*/
            {
                if(y>=CN_LCD_SIZE_Y-g_tCVsr->wGBFontWidth) return(-1);
                if(x>CN_LCD_SIZE_X-g_tCVsr->wGBFontWidth)
                /*横坐标不足一个字*/
                {
                    err=-1;
                    for(LineOver=0;LineOver<100;LineOver++)
                    {//搜索换行符
                        charindex++;
                        if(pcString[charindex]==0x0a)
                        {//找到换行符，换行。
                            x=bakx;
                            y+=g_tCVsr->wGBFontWidth+g_tDspAttrib.rowSpace;
                            charindex++;
                            break;
                        }
                        if(pcString[charindex]==0x0)
                            return(err);
                    }
                    //LineOver>=100表明没有搜索到换行符，返回错误。
                    if( LineOver>=100 )   return( err );
                }else
                {
                    Lcd_GB2312Tr(byScreen,x,(uint16)(y+g_tCVsr->wGBFontWidth-1),
                            (uint16)(pcString[charindex]*256+pcString[charindex+1]) );
                    charindex+=2;x+=g_tCVsr->wGBFontWidth;
                }
            }
            else
            /*显示字符*/
            {
                if(pcString[charindex]==0x0a)
                {
                    x=bakx;
                    y+=g_tCVsr->wGBFontWidth+g_tDspAttrib.rowSpace;
                    charindex++;
                }
                else
                {
                    if(pcString[charindex]<0x10)
                    {//小于0x10，不处理
                        charindex++;
                        continue;
                    }
                    if(x>CN_LCD_SIZE_X-g_tCVsr->wGBFontWidth)
                    /*横坐标不足一个字符*/
                    {
                        err=-1;
                        for(LineOver=0;LineOver<100;LineOver++)
                        {//搜索换行符
                            charindex++;
                            if(pcString[charindex]==0x0a)
                            {//找到换行符，换行。
                                x=bakx;
                                y+=g_tCVsr->wGBFontWidth+g_tDspAttrib.rowSpace;
                                charindex++;
                                break;
                            }
                            if(pcString[charindex]==0x0)
                                return(err);
                        }
                        //LineOver>=100表明没有搜索到换行符，返回错误。
                        if( LineOver>=100 )   return( err );
                    }else
                    {
                        Lcd_AsciiTr(byScreen,x,(uint16)(y+g_tCVsr->wGBFontWidth-1),\
                                    pcString[charindex]);
                        charindex++;x+=g_tCVsr->wASCIIFontHeight;
                    }

                }
            }
        }
    }else if((g_tDspAttrib.StringDirect==1)&&(g_tDspAttrib.Rotate==1))
    {   //字符旋转，字符串竖向（自下至上）,坐标为字符串左下角坐标
        if(y<g_tCVsr->wASCIIFontWidth-1)return(-2);   //起始Y坐标越界，直接返回
        while(pcString[charindex]!=0x0)
        {
            /*若坐标越界则返回错误信息*/
            if(x>CN_LCD_SIZE_X-g_tCVsr->wGBFontHeight) return(-2);
            if(pcString[charindex]>0x7f)
            /*显示汉字*/
            {
                if(y<g_tCVsr->wGBFontWidth-1)
                /*纵坐标不足一个字*/
                {
                    err=-1;
                    for(LineOver=0;LineOver<100;LineOver++)
                    {//搜索换行符
                        charindex++;
                        if(pcString[charindex]==0x0a)
                        {//找到换行符，换行。
                            y=baky;
                            x+=g_tCVsr->wGBFontHeight+g_tDspAttrib.rowSpace;
                            charindex++;
                            break;
                        }
                        if(pcString[charindex]==0x0)
                            return(err);
                    }
                    //LineOver>=100表明没有搜索到换行符，返回错误。
                    if( LineOver>=100 )   return( err );
                }else
                {
                    Lcd_GB2312Tr( byScreen, x,y,
                                  (uint16)(pcString[charindex]*256+pcString[charindex+1]) );
                    charindex+=2;y-=g_tCVsr->wGBFontWidth;
                }
            }
            else
            /*显示字符*/
            {
                if(pcString[charindex]==0x0a)
                {
                    y=baky;
                    x+=g_tCVsr->wGBFontHeight+g_tDspAttrib.rowSpace;
                    charindex++;
                }
                else
                {
                    if(pcString[charindex]<0x10)
                    {//小于0x10，不处理
                        charindex++;
                        continue;
                    }
                    if(y<g_tCVsr->wASCIIFontWidth-1)
                    /*纵坐标不足一个字符*/
                    {
                        err=-1;
                        for(LineOver=0;LineOver<100;LineOver++)
                        {//搜索换行符
                            charindex++;
                            if(pcString[charindex]==0x0a)
                            {//找到换行符，换行。
                                y=baky;
                                x+=g_tCVsr->wGBFontWidth+g_tDspAttrib.rowSpace;
                                charindex++;
                                break;
                            }
                            if(pcString[charindex]==0x0)
                                return(err);
                        }
                        //LineOver>=100表明没有搜索到换行符，返回错误。
                        if( LineOver>=100 )   return( err );
                    }else
                    {
                        Lcd_AsciiTr(byScreen,(uint16)(x+g_tCVsr->wGBFontHeight-g_tCVsr->wASCIIFontHeight),y,\
                                    pcString[charindex]);
                        charindex++;y-=g_tCVsr->wASCIIFontWidth;
                    }
                }
            }
        }

    }
    return(0);
}

//==============================================================================
//返回：正确显示时返回0；x坐标超出范围返回-2，y坐标超出范围返回-1。byBitNum错返回-3
//参数：byScreen，无符号字符，虚拟界面编号。
//x、y，无符号短整数，显示字符的点阵坐标位置。
//wAttrib，显示附加属性(含义见显示附件属性说明)。
//dwData，无符号长整数，所要显示的数据。
//byBitNum，数据的字符长度；位数不足在前面补零。
//byDotPos，小数点位置；从右向左数；若大于等于byBitNum则无小数点，即整数位数。
//功能：使用系统缺省字体和当前系统设置以十进制方式显示定点数（含小数点，如有）。
//若超出屏幕时，显示屏幕范围内的部分。
//==============================================================================
uint16 Lcd_Dec ( uint8 byScreen, uint16 x, uint16 y,  uint32 dwData, uint8 byBitNum, uint8 byDotPos )
{
    char buf[20];
    uint16 wBitX, i,err,l,h;
    int strl;

    if( (byBitNum>12) || (byBitNum==0) ) return(-3);  // 长整数最多10，用于容错
    if( (y+g_tCVsr->wASCIIFontHeight) >CN_LCD_SIZE_Y ) return(-1);  //列坐标越界,直接返回.
    if( (x+g_tCVsr->wASCIIFontWidth ) >CN_LCD_SIZE_X ) return(-2);  //行越界，直接返回

    Lcd_Pub_GetCurrentSet( byScreen );

    g_tDspAttrib.StringDirect=0;     // 屏蔽不需要的属性，防止出错
    g_tDspAttrib.Rotate=0;

    err=0;
    if( dwData<65536 )  sprintf(buf,"%u",(uint16)dwData);
    else                sprintf(buf,"%lu",(unsigned long)dwData);
    strl=strlen(buf)-1;

    wBitX = x + byBitNum * g_tCVsr->wASCIIFontWidth;    // 当前位的X坐标初始值
    for( i=byBitNum; i>0; i-- )
    {
        wBitX = wBitX - g_tCVsr->wASCIIFontWidth;       // 当前位的X坐标:x+(i-1)*g_tCVsr->wASCIIFontWidth
        if( (wBitX+g_tCVsr->wASCIIFontWidth) <= CN_LCD_SIZE_X )
        {// 当前位的最后象素未超出屏幕
            if( i==(byBitNum-byDotPos) )
            {
                Lcd_Ascii(byScreen, wBitX,y, '.' );
            }else
            {
                if( strl>=0 )
                {
                    Lcd_Ascii(byScreen, wBitX,y, buf[strl]);
                }else
                {
                    if( (i>=(byBitNum-byDotPos)-1) || (g_tDspAttrib.fillZero==1) )
                        Lcd_Ascii(byScreen, wBitX,y, '0' );
                    else
                        Lcd_Ascii(byScreen, wBitX,y, ' ' );
                }
            }
        }else
        {// 超出屏幕，不显示，在屏幕内但不足一个字符则擦除剩余部分。
            err=-2;
            if(g_tDspAttrib.style==0)
            {
                for(h=0;h<g_tCVsr->wASCIIFontHeight;h++)
                {
                    for(l=wBitX;l<CN_LCD_SIZE_X;l++)
                        Lcd_Pub_PixelBK(l,y+h);
                }
            }else
            {
                for(h=0;h<g_tCVsr->wASCIIFontHeight;h++)
                {
                    for(l=wBitX;l<CN_LCD_SIZE_X;l++)
                        Lcd_Pub_PixelFT(l,y+h);
                }
            }
        }
        if( i!=(byBitNum-byDotPos) )  strl--;       // 小数点不包含在临时字符串中
    }

    return err;
}



//==============================================================================
//返回：正确显示时返回0；x坐标超出范围返回-2，y坐标超出范围返回-1。
//参数：byScreen，无符号字符，虚拟界面编号。
//x、y，无符号短整数，显示字符的点阵坐标位置。
//wAttrib，显示附加属性(含义见显示附件属性说明)。
//dwData，无符号长整数，所要显示的数据。
//byBitNum，数据的字符长度；位数不足在前面补零。
//功能：使用系统缺省字体和当前系统设置以十六进制方式显示数据；字符大写，无0x前缀。
//若超出屏幕时，显示屏幕范围内的部分。
//==============================================================================
uint16 Lcd_Hex ( uint8 byScreen, uint16 x, uint16 y,  uint32 dwData, uint8 byBitNum )
{
    char buf[20];
    uint16 wBitX, i,err,l,h;
    int  strl;

    if( (byBitNum>12) || (byBitNum==0) ) return(-3);  // 长整数最多10，用于容错
    Lcd_Pub_GetCurrentSet(byScreen);
    if (y>CN_LCD_SIZE_Y-g_tCVsr->wASCIIFontHeight) return(-1); //列坐标越界,直接返回.
    if(x>CN_LCD_SIZE_X-g_tCVsr->wASCIIFontWidth) return(-2);  //行越界，直接返回
    g_tDspAttrib.StringDirect=0;     //屏蔽不需要的属性，防止出错
    g_tDspAttrib.Rotate=0;
    err=0;
    if(dwData<65536)
        sprintf(buf,"%X",(uint16)dwData);
    else
        sprintf(buf,"%lX",(long)dwData);
    strl=strlen(buf)-1;
//  for(i=0;i<=strl;i++)
//  {
//      if (islower(buf[i]))
//          buf[i]=toupper(buf[i]);
//  }

    wBitX = x + byBitNum * g_tCVsr->wASCIIFontWidth;    // 当前位的X坐标初始值
    for( i=byBitNum;i>0;i-- )
    {
        wBitX = wBitX -g_tCVsr->wASCIIFontWidth;        // 当前位的X坐标:x+(i-1)*g_tCVsr->wASCIIFontWidth
        if( (wBitX+g_tCVsr->wASCIIFontWidth) <=CN_LCD_SIZE_X )
        {// 当前位的最后象素未超出屏幕
            if(strl>=0)
                Lcd_Ascii(byScreen, wBitX,y,buf[strl]);
            else
                Lcd_Ascii(byScreen, wBitX,y,'0');

        }else
        {// 超出屏幕，不显示，在屏幕内但不足一个字符则擦除剩余部分。
            err=-2;
            if(g_tDspAttrib.style==0)
            {
                for(h=0;h<g_tCVsr->wASCIIFontHeight;h++)
                {
                    for(l=wBitX; l<CN_LCD_SIZE_X;l++)
                        Lcd_Pub_PixelBK(l,y+h);
                }
            }else
            {
                for(h=0;h<g_tCVsr->wASCIIFontHeight;h++)
                {
                    for(l=wBitX;l<CN_LCD_SIZE_X;l++ )
                        Lcd_Pub_PixelFT(l,y+h);
                }
            }
        }
        strl--;
    }

    return err;
}


//==============================================================================
//返回：无。
//参数：byScreen，无符号字符，虚拟界面编号。
//lx、ty，无符号短整数，矩形左上角的坐标。
//rx、by，无符号短整数，矩形右下角的坐标。
//dwRGB，边框颜色。
//功能：使用系统当前设置画一矩形。x或y坐标超出范围则取液晶许可的最大值。
//==============================================================================
void Lcd_Rect( uint8 byScreen, uint16 lx, uint16 ty, uint16 rx, uint16 by, uint32 dwRGB )
{
    uint32 dwRRGB;
    dwRRGB=Lcd_GetFontColor(byScreen);
    Lcd_SetFontColor(byScreen,dwRGB);   //设置当前颜色设置
    Lcd_LineH( byScreen, lx, rx,ty, 1 );
    Lcd_LineH( byScreen, lx, rx + 1,by, 1 );
    Lcd_LineV( byScreen, ty, by,lx, 1 );
    Lcd_LineV( byScreen, ty, by,rx, 1 );
    Lcd_SetFontColor(byScreen,dwRRGB);  //恢复当前颜色设置
}
//==============================================================================
//返回：无。
//参数：byScreen，无符号字符，虚拟界面编号。
//lx、ty，无符号短整数，矩形左上角的坐标。
//rx、by，无符号短整数，矩形右下角的坐标。
//dwRGB，无符号短整数，点阵的显示数据。
//功能：用点阵数据填充矩形区域。x或y坐标超出范围则取液晶许可的最大值。
//==============================================================================
void Lcd_FillRect( uint8 byScreen, uint16 lx, uint16 ty, uint16 rx, uint16 by, uint32 dwRGB)
{
    uint16 h,l,err;
    uint32 dwRRGB;
    dwRRGB=Lcd_GetFontColor(byScreen);
    Lcd_SetFontColor(byScreen,dwRGB);   //设置当前颜色设置
    Lcd_Pub_GetCurrentSet(byScreen);

    err = Lcd_Pub_CheckPosi( &lx, CN_LCD_SIZE_X, -2 );
    err = Lcd_Pub_CheckPosi( &rx, CN_LCD_SIZE_X, -2 );
    err = Lcd_Pub_CheckPosi( &ty, CN_LCD_SIZE_Y, -1 );
    err = Lcd_Pub_CheckPosi( &by, CN_LCD_SIZE_Y, -1 );

    for(h=ty;h<=by;h++)
    {
        for(l=lx;l<=rx;l++)
            Lcd_Pub_PixelFT(l,h);
    }
    Lcd_SetFontColor(byScreen,dwRRGB);  //恢复当前颜色设置
}

//==============================================================================
//返回：无。
//参数：byScreen，无符号字符，虚拟界面编号。
//lx、ty，无符号短整数，矩形左上角的坐标。
//rx、by，无符号短整数，矩形右下角的坐标。
//dwRGB，按钮颜色。
//功能：使用系统当前设置画一白色边框矩形。x或y坐标超出范围则取液晶许可的最大值。按钮用dwRGB填充。
//==============================================================================
void Lcd_White_Button( uint8 byScreen, uint16 lx, uint16 ty, uint16 rx, uint16 by, uint32 dwRGB )
{
    uint16 h,l,err;
    uint32 dwRRGB;
    dwRRGB=Lcd_GetFontColor(byScreen);
    Lcd_SetFontColor(byScreen,dwRGB);   //设置当前颜色设置
    Lcd_Pub_GetCurrentSet(byScreen);

    err = Lcd_Pub_CheckPosi( &lx, CN_LCD_SIZE_X, -2 );
    err = Lcd_Pub_CheckPosi( &rx, CN_LCD_SIZE_X, -2 );
    err = Lcd_Pub_CheckPosi( &ty, CN_LCD_SIZE_Y, -1 );
    err = Lcd_Pub_CheckPosi( &by, CN_LCD_SIZE_Y, -1 );

    for(h=ty+1;h<by;h++)
    {
        for(l=lx+1;l<rx;l++)
            Lcd_Pub_PixelFT(l,h);
    }
    Lcd_SetFontColor(byScreen,CN_RGB_WHITE);   //设置左侧及上侧线颜色设置为白色
    Lcd_LineH( byScreen, lx, rx,ty, 1 );
    Lcd_LineV( byScreen, ty, by,lx, 1 );
    Lcd_SetFontColor(byScreen,CN_RGB_WHITE);   //设置下侧及右侧线颜色设置为黑色
    Lcd_LineH( byScreen, lx, rx + 1,by, 1 );
    Lcd_LineV( byScreen, ty, by,rx, 1 );
    Lcd_SetFontColor(byScreen,dwRRGB);  //恢复当前颜色设置
}

//==============================================================================
//返回：无。
//参数：byScreen，无符号字符，虚拟界面编号。
//lx、ty，无符号短整数，矩形左上角的坐标。
//rx、by，无符号短整数，矩形右下角的坐标。
//dwRGB，按钮颜色。
//功能：使用系统当前设置画一黑色边框矩形。x或y坐标超出范围则取液晶许可的最大值。按钮用dwRGB填充。
//==============================================================================
void Lcd_Black_Button( uint8 byScreen, uint16 lx, uint16 ty, uint16 rx, uint16 by, uint32 dwRGB )
{
    uint16 h,l,err;
    uint32 dwRRGB;
    dwRRGB=Lcd_GetFontColor(byScreen);
    Lcd_SetFontColor(byScreen,dwRGB);   //设置当前颜色设置
    Lcd_Pub_GetCurrentSet(byScreen);

    err = Lcd_Pub_CheckPosi( &lx, CN_LCD_SIZE_X, -2 );
    err = Lcd_Pub_CheckPosi( &rx, CN_LCD_SIZE_X, -2 );
    err = Lcd_Pub_CheckPosi( &ty, CN_LCD_SIZE_Y, -1 );
    err = Lcd_Pub_CheckPosi( &by, CN_LCD_SIZE_Y, -1 );

    for(h=ty+1;h<by;h++)
    {
        for(l=lx+1;l<rx;l++)
            Lcd_Pub_PixelFT(l,h);
    }
    Lcd_SetFontColor(byScreen,CN_RGB_BLACK);   //设置左侧及上侧线颜色设置为白色
    Lcd_LineH( byScreen, lx, rx,ty, 1 );
    Lcd_LineV( byScreen, ty, by,lx, 1 );
    Lcd_SetFontColor(byScreen,CN_RGB_BLACK);   //设置下侧及右侧线颜色设置为黑色
    Lcd_LineH( byScreen, lx, rx + 1,by, 1 );
    Lcd_LineV( byScreen, ty, by,rx, 1 );
    Lcd_SetFontColor(byScreen,dwRRGB);  //恢复当前颜色设置
}

//==============================================================================
//返回：无。
//参数：byScreen，无符号字符，虚拟界面编号。
//lx、ty，无符号短整数，矩形左上角的坐标。
//rx、by，无符号短整数，矩形右下角的坐标。
//dwRGB，按钮颜色。
//功能：使用系统当前设置画一弹起按钮。x或y坐标超出范围则取液晶许可的最大值。按钮用dwRGB填充。
//==============================================================================
void Lcd_Button( uint8 byScreen, uint16 lx, uint16 ty, uint16 rx, uint16 by, uint32 dwRGB )
{
    uint16 h,l,err;
    uint32 dwRRGB;
    dwRRGB=Lcd_GetFontColor(byScreen);
    Lcd_SetFontColor(byScreen,dwRGB);   //设置当前颜色设置
    Lcd_Pub_GetCurrentSet(byScreen);

    err = Lcd_Pub_CheckPosi( &lx, CN_LCD_SIZE_X, -2 );
    err = Lcd_Pub_CheckPosi( &rx, CN_LCD_SIZE_X, -2 );
    err = Lcd_Pub_CheckPosi( &ty, CN_LCD_SIZE_Y, -1 );
    err = Lcd_Pub_CheckPosi( &by, CN_LCD_SIZE_Y, -1 );

    for(h=ty+1;h<by;h++)
    {
        for(l=lx+1;l<rx;l++)
            Lcd_Pub_PixelFT(l,h);
    }
    Lcd_SetFontColor(byScreen,CN_RGB_WHITE);   //设置左侧及上侧线颜色设置为白色
    Lcd_LineH( byScreen, lx, rx,ty, 1 );
    Lcd_LineV( byScreen, ty, by,lx, 1 );
    Lcd_SetFontColor(byScreen,CN_RGB_BLACK);   //设置下侧及右侧线颜色设置为黑色
    Lcd_LineH( byScreen, lx, rx,by, 1 );
    Lcd_LineV( byScreen, ty, by,rx, 1 );
    Lcd_SetFontColor(byScreen,dwRRGB);  //恢复当前颜色设置
}

//==============================================================================
//返回：无。
//参数：byScreen，无符号字符，虚拟界面编号。
//lx、ty，无符号短整数，矩形左上角的坐标。
//rx、by，无符号短整数，矩形右下角的坐标。
//dwRGB，按钮颜色。
//功能：使用系统当前设置画一凹陷按钮。x或y坐标超出范围则取液晶许可的最大值。按钮用dwRGB填充。
//==============================================================================
void Lcd_Sunk( uint8 byScreen, uint16 lx, uint16 ty, uint16 rx, uint16 by, uint32 dwRGB )
{
    uint16 h,l,err;
    uint32 dwRRGB;
    dwRRGB=Lcd_GetFontColor(byScreen);
    Lcd_SetFontColor(byScreen,dwRGB);   //设置当前颜色设置
    Lcd_Pub_GetCurrentSet(byScreen);

    err = Lcd_Pub_CheckPosi( &lx, CN_LCD_SIZE_X, -2 );
    err = Lcd_Pub_CheckPosi( &rx, CN_LCD_SIZE_X, -2 );
    err = Lcd_Pub_CheckPosi( &ty, CN_LCD_SIZE_Y, -1 );
    err = Lcd_Pub_CheckPosi( &by, CN_LCD_SIZE_Y, -1 );

    for(h=ty+1;h<by;h++)
    {
        for(l=lx+1;l<rx;l++)
            Lcd_Pub_PixelFT(l,h);
    }
//    dwRGB=Lcd_GetFontColor(byScreen);
    Lcd_SetFontColor(byScreen,CN_RGB_BLACK);   //设置左侧及上侧线颜色设置为黑
    Lcd_LineH( byScreen, lx, rx,ty, 1 );
    Lcd_LineV( byScreen, ty, by,lx, 1 );
    Lcd_SetFontColor(byScreen,CN_RGB_WHITE);   //设置下侧及右侧线颜色设置为白
    Lcd_LineH( byScreen, lx, rx,by, 1 );
    Lcd_LineV( byScreen, ty, by,rx, 1 );

    Lcd_SetFontColor(byScreen,dwRRGB);  //恢复当前颜色设置
}

//==============================================================================
//功能:对指定的矩形框产生突出的按钮效果，不填充按钮颜色
//返回值:无
//参数:byScreen，无符号字符，虚拟界面编号。
//     lx、ty，无符号短整数，矩形左上角的坐标。
//     rx、by，无符号短整数，矩形右下角的坐标。
//==============================================================================
void Lcd_ButtonPull( uint8 byScreen, uint16 lx, uint16 ty, uint16 rx, uint16 by)
{
    uint16    err;
    uint32   dwRGB;

    err = Lcd_Pub_CheckPosi( &lx, CN_LCD_SIZE_X, -2 );
    err = Lcd_Pub_CheckPosi( &rx, CN_LCD_SIZE_X, -2 );
    err = Lcd_Pub_CheckPosi( &ty, CN_LCD_SIZE_Y, -1 );
    err = Lcd_Pub_CheckPosi( &by, CN_LCD_SIZE_Y, -1 );

    dwRGB=Lcd_GetFontColor(byScreen);
    Lcd_SetFontColor(byScreen,CN_RGB_WHITE);   //设置左侧及上侧线颜色设置为黑
    Lcd_LineH( byScreen, lx, rx,ty, 1 );
    Lcd_LineV( byScreen, ty, by,lx, 1 );
    Lcd_SetFontColor(byScreen,CN_RGB_BLACK);   //设置下侧及右侧线颜色设置为白
    Lcd_LineH( byScreen, lx, rx,by, 1 );
    Lcd_LineV( byScreen, ty, by,rx, 1 );

    Lcd_SetFontColor(byScreen,dwRGB);  //恢复当前颜色设置
}


//==============================================================================
//功能:对指定的矩形框产生压下的按钮效果，不填充按钮颜色
//返回值:无
//参数:byScreen，无符号字符，虚拟界面编号。
//     lx、ty，无符号短整数，矩形左上角的坐标。
//     rx、by，无符号短整数，矩形右下角的坐标。
//==============================================================================
void Lcd_ButtonPush( uint8 byScreen, uint16 lx, uint16 ty, uint16 rx, uint16 by)
{
    uint16    err;
    uint32   dwRGB;

    err = Lcd_Pub_CheckPosi( &lx, CN_LCD_SIZE_X, -2 );
    err = Lcd_Pub_CheckPosi( &rx, CN_LCD_SIZE_X, -2 );
    err = Lcd_Pub_CheckPosi( &ty, CN_LCD_SIZE_Y, -1 );
    err = Lcd_Pub_CheckPosi( &by, CN_LCD_SIZE_Y, -1 );

    dwRGB=Lcd_GetFontColor(byScreen);
    Lcd_SetFontColor(byScreen,CN_RGB_BLACK);   //设置左侧及上侧线颜色设置为黑
    Lcd_LineH( byScreen, lx, rx,ty, 1 );
    Lcd_LineV( byScreen, ty, by,lx, 1 );
    Lcd_SetFontColor(byScreen,CN_RGB_WHITE);   //设置下侧及右侧线颜色设置为白
    Lcd_LineH( byScreen, lx, rx,by, 1 );
    Lcd_LineV( byScreen, ty, by,rx, 1 );

    Lcd_SetFontColor(byScreen,dwRGB);  //恢复当前颜色设置
}


//==============================================================================
//功能:清除指定的矩形框的按钮效果,即用填充色重画边框。
//返回值:无
//参数:byScreen，无符号字符，虚拟界面编号。
//     lx、ty，无符号短整数，矩形左上角的坐标。
//     rx、by，无符号短整数，矩形右下角的坐标。
//     dwRGB:按钮填充色
//==============================================================================
void Lcd_ButtonClr( uint8 byScreen, uint16 lx, uint16 ty, uint16 rx, uint16 by,uint32 dwRGB)
{
    uint16    err;
    uint32   dwRRGB;

    err = Lcd_Pub_CheckPosi( &lx, CN_LCD_SIZE_X, -2 );
    err = Lcd_Pub_CheckPosi( &rx, CN_LCD_SIZE_X, -2 );
    err = Lcd_Pub_CheckPosi( &ty, CN_LCD_SIZE_Y, -1 );
    err = Lcd_Pub_CheckPosi( &by, CN_LCD_SIZE_Y, -1 );

    dwRRGB=Lcd_GetFontColor(byScreen);
    Lcd_SetFontColor(byScreen,dwRGB);   //设置左侧及上侧线颜色设置为按钮色
    Lcd_LineH( byScreen, lx, rx,ty, 1 );
    Lcd_LineV( byScreen, ty, by,lx, 1 );
    Lcd_LineH( byScreen, lx, rx,by, 1 );
    Lcd_LineV( byScreen, ty, by,rx, 1 );

    Lcd_SetFontColor(byScreen,dwRRGB);  //恢复当前颜色设置
}




/*
unsigned short *pawbyLCDMemory;      //定义显存
unsigned char LcdDispBuf[261120];//480*272
*/
//==============================================================================
//完成液晶显示的硬件初始化工作，包括设置默认的背景色、前景色等。
void Lcd_InitMemory( void )
{
    uint32 i;

    g_tDspAttrib.style = 0;
    g_tDspAttrib.StringDirect = 0;
    g_tDspAttrib.fillZero = 0;
    g_tDspAttrib.Rotate = 0;
    g_tDspAttrib.rowSpace = 2;

    for( i=0;i<2;i++ )
    {
        g_tVsr[i].wBackColor = Lcd_Pub_RGBtoColor( CN_RGB_WHITE );
        g_tVsr[i].wFontColor = Lcd_Pub_RGBtoColor( CN_RGB_BLACK );
        g_tVsr[i].pwbyLCDMemory = (uint16 *)(0xc1f00020 + i*CN_DISPLAYMEMORYSIZE);          
    }
}

void lcdclear( uint8 byScreen )
{
    uint32 i;
    uint16 *buf = Lcd_Pub_GetScreenAddr(byScreen);
    uint32 color = g_tVsr[Lcd_Pub_GetScreenNo( byScreen)].wBackColor;


    for(i=0; i < CN_DISPLAYMEMORYSIZE; i++)
        buf[i] = color;
}


//=======================================================
// 函数名称：Lcd_Icon
// 函数功能：显示ICON 图形
//
// 输入参数：byLength:  信息框长度
// 输入参数：byWidth :  信息框宽度
// 输入参数：byNameStr : 信息框名
// 输入参数：byNameStr : 信息框提示信息
// 输出参数：g_tDspCtrl.bFirst
//=======================================================
unsigned char Lcd_Icon(unsigned char byScreen, const unsigned char *addrBmp, unsigned int x,unsigned int y,
						unsigned int x_len,unsigned int y_len)
{
    unsigned short (*screen_Addr)[CN_LCD_SIZE_X];
    unsigned int row,col,offset;
	unsigned short *dispdata;

	dispdata=(unsigned short *)addrBmp;
    screen_Addr=(void *)Lcd_Pub_GetScreenAddr(byScreen);

    offset = 0;
    for(row=y_len;row> 0;row--)
    {
        for(col=0;col<x_len;col++)
        {
			 screen_Addr[y+row][x+col]=dispdata[offset++];
        }
    }
    return 0;
}

void dspmenu(void)
{
	Lcd_Clear(1);

	Lcd_Icon(1, g_byIcon_ip,   10, 10, 100, 100 );Lcd_Str( 1, 36,  114, "IP设置" );
	Lcd_Icon(1, g_byIcon_rtc, 130, 10, 100, 100 );Lcd_Str( 1, 148, 114, "时钟设置" );
	Lcd_Icon(1, g_byIcon_led, 250, 10, 100, 100 );Lcd_Str( 1, 264, 114, "LED流水灯");
	Lcd_Icon(1, g_byIcon_phot,370, 10, 100, 100 );Lcd_Str( 1, 388, 114, "图片测试");
	Lcd_Icon(1, g_byIcon_uart, 10, 146,100, 100); Lcd_Str( 1, 44,  250, "help");
	Lcd_Icon(1, g_byIcon_sd,  130, 146,100, 100); Lcd_Str( 1, 155, 250, "SD CARD" );
	Lcd_Icon(1, g_byIcon_usbd,250, 146,100, 100); Lcd_Str( 1, 260, 250, "USB DEVICE" );
	Lcd_Icon(1, g_byIcon_version,370, 146,100, 100); Lcd_Str( 1, 388, 250, "version" );

}

void Dsp_Init(void)
{
	// 初始化显存
	Lcd_InitMemory();

	g_awLibHZ16_16 = (void *)g_byHzk1616;


	Lcd_Clear(1);
	
	Lcd_SetFontColor(1, CN_RGB_BLACK);
	Lcd_SetFont(1,CN_GB16BIT,CN_ASCII168);

}


