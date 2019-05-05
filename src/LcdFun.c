/*========================================================*
 * �ļ�ģ��˵��:
 * lcdFun.c ͨ�ý���ģ��Ĺ��ܺ���
 * �ļ��汾: V1.0.0
 * ������Ա: Rock
 * ����ʱ��: 2013-01-01 
 * Copyright(c) 2013-2015  Rock-Embed Limited Co.
 *========================================================*
 * �����޸ļ�¼��
 * <�汾��> <�޸�����>, <�޸���Ա>: <�޸Ĺ��ܸ���>
 *      V1.0.0           2013-01-01              Rock                        ����
 *========================================================*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "LcdMacro.h"

//==============================================================================
// ���������Ƿ񳬳���Χ                       **** �����������ڲ����� ****
inline int Lcd_Pub_CheckPosi( uint16 *pwPosi, uint16 wSize, int iErrCode )
{
    int iRetCode = 0;

    if( *pwPosi >=wSize )             // ����Խ��
    {
        iRetCode = iErrCode;
        *pwPosi = wSize - 1;
    }

    return iRetCode;
}

//==============================================================================
//����:24λɫתΪ16λɫ,�ڲ�����                      **** �����������ڲ����� ****
//����:uint32 dwRGB,24λ��ɫֵ
//����:16λ��ɫֵ
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
//����:��RGB��ɫֵת��Ϊ����LCD֧�ֵ���ɫֵ
//���أ�ת�������ɫֵ
//������dwRGB��24λ��ɫֵ
uint16 Lcd_Pub_RGBtoColor( uint32 dwRGB )
{
    return( Lcd_Pub_RGBto16B(dwRGB) );

}

//==============================================================================
//����:��16λ��8λ��ɫֵת��Ϊ24λ��ɫֵ---�ڲ�����
//���أ�ת�������ɫֵ
//������wColor��24λ��ɫֵ
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
//����:���������ȡ����Ӧ�������,������������±�  **** �����������ڲ����� ****
//����:uint8 byScreen,���������
//����:�����������
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
//����:����ȫ��ָ��g_tCVsrָ��byScreen���������    **** �����������ڲ����� ****
//������byScreen���޷����ַ�����������š�
//����:��
//==============================================================================
void Lcd_Pub_GetCurrentSet( uint8 byScreen )
{
    uint16 wSno;

    wSno=Lcd_Pub_GetScreenNo( byScreen );
    g_tCVsr=&g_tVsr[wSno];
}


//==============================================================================
//����:������������ȡ����Ӧ���Դ��ַ.            **** �����������ڲ����� ****
//����:uint8 byScreen,���������
//����:�Դ��ַ
//==============================================================================
void *Lcd_Pub_GetScreenAddr( uint8 byScreen )
{
    return( g_tVsr[Lcd_Pub_GetScreenNo( byScreen)].pwbyLCDMemory );
}

//==============================================================================
//���ܣ�ʹ��ϵͳȱʡ����(8*16����)�͵�ǰϵͳ������ʾһ��Ӣ���ַ���  ****�ڲ�ʹ��
//���أ���ȷ��ʾʱ����0��x���곬����Χ����-2��y���곬����Χ����-1��
//������byScreen���޷����ַ�����������š�
//x��y���޷��Ŷ���������ʾ�ַ�������λ�á�
//wAttrib����ʾ��������((�������ʾ��������˵��))��
//byAscii �޷����ַ�����Ҫ��ʾ�ַ���ASCII���롣
//������ʾʱ��X:��-->��,Y:��-->��,����Ϊ���Ͻ�����
//������ʾʱ, X:��-->��,Y:��-->��,����Ϊ���½�����.
//==============================================================================
inline uint16 Lcd_Pub_Ascii( uint8 byScreen, uint16 x, uint16 y,  uint8 byAscii, bool_t bTR )
{
    uint8 i,j;
    uint8    byLine, *pbyFontLib;

    Lcd_Pub_GetCurrentSet( byScreen );
    pbyFontLib = &g_tCVsr->pbyLibAscii[ byAscii*g_tCVsr->wASCIIFontHeight];     // ��ģ�׵�ַ

    if( 0 ==g_tDspAttrib.style )
    {// ������ʾ
        if( 0 ==g_tDspAttrib.Rotate )
        {// ������ʾ
			if( (x+g_tCVsr->wASCIIFontWidth ) >CN_LCD_SIZE_X ) return(-2);  // ��Խ��
            if( (y+g_tCVsr->wASCIIFontHeight) >CN_LCD_SIZE_Y ) return(-1);  // ��Խ��

            for( j=0; j<g_tCVsr->wASCIIFontHeight; j++ )
            {
                byLine = pbyFontLib[j];                         // ȡ��ģ�ĵ�ǰ��
                for( i=0; i<g_tCVsr->wASCIIFontWidth; i++ )
                {
                    if( (byLine<<i) & 0x80 ) Lcd_Pub_PixelFT( x+i, y+j );
                    else if( false ==bTR   ) Lcd_Pub_PixelBK( x+i, y+j );   // ��͸������ʾ����
                }
            } // end for( j )
        }
		else
        {// ������ʾ�����ַ���ʱ����ת90��
			if( (x+g_tCVsr->wASCIIFontHeight) >CN_LCD_SIZE_X )  return(-2); // ��Խ��
            if( (y+1) < g_tCVsr->wASCIIFontWidth )              return(-1); // ��Խ��

            for (j=0; j<g_tCVsr->wASCIIFontHeight; j++ )
            {
                byLine = pbyFontLib[j];                         // ȡ��ģ�ĵ�ǰ��
                for( i=0;i <g_tCVsr->wASCIIFontWidth; i++ )
                {
                    if( (byLine<<i) & 0x80 ) Lcd_Pub_PixelFT( x+j, y-i );
                    else if( false ==bTR   ) Lcd_Pub_PixelBK( x+j, y-i );   // ��͸������ʾ����
                }
            } // end for( j )
        }// end if( Rotate ) ... else ...
    }
	else
    { // ͻ����ʾ�����ַ��뱳����ɫ�Ե���
        if( 0 ==g_tDspAttrib.Rotate )
        {// ������ʾ
            if( (x+g_tCVsr->wASCIIFontWidth ) >CN_LCD_SIZE_X ) return(-2);  // ��Խ��
            if( (y+g_tCVsr->wASCIIFontHeight) >CN_LCD_SIZE_Y ) return(-1);  // ��Խ��

            for( j=0; j<g_tCVsr->wASCIIFontHeight; j++ )
            {
                byLine = pbyFontLib[j];                         // ȡ��ģ�ĵ�ǰ��
                for( i=0; i<g_tCVsr->wASCIIFontWidth; i++ )
                {
                    if( (byLine<<i) & 0x80 ) Lcd_Pub_PixelBK( x+i, y+j );
                    else if( false ==bTR   ) Lcd_Pub_PixelFT( x+i, y+j );   // ��͸������ʾ����
                }
            } // end for( j )
        }else
        {// ������ʾ�����ַ���ʱ����ת90��
            if( (x+g_tCVsr->wASCIIFontHeight) >CN_LCD_SIZE_X )  return(-2); // ��Խ��
            if( (y+1) < g_tCVsr->wASCIIFontWidth )              return(-1); // ��Խ��

            for (j=0; j<g_tCVsr->wASCIIFontHeight; j++ )
            {
                byLine = pbyFontLib[j];                         // ȡ��ģ�ĵ�ǰ��
                for( i=0;i <g_tCVsr->wASCIIFontWidth; i++ )
                {
                    if( (byLine<<i) & 0x80 ) Lcd_Pub_PixelBK( x+j, y-i );
                    else if( false ==bTR   ) Lcd_Pub_PixelFT( x+j, y-i );   // ��͸������ʾ����
                }
            } // end for( j )
        }// end if( Rotate ) ... else ...
    }// end if( style ) ... else ...

    return( 0 );
}

//==============================================================================
//���ܣ�ʹ��ϵͳȱʡ����͵�ǰϵͳ������ʾһ�����֡�
//ע�⣺���������ṩ���������ڲ����ã�Ϊ���Ч�ʣ����ж������Ƿ�Խ�硣
//������byScreen: �޷����ַ�����������š�
//      x��y:     �޷��Ŷ���������ʾ�ַ�������λ�á�
//                ������ʾʱ��X:��-->��,Y:��-->��,����Ϊ���Ͻ�����
//                ������ʾʱ, X:��-->��,Y:��-->��,����Ϊ���½�����.
//      wAttrib:  ��ʾ��������(�������ʾ��������˵��)��
//      byAscii:  ��Ҫ��ʾ�ַ���ASCII���롣
//���أ���
//==============================================================================
void Lcd_Pub_GB2312( uint8 byScreen, uint16 x, uint16 y,  uint16 wCharGB , bool_t bTR )
{
    uint8    i,j;
    uint16    offset, wLine, *pwFontLib;

    Lcd_Pub_GetCurrentSet( byScreen );
    offset    = ( (wCharGB>>8)-0xA1 ) * 94 + (wCharGB&0xFF) -0xA1;
    pwFontLib = &g_tCVsr->pwLibHZ[ offset * g_tCVsr->wGBFontHeight];    // ��ģ�׵�ַ

    if( 0 ==g_tDspAttrib.style )
    {// ������ʾ
        if( 0 ==g_tDspAttrib.Rotate )
        {// ������ʾ
            for( j=0; j<g_tCVsr->wGBFontHeight; j++ )
            {
                wLine = pwFontLib[j];                       // ȡ��ģ�ĵ�ǰ��
                wLine = (wLine<<8) + (wLine>>8);

                for( i=0;i <g_tCVsr->wGBFontWidth; i++ )
                {
                    if( (wLine<<i) & 0x8000 ) Lcd_Pub_PixelFT( x+i, y+j );
                    else if( false ==bTR    ) Lcd_Pub_PixelBK( x+i, y+j );  // ��͸������ʾ����

                }
            } // end for( j )
        }else
        {// ������ʾ�����ַ���ʱ����ת90��
            for( j=0; j<g_tCVsr->wGBFontHeight; j++ )
            {
//                wLine = pwFontLib[j];                       // ȡ��ģ�ĵ�ǰ��
                wLine = pwFontLib[j];                       // ȡ��ģ�ĵ�ǰ��
                wLine = (wLine<<8) + (wLine>>8);
                for( i=0;i <g_tCVsr->wGBFontWidth; i++ )
                {
                    if( (wLine<<i) & 0x8000 ) Lcd_Pub_PixelFT( x+j, y-i );
                    else if( false ==bTR    ) Lcd_Pub_PixelBK( x+j, y-i );  // ��͸������ʾ����
                }
            } // end for( j )
        }// end if( Rotate ) ... else ...
    }else
    { // ͻ����ʾ�����ַ��뱳����ɫ�Ե���
        if( 0 ==g_tDspAttrib.Rotate )
        {// ������ʾ
            for( j=0; j<g_tCVsr->wGBFontHeight; j++ )
            {
//                wLine = pwFontLib[j];                       // ȡ��ģ�ĵ�ǰ��
                wLine = pwFontLib[j];                       // ȡ��ģ�ĵ�ǰ��
                wLine = (wLine<<8) + (wLine>>8);
                for( i=0;i <g_tCVsr->wGBFontWidth; i++ )
                {
                    if( (wLine<<i) & 0x8000 ) Lcd_Pub_PixelBK( x+i, y+j );
                    else if( false ==bTR    ) Lcd_Pub_PixelFT( x+i, y+j );  // ��͸������ʾ����
                }
            } // end for( j )
        }else
        {// ������ʾ�����ַ���ʱ����ת90��
            for( j=0; j<g_tCVsr->wGBFontHeight; j++ )
            {
//                wLine = pwFontLib[j];                       // ȡ��ģ�ĵ�ǰ��
                wLine = pwFontLib[j];                       // ȡ��ģ�ĵ�ǰ��
                wLine = (wLine<<8) + (wLine>>8);
                for( i=0;i <g_tCVsr->wGBFontWidth; i++ )
                {
                    if( (wLine<<i) & 0x8000 ) Lcd_Pub_PixelBK( x+j, y-i );
                    else if( false ==bTR    ) Lcd_Pub_PixelFT( x+j, y-i );  // ��͸������ʾ����
                }
             } // end for( j )
        }// end if( Rotate ) ... else ...
    }// end if( style ) ... else ...

    return;
}



//==============================================================================
//���أ��ޡ�
//������byScreen����������š�
//dwRGB���޷��ų�����������ɫ��RGBֵ��
//����ϵͳ��ǰ��ʾ���õı���ɫ��
//==============================================================================
void Lcd_SetBackColor ( uint8 byScreen, uint32 dwRGB )
{
    g_tVsr[Lcd_Pub_GetScreenNo( byScreen)].wBackColor=Lcd_Pub_RGBtoColor( dwRGB);
}


//==============================================================================
//��ȡָ������ĵ�ǰ�ַ�ɫ��
//������byScreen����������š�
//���أ�24λ��ɫֵ��
uint32 Lcd_GetFontColor ( uint8 byScreen)
{
    return( Lcd_Pub_ColortoRGB(g_tVsr[Lcd_Pub_GetScreenNo( byScreen)].wFontColor));
}


//==============================================================================
//���ܣ�����ϵͳ��ǰ��ʾ���õ��ַ���ɫ��
//������byScreen����������š�
//      dwRGB���޷��ų��������ַ���ɫ��RGBֵ��
//���أ��ޡ�
void Lcd_SetFontColor ( uint8 byScreen, uint32 dwRGB )
{
    g_tVsr[Lcd_Pub_GetScreenNo( byScreen)].wFontColor=Lcd_Pub_RGBtoColor( dwRGB);
}


//==============================================================================
//���ܣ����ԭ�����ݲ���ȱʡ����ɫ���������Ļ��
//������byScreen���޷����ַ�����������š�
//���أ��ޡ�
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
//���أ���ȷ��ʾʱ����0��x���곬����Χ����-2��y���곬����Χ����-1��
//������byScreen���޷����ַ�����������š�
//xStart��yStart���޷��Ŷ�������ֱ�ߵ���ʼ���ꡣ
//xEnd���޷��Ŷ�������ֱ�ߵ�ĩ�����ꡣ
//wWidth���޷��Ŷ�������ֱ�ߵĿ�ȡ�
//���ܣ�ʹ��ϵͳ��ǰ���û�һˮƽ�ߡ�
//==============================================================================
int Lcd_LineH( uint8 byScreen, uint16 wXStart, uint16 wXEnd, uint16 wYStart, uint16 wWidth )
{
    uint16    wLoopX, wLoopY, wYEnd;
    int     iRetCode;

    wYEnd = wYStart + wWidth;           // ���
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
//���أ���ȷ��ʾʱ����0��x���곬����Χ����-2��y���곬����Χ����-1��
//������byScreen���޷����ַ�����������š�
//      xStart��yStart���޷��Ŷ�������ֱ�ߵ���ʼ���ꡣ
//      xEnd���޷��Ŷ�������ֱ�ߵ�ĩ�����ꡣ
//      wWidth���޷��Ŷ�������ֱ�ߵĿ�ȡ�
//���ܣ�ʹ��ϵͳ��ǰ���û�һ��ֱ�ߡ�
//==============================================================================
int Lcd_LineV( uint8 byScreen, uint16 wYStart, uint16 wYEnd, uint16 wXStart, uint16 wWidth )
{
    uint16    wLoopX, wLoopY, wXEnd;
    int     iRetCode;

    wXEnd = wXStart + wWidth;           // ���
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
//���أ���ȷ��ʾʱ����0��x���곬����Χ����-2��y���곬����Χ����-1��
//������byScreen���޷����ַ�����������š�
//xStart��yStart���޷��Ŷ�������ֱ�ߵ���ʼ���ꡣ
//xEnd��yEnd���޷��Ŷ�������ֱ�ߵ�ĩ�����ꡣ
//wWidth���޷��Ŷ�������ֱ�ߵĿ�ȡ�
//���ܣ�ʹ��ϵͳ��ǰ���û�һֱ�ߡ�
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
//����:����ϵͳʹ�õ�Ĭ������,�����ַ����ַ�����ʾ�������������ý�����ʾ
//����:��ȷ����,����1,��������,����-1;
//����:byScreen �޷����ַ������������
//     wGB,��������,��ѡ����ΪCN_GB12Bit,CN_GB14Bit,CN_GB16Bit
//     wASCII,�ַ�����,��ѡ����Ϊ CN_ASCII88,CN_ASCII126,CN_ASCII148,CN_ASCII168
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
//���أ���ȷ��ʾʱ����0��x���곬����Χ����-2��y���곬����Χ����-1��
//������byScreen���޷����ַ�����������š�
//x��y���޷��Ŷ���������ʾ�ַ�������λ�á�
//wAttrib����ʾ��������((�������ʾ��������˵��))��
//byAscii �޷����ַ�����Ҫ��ʾ�ַ���ASCII���롣
//���ܣ�ʹ��ϵͳȱʡ����(8*16����)�͵�ǰϵͳ������ʾһ��Ӣ���ַ���
//������ʾʱ��X:��-->��,Y:��-->��,����Ϊ���Ͻ�����
//������ʾʱ, X:��-->��,Y:��-->��,����Ϊ���½�����.
//==============================================================================
uint16 Lcd_Ascii( uint8 byScreen, uint16 x, uint16 y,  uint8 byAscii )
{
	return Lcd_Pub_Ascii( byScreen, x, y, byAscii, false );
}

//==============================================================================
//���أ���ȷ��ʾʱ����0��x���곬����Χ����-2��y���곬����Χ����-1��
//������byScreen���޷����ַ�����������š�
//x��y���޷��Ŷ���������ʾ�ַ�������λ�á�
//wAttrib����ʾ��������((�������ʾ��������˵��))��
//byAscii �޷����ַ�����Ҫ��ʾ�ַ���ASCII���롣
//���ܣ�ʹ��ϵͳȱʡ����(8*16����)�͵�ǰϵͳ������ʾһ��Ӣ���ַ���͸����ʾ
//������ʾʱ��X:��-->��,Y:��-->��,����Ϊ���Ͻ�����
//������ʾʱ, X:��-->��,Y:��-->��,����Ϊ���½�����.
//==============================================================================
uint16 Lcd_AsciiTr( uint8 byScreen, uint16 x, uint16 y, uint8 byAscii )
{
    return Lcd_Pub_Ascii( byScreen, x, y, byAscii, true );
}

//==============================================================================
//���أ���
//������byScreen���޷����ַ�����������š�
//x��y���޷��Ŷ���������ʾ�ַ�������λ�á�
//wAttrib����ʾ��������(�������ʾ��������˵��)��
//byAscii����Ҫ��ʾ�ַ���ASCII���롣
//���ܣ�ʹ��ϵͳȱʡ����͵�ǰϵͳ������ʾһ�����֡�
//ע�⣺���������ṩ���������ڲ����ã�Ϊ���Ч�ʣ����ж������Ƿ�Խ�硣
//������ʾʱ��X:��-->��,Y:��-->��,����Ϊ���Ͻ�����
//������ʾʱ, X:��-->��,Y:��-->��,����Ϊ���½�����.
//==============================================================================
void Lcd_GB2312( uint8 byScreen, uint16 x, uint16 y,  uint16 wCharGB )
{
    Lcd_Pub_GB2312( byScreen, x, y, wCharGB, false );
    return;
}


//==============================================================================
//���أ���
//������byScreen���޷����ַ�����������š�
//x��y���޷��Ŷ���������ʾ�ַ�������λ�á�
//wAttrib����ʾ��������(�������ʾ��������˵��)��
//byAscii����Ҫ��ʾ�ַ���ASCII���롣
//���ܣ�ʹ��ϵͳȱʡ����͵�ǰϵͳ������ʾһ�����֡�͸����ʾ
//ע�⣺���������ṩ���������ڲ����ã�Ϊ���Ч�ʣ����ж������Ƿ�Խ�硣
//������ʾʱ��X:��-->��,Y:��-->��,����Ϊ���Ͻ�����
//������ʾʱ, X:��-->��,Y:��-->��,����Ϊ���½�����.
//==============================================================================
void Lcd_GB2312Tr( uint8 byScreen, uint16 x, uint16 y,  uint16 GB )
{
    Lcd_Pub_GB2312( byScreen, x, y, GB, true );

}

//==============================================================================
//���أ���ȷ��ʾʱ����0��x���곬����Χ����-2��y���곬����Χ����-1��
//������byScreen���޷����ַ�����������š�
//x��y���޷��Ŷ���������ʾ�ַ�������λ�á�
//wAttrib����ʾ��������(�������ʾ��������˵��)��
//pcString���ַ���ָ�룬��NULL������
//���ܣ�ʹ��ϵͳȱʡ����͵�ǰϵͳ������ʾ�ַ��������������ַ������Զ�ʶ����Ӣ�Ļ��������ַ���
//����ַ����м�������з���(ASCII��ֵΪ0x0A)������л�����ʾ����һ�еĺ�����Ϊx��
//����������Ϊ��ǰ�е������ꡢ
//ȱʡ�������߶�(��Ϊ8*16������Ϊ16)�Լ��о����ߵĺ͡�
//������ʾʱ��X:��-->��,Y:��-->��,����Ϊ���Ͻ�����
//������ʾʱ, X:��-->��,Y:��-->��,����Ϊ���½�����.
//==============================================================================
uint16 Lcd_Str( uint8 byScreen, uint16 x, uint16 y, uint8 *pcString )
{
    uint16 charindex,i,j,err,bakx,baky,LineOver;

    bakx=x;
    baky=y;
    Lcd_Pub_GetCurrentSet(byScreen);

    charindex = 0;
    if((g_tDspAttrib.StringDirect==0)&&(g_tDspAttrib.Rotate==0))
    {// �ַ�����ת���ַ�������,����Ϊ�ַ������Ͻ�����
        if( (y+g_tCVsr->wGBFontWidth) >CN_LCD_SIZE_Y ) return(-1);

        while( pcString[charindex]!=0x0 )
        {
            // ������Խ���򷵻ش�����Ϣ
            if( (y+g_tCVsr->wGBFontHeight) >CN_LCD_SIZE_Y ) return(-2);

            if( pcString[charindex]>0x7f )
            {// ��ʾ����
                if( (x+g_tCVsr->wGBFontWidth) <= CN_LCD_SIZE_X )
                {
                    Lcd_GB2312( byScreen,x,y,
                                (uint16)((pcString[charindex]<<8)+pcString[charindex+1]) );
                    charindex += 2;
                    x += g_tCVsr->wGBFontWidth;
                }
				else
                {// �����겻��һ����
                    if(g_tDspAttrib.style==0)
                    {
                        for( j=0;j<g_tCVsr->wGBFontHeight;j++ )
                        {//��������ʣ�ಿ��
                            for(i=x;i<CN_LCD_SIZE_X;i++)
                            {
                                Lcd_Pub_PixelBK(i,y+j);
                            }
                        }
                    }
					else
                    {
                        for (j=0;j<g_tCVsr->wGBFontHeight;j++)
                        {//��������ʣ�ಿ��
                            for(i=x;i<CN_LCD_SIZE_X;i++)
                            {
                                Lcd_Pub_PixelFT(i,y+j);
                            }
                        }
                    }
                    err=-1;

                    for( LineOver=0; LineOver<100; LineOver++ )
                    {//�������з�
                        charindex++;
                        if( pcString[charindex]==0x0a )
                        {//�ҵ����з������С�
                            x = bakx;
                            y += g_tCVsr->wGBFontHeight+g_tDspAttrib.rowSpace;
                            charindex++;
                            break;
                        }
                        if(pcString[charindex]==0x0) return(err);
                    }
                    //LineOver>=100����û�����������з������ش���
                    if( LineOver>=100 )  return(err);
                }
            }
			else
            { // ��ʾ�ַ�
                if(pcString[charindex]==0x0a)
                {
                    x=bakx;
                    y+=g_tCVsr->wGBFontHeight+g_tDspAttrib.rowSpace;
                    charindex++;
                }
                else
                {
                    if(pcString[charindex]<0x10)
                    {//С��0x10��������
                        charindex++;
                        continue;
                    }
                    if(x>CN_LCD_SIZE_X-g_tCVsr->wASCIIFontWidth)
                    /*�����겻��һ���ַ�*/
                    {
                        if(g_tDspAttrib.style==0)
                        {
                            for (j=0;j<g_tCVsr->wGBFontHeight;j++)
                            {//��������ʣ�ಿ��
                                for(i=x;i<CN_LCD_SIZE_X;i++)
                                {
                                    Lcd_Pub_PixelBK(i,y+j);
                                }
                            }
                        }else
                        {
                            for (j=0;j<g_tCVsr->wGBFontHeight;j++)
                            {//��������ʣ�ಿ��
                                for(i=x;i<CN_LCD_SIZE_X;i++)
                                {
                                    Lcd_Pub_PixelFT(i,y+j);
                                }
                            }
                        }
                        err=-1;
                        for(LineOver=0;LineOver<100;LineOver++)
                        {//�������з�
                            charindex++;
                            if(pcString[charindex]==0x0a)
                            {//�ҵ����з������С�
                                x=bakx;
                                y+=g_tCVsr->wGBFontHeight+g_tDspAttrib.rowSpace;
                                charindex++;
                                break;
                            }
                            if(pcString[charindex]==0x0)
                                return(err);
                        }
                        //LineOver>=100����û�����������з������ش���
                        if( LineOver>=100 )   return( err );
                    }
					else
                    {
						if(g_tDspAttrib.style==0)
                        {
                            for (j=0;j<(g_tCVsr->wGBFontHeight-g_tCVsr->wASCIIFontHeight);j++)
                            {//�¶��룬�������ָ߳��ַ��Ĳ���
                                for(i=0;i<g_tCVsr->wASCIIFontWidth;i++)
                                {
                                    Lcd_Pub_PixelBK(x+i,y+j);
                                }
                            }
                        }
                        else
                        {// ����
                            for (j=0;j<(g_tCVsr->wGBFontHeight-g_tCVsr->wASCIIFontHeight);j++)
                            {//�¶��룬�������ָ߳��ַ��Ĳ���
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
    {   //�ַ���ת���ַ�������,����Ϊ�ַ������Ͻ�����
        if(x>CN_LCD_SIZE_X-g_tCVsr->wGBFontHeight)return(-1);   //��ʼX����Խ�磬ֱ�ӷ���
        while(pcString[charindex]!=0x0)
        {
            /*������Խ���򷵻ش�����Ϣ*/
            if(pcString[charindex]>0x7f)
            {/*��ʾ����*/
                if( (y+g_tCVsr->wGBFontWidth)>=CN_LCD_SIZE_Y ) return(-1);
                if( (x+g_tCVsr->wGBFontWidth)> CN_LCD_SIZE_X )
                { // �����������ʾһ����
                    Lcd_GB2312( byScreen, x,(uint16)(y+g_tCVsr->wGBFontWidth-1),
                            (uint16)(pcString[charindex]*256+pcString[charindex+1]) );
                    charindex+=2;x+=g_tCVsr->wGBFontWidth;
                }else
                {  /*�����겻��һ����*/
                    if(g_tDspAttrib.style==0)
                        for (j=0;j<g_tCVsr->wGBFontWidth;j++)
                        {//��������ʣ�ಿ��
                            for(i=x;i<CN_LCD_SIZE_X;i++)
                            {
                                Lcd_Pub_PixelBK(i,y+j);
                            }
                        }
                    else
                        for (j=0;j<g_tCVsr->wGBFontWidth;j++)
                        {//��������ʣ�ಿ��
                            for(i=x;i<CN_LCD_SIZE_X;i++)
                            {
                                Lcd_Pub_PixelFT(i,y+j);
                            }
                        }
                    err=-1;
                    for(LineOver=0;LineOver<100;LineOver++)
                    {//�������з�
                        charindex++;
                        if(pcString[charindex]==0x0a)
                        {//�ҵ����з������С�
                            x=bakx;
                            y+=g_tCVsr->wGBFontWidth+g_tDspAttrib.rowSpace;
                            charindex++;
                            break;
                        }
                        if(pcString[charindex]==0x0)
                            return(err);
                    }
                    //LineOver>=100����û�����������з������ش���
                    if( LineOver>=100 )   return( err );
                }
            }
            else
            { /*��ʾ�ַ�*/
                if(pcString[charindex]==0x0a)
               {
                    x=bakx;
                    y+=g_tCVsr->wGBFontWidth+g_tDspAttrib.rowSpace;
                    charindex++;

                }else if(pcString[charindex]<0x10)
                {//С��0x10��������
                    charindex++;
                    continue;

                }else if( (x+g_tCVsr->wGBFontWidth) < CN_LCD_SIZE_X )
                {// �����������ʾһ���ַ�
                    if(g_tDspAttrib.style==0)
                        for (j=0;j<(g_tCVsr->wGBFontWidth-g_tCVsr->wASCIIFontWidth);j++)
                        {//�¶��룬�������ָ߳��ַ��Ĳ���
                            for(i=0;i<g_tCVsr->wASCIIFontHeight;i++)
                            {
                                Lcd_Pub_PixelBK(x+i,y+j);
                            }
                        }
                    else
                        for (j=0;j<(g_tCVsr->wGBFontWidth-g_tCVsr->wASCIIFontWidth);j++)
                        {//�¶��룬�������ָ߳��ַ��Ĳ���
                            for(i=0;i<g_tCVsr->wASCIIFontHeight;i++)
                            {
                                Lcd_Pub_PixelFT(x+i,y+j);
                            }
                        }
                    Lcd_Ascii(byScreen, x,(uint16)(y+g_tCVsr->wGBFontWidth-1), pcString[charindex]);
                    charindex++;x+=g_tCVsr->wASCIIFontHeight;

                }else
                { // �����겻��һ���ַ�
                    if(g_tDspAttrib.style==0)
                        for (j=0;j<g_tCVsr->wGBFontWidth;j++)
                        {//��������ʣ�ಿ��
                            for(i=x;i<CN_LCD_SIZE_X;i++)
                            {
                                Lcd_Pub_PixelBK(i,y+j);
                            }
                        }
                    else
                        for (j=0;j<g_tCVsr->wGBFontWidth;j++)
                        {//��������ʣ�ಿ��
                            for(i=x;i<CN_LCD_SIZE_X;i++)
                            {
                                Lcd_Pub_PixelFT(i,y+j);
                            }
                        }

                    err=-1;
                    for(LineOver=0;LineOver<100;LineOver++)
                    {//�������з�
                        charindex++;
                        if(pcString[charindex]==0x0a)
                        {//�ҵ����з������С�
                            x=bakx;
                            y+=g_tCVsr->wGBFontWidth+g_tDspAttrib.rowSpace;
                            charindex++;
                            break;
                        }
                        if(pcString[charindex]==0x0)    return(err);
                    }
                    //LineOver>=100����û�����������з������ش���
                    if( LineOver>=100 )   return( err );
                }
            } // end if( >0x7F ) else( .. )
        } // end while( )
    }
	else if((g_tDspAttrib.StringDirect==1)&&(g_tDspAttrib.Rotate==0))
    {   //�ַ�����ת���ַ��������������£�,����Ϊ�ַ������Ͻ�����
        if(y>CN_LCD_SIZE_Y-g_tCVsr->wGBFontHeight)return(-2);   //��ʼY����Խ�磬ֱ�ӷ���
        while(pcString[charindex]!=0x0)
        {
            /*������Խ���򷵻ش�����Ϣ*/
//          if(x<g_tCVsr->wGBFontWidth-1) return(-2);
            if(pcString[charindex]>0x7f)
            {  /*��ʾ����*/
                if(y>CN_LCD_SIZE_Y-g_tCVsr->wGBFontHeight)
                /*�����겻��һ����*/
                {
                    if(g_tDspAttrib.style==0)
                        for (j=0;j<g_tCVsr->wGBFontWidth;j++)
                        {//��������ʣ�ಿ��
                            for(i=y;i<CN_LCD_SIZE_Y;i++)
                            {
                                Lcd_Pub_PixelBK(x-j,i);
                            }
                        }
                    else
                        for (j=0;j<g_tCVsr->wGBFontWidth;j++)
                        {//��������ʣ�ಿ��
                            for(i=y;i<CN_LCD_SIZE_Y;i++)
                            {
                                Lcd_Pub_PixelFT(x-j,i);
                            }
                        }
                    err=-1;
                    for(LineOver=0;LineOver<100;LineOver++)
                    {//�������з�
                        charindex++;
                        if(pcString[charindex]==0x0a)
                        {//�ҵ����з������С�
                            y=baky;
                            x-=g_tCVsr->wGBFontWidth+g_tDspAttrib.rowSpace;
                            charindex++;
                            break;
                        }
                        if(pcString[charindex]==0x0)
                            return(err);
                    }
                    //LineOver>=100����û�����������з������ش���
                    if( LineOver>=100 )   return(err);
                }else
                {
                    Lcd_GB2312(byScreen, (uint16)(x-g_tCVsr->wGBFontWidth+1),y,
                            (uint16)(pcString[charindex]*256+pcString[charindex+1]) );
                    charindex+=2;y+=g_tCVsr->wGBFontHeight;
                }
            }
            else
            /*��ʾ�ַ�*/
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
                    {//С��0x10��������
                        charindex++;
                        continue;
                    }
                    if(y>CN_LCD_SIZE_Y-g_tCVsr->wASCIIFontHeight)
                    /*�����겻��һ���ַ�*/
                    {
                        if(g_tDspAttrib.style==0)
                            for (j=0;j<g_tCVsr->wGBFontWidth;j++)
                            {//��������ʣ�ಿ��
                                for(i=y;i<CN_LCD_SIZE_Y;i++)
                                {
                                    Lcd_Pub_PixelBK(x-j,i);
                                }
                            }
                        else
                            for (j=0;j<g_tCVsr->wGBFontWidth;j++)
                            {//��������ʣ�ಿ��
                                for(i=y;i<CN_LCD_SIZE_Y;i++)
                                {
                                    Lcd_Pub_PixelFT(x-j,i);
                                }
                            }
                        err=-1;
                        for(LineOver=0;LineOver<100;LineOver++)
                        {//�������з�
                            charindex++;
                            if(pcString[charindex]==0x0a)
                            {//�ҵ����з������С�
                                y=baky;
                                x-=g_tCVsr->wGBFontWidth+g_tDspAttrib.rowSpace;
                                charindex++;
                                break;
                            }
                            if(pcString[charindex]==0x0)
                                return(err);
                        }
                        //LineOver>=100����û�����������з������ش���
                        if( LineOver>=100 )   return( err );
                    }else
                    {
                        if(g_tDspAttrib.style==0)
                            for (j=0;j<g_tCVsr->wASCIIFontHeight;j++)
                            {//�¶��룬�������ָ߳��ַ��Ĳ���
                                for(i=0;i<(g_tCVsr->wGBFontWidth-g_tCVsr->wASCIIFontWidth);i++)
                                {
                                    Lcd_Pub_PixelBK(x-i,y+j);
                                }
                            }
                        else
                            for (j=0;j<g_tCVsr->wASCIIFontHeight;j++)
                            {//�¶��룬�������ָ߳��ַ��Ĳ���
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
    {   //�ַ���ת���ַ��������������ϣ�,����Ϊ�ַ������½�����
        if(y<g_tCVsr->wASCIIFontWidth-1)return(-2);   //��ʼY����Խ�磬ֱ�ӷ���
        while(pcString[charindex]!=0x0)
        {
            /*������Խ���򷵻ش�����Ϣ*/
            if(x>CN_LCD_SIZE_X-g_tCVsr->wGBFontHeight) return(-2);
            if(pcString[charindex]>0x7f)
            /*��ʾ����*/
            {
                if(y<g_tCVsr->wGBFontWidth-1)
                /*�����겻��һ����*/
                {
                    if(g_tDspAttrib.style==0)
                        for (j=0;j<=y;j++)
                        {//��������ʣ�ಿ��
                            for(i=0;i<g_tCVsr->wGBFontHeight;i++)
                            {
                                Lcd_Pub_PixelBK(x+i,j);
                            }
                        }
                    else
                        for (j=0;j<=y;j++)
                        {//��������ʣ�ಿ��
                            for(i=0;i<g_tCVsr->wGBFontHeight;i++)
                            {
                                Lcd_Pub_PixelFT(x+i,j);
                            }
                        }
                    err=-1;
                    for(LineOver=0;LineOver<100;LineOver++)
                    {//�������з�
                        charindex++;
                        if(pcString[charindex]==0x0a)
                        {//�ҵ����з������С�
                            y=baky;
                            x+=g_tCVsr->wGBFontHeight+g_tDspAttrib.rowSpace;
                            charindex++;
                            break;
                        }
                        if(pcString[charindex]==0x0)
                            return(err);
                    }
                    //LineOver>=100����û�����������з������ش���
                    if( LineOver>=100 )   return( err );

                }else
                {
                    Lcd_GB2312(byScreen,x,y, (uint16)(pcString[charindex]*256+pcString[charindex+1]) );
                    charindex+=2;y-=g_tCVsr->wGBFontWidth;
                }
            }
            else
            /*��ʾ�ַ�*/
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
                    {//С��0x10��������
                        charindex++;
                        continue;
                    }
                    if(y<g_tCVsr->wASCIIFontWidth-1)
                    /*�����겻��һ���ַ�*/
                    {
                        if(g_tDspAttrib.style==0)
                            for (j=0;j<=y;j++)
                            {//��������ʣ�ಿ��
                                for(i=0;i<g_tCVsr->wGBFontHeight;i++)
                                {
                                    Lcd_Pub_PixelBK(x+i,j);
                                }
                            }
                        else
                            for (j=0;j<=y;j++)
                            {//��������ʣ�ಿ��
                                for(i=0;i<g_tCVsr->wGBFontHeight;i++)
                                {
                                    Lcd_Pub_PixelFT(x+i,j);
                                }
                            }
                        err=-1;
                        for(LineOver=0;LineOver<100;LineOver++)
                        {//�������з�
                            charindex++;
                            if(pcString[charindex]==0x0a)
                            {//�ҵ����з������С�
                                y=baky;
                                x+=g_tCVsr->wGBFontWidth+g_tDspAttrib.rowSpace;
                                charindex++;
                                break;
                            }
                            if(pcString[charindex]==0x0)
                                return(err);
                        }
                        //LineOver>=100����û�����������з������ش���
                        if( LineOver>=100 )   return( err );
                    }else
                    {
                        if(g_tDspAttrib.style==0)
                            for (j=0;j<g_tCVsr->wASCIIFontWidth;j++)
                            {//�¶��룬�������ָ߳��ַ��Ĳ���
                                for(i=0;i<(g_tCVsr->wGBFontHeight-g_tCVsr->wASCIIFontHeight);i++)
                                {
                                    Lcd_Pub_PixelBK(x+i,y-j);
                                }
                            }
                        else
                            for (j=0;j<g_tCVsr->wASCIIFontWidth;j++)
                            {//�¶��룬�������ָ߳��ַ��Ĳ���
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
//���أ���ȷ��ʾʱ����0��x���곬����Χ����-2��y���곬����Χ����-1��
//������byScreen���޷����ַ�����������š�
//x��y���޷��Ŷ���������ʾ�ַ�������λ�á�
//wAttrib����ʾ��������(�������ʾ��������˵��)��
//pcString���ַ���ָ�룬��NULL������
//���ܣ�ʹ��ϵͳȱʡ����͵�ǰϵͳ������ɫ��ʾ�ַ��������������ַ������Զ�ʶ����Ӣ��
//���������ַ���
//���ڷ���ʾ��(����ֵΪ0)������䱳��ɫ(��͸����ʾ����ԭ����ʾ�����ϵ���)��
//����ַ����м�������з���
//(ASCII��ֵΪ0x0A)������л�����ʾ����һ�еĺ�����Ϊx������������Ϊ��ǰ�е������ꡢ
//ȱʡ�������߶�
//(��Ϊ8*16������Ϊ16)�Լ��о����ߵĺ͡�
//==============================================================================
uint16 Lcd_StrTr ( uint8 byScreen, uint16 x, uint16 y,  uint8 *pcString )
{
    uint16 charindex, err,bakx,baky,LineOver;

    bakx=x;
    baky=y;
    Lcd_Pub_GetCurrentSet(byScreen);
    charindex=0;
    if((g_tDspAttrib.StringDirect==0)&&(g_tDspAttrib.Rotate==0))
    {   //�ַ�����ת���ַ�������,����Ϊ�ַ������Ͻ�����
        if(x>CN_LCD_SIZE_X-g_tCVsr->wASCIIFontWidth)return(-1);   //��ʼX����Խ�磬ֱ�ӷ���
        while(pcString[charindex]!=0x0)
        {
            /*������Խ���򷵻ش�����Ϣ*/
            if(y>=CN_LCD_SIZE_Y-g_tCVsr->wGBFontHeight) return(-2);
            if(pcString[charindex]>0x7f)
            /*��ʾ����*/
            {
                if(x>CN_LCD_SIZE_X-g_tCVsr->wGBFontWidth)
                /*�����겻��һ����*/
                {
                    err=-1;
                    for(LineOver=0;LineOver<100;LineOver++)
                    {//�������з�
                        charindex++;
                        if(pcString[charindex]==0x0a)
                        {//�ҵ����з������С�
                            x=bakx;
                            y+=g_tCVsr->wGBFontHeight+g_tDspAttrib.rowSpace;
                            charindex++;
                            break;
                        }
                        if(pcString[charindex]==0x0)
                            return(err);
                    }
                    //LineOver>=100����û�����������з������ش���
                    if( LineOver>=100 )   return( err );
                }else
                {
                    Lcd_GB2312Tr(byScreen, x,y, (uint16)(pcString[charindex]*256+pcString[charindex+1]) );
                    charindex+=2;x+=g_tCVsr->wGBFontWidth;
                }
            }
            else
            {/*��ʾ�ַ�*/
                if(pcString[charindex]==0x0a)
                {
                    x=bakx;
                    y+=g_tCVsr->wGBFontHeight+g_tDspAttrib.rowSpace;
                    charindex++;
                }
                else
                {
                    if(pcString[charindex]<0x10)
                    {//С��0x10��������
                        charindex++;
                        continue;
                    }
                    if(x>CN_LCD_SIZE_X-g_tCVsr->wASCIIFontWidth)
                    /*�����겻��һ���ַ�*/
                    {
                        err=-1;
                        for(LineOver=0;LineOver<100;LineOver++)
                        {//�������з�
                            charindex++;
                            if(pcString[charindex]==0x0a)
                            {//�ҵ����з������С�
                                x=bakx;
                                y+=g_tCVsr->wGBFontHeight+g_tDspAttrib.rowSpace;
                                charindex++;
                                break;
                            }
                            if(pcString[charindex]==0x0)
                                return(err);
                        }
                        //LineOver>=100����û�����������з������ش���
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
    {   //�ַ�����ת���ַ��������������£�,����Ϊ�ַ������Ͻ�����
        if(y>CN_LCD_SIZE_Y-g_tCVsr->wGBFontHeight)return(-2);   //��ʼY����Խ�磬ֱ�ӷ���
        while(pcString[charindex]!=0x0)
        {
            /*������Խ���򷵻ش�����Ϣ*/
//          if(x<g_tCVsr->wGBFontWidth-1) return(-2);
            if(pcString[charindex]>0x7f)
            /*��ʾ����*/
            {
                if(y>CN_LCD_SIZE_Y-g_tCVsr->wGBFontHeight)
                /*�����겻��һ����*/
                {
                    err=-1;
                    for(LineOver=0;LineOver<100;LineOver++)
                    {//�������з�
                        charindex++;
                        if(pcString[charindex]==0x0a)
                        {//�ҵ����з������С�
                            y=baky;
                            x-=g_tCVsr->wGBFontWidth+g_tDspAttrib.rowSpace;
                            charindex++;
                            break;
                        }
                        if(pcString[charindex]==0x0)
                            return(err);
                    }
                    //LineOver>=100����û�����������з������ش���
                    if( LineOver>=100 )   return( err );
                }else
                {
                    Lcd_GB2312Tr(byScreen, (uint16)(x-g_tCVsr->wGBFontWidth+1),y,\
                            (uint16)(pcString[charindex]*256+pcString[charindex+1]) );
                    charindex+=2;y+=g_tCVsr->wGBFontHeight;
                }
            }
            else
            /*��ʾ�ַ�*/
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
                    {//С��0x10��������
                        charindex++;
                        continue;
                    }
                    if(y>CN_LCD_SIZE_Y-g_tCVsr->wASCIIFontHeight)
                    /*�����겻��һ���ַ�*/
                    {
                        err=-1;
                        for(LineOver=0;LineOver<100;LineOver++)
                        {//�������з�
                            charindex++;
                            if(pcString[charindex]==0x0a)
                            {//�ҵ����з������С�
                                y=baky;
                                x-=g_tCVsr->wGBFontWidth+g_tDspAttrib.rowSpace;
                                charindex++;
                                break;
                            }
                            if(pcString[charindex]==0x0)
                                return(err);
                        }
                        //LineOver>=100����û�����������з������ش���
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
    {   //�ַ���ת���ַ�������,����Ϊ�ַ������Ͻ�����
        if(x>CN_LCD_SIZE_X-g_tCVsr->wGBFontHeight)return(-1);   //��ʼX����Խ�磬ֱ�ӷ���
        while(pcString[charindex]!=0x0)
        {
            /*������Խ���򷵻ش�����Ϣ*/
            if(pcString[charindex]>0x7f)
            /*��ʾ����*/
            {
                if(y>=CN_LCD_SIZE_Y-g_tCVsr->wGBFontWidth) return(-1);
                if(x>CN_LCD_SIZE_X-g_tCVsr->wGBFontWidth)
                /*�����겻��һ����*/
                {
                    err=-1;
                    for(LineOver=0;LineOver<100;LineOver++)
                    {//�������з�
                        charindex++;
                        if(pcString[charindex]==0x0a)
                        {//�ҵ����з������С�
                            x=bakx;
                            y+=g_tCVsr->wGBFontWidth+g_tDspAttrib.rowSpace;
                            charindex++;
                            break;
                        }
                        if(pcString[charindex]==0x0)
                            return(err);
                    }
                    //LineOver>=100����û�����������з������ش���
                    if( LineOver>=100 )   return( err );
                }else
                {
                    Lcd_GB2312Tr(byScreen,x,(uint16)(y+g_tCVsr->wGBFontWidth-1),
                            (uint16)(pcString[charindex]*256+pcString[charindex+1]) );
                    charindex+=2;x+=g_tCVsr->wGBFontWidth;
                }
            }
            else
            /*��ʾ�ַ�*/
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
                    {//С��0x10��������
                        charindex++;
                        continue;
                    }
                    if(x>CN_LCD_SIZE_X-g_tCVsr->wGBFontWidth)
                    /*�����겻��һ���ַ�*/
                    {
                        err=-1;
                        for(LineOver=0;LineOver<100;LineOver++)
                        {//�������з�
                            charindex++;
                            if(pcString[charindex]==0x0a)
                            {//�ҵ����з������С�
                                x=bakx;
                                y+=g_tCVsr->wGBFontWidth+g_tDspAttrib.rowSpace;
                                charindex++;
                                break;
                            }
                            if(pcString[charindex]==0x0)
                                return(err);
                        }
                        //LineOver>=100����û�����������з������ش���
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
    {   //�ַ���ת���ַ��������������ϣ�,����Ϊ�ַ������½�����
        if(y<g_tCVsr->wASCIIFontWidth-1)return(-2);   //��ʼY����Խ�磬ֱ�ӷ���
        while(pcString[charindex]!=0x0)
        {
            /*������Խ���򷵻ش�����Ϣ*/
            if(x>CN_LCD_SIZE_X-g_tCVsr->wGBFontHeight) return(-2);
            if(pcString[charindex]>0x7f)
            /*��ʾ����*/
            {
                if(y<g_tCVsr->wGBFontWidth-1)
                /*�����겻��һ����*/
                {
                    err=-1;
                    for(LineOver=0;LineOver<100;LineOver++)
                    {//�������з�
                        charindex++;
                        if(pcString[charindex]==0x0a)
                        {//�ҵ����з������С�
                            y=baky;
                            x+=g_tCVsr->wGBFontHeight+g_tDspAttrib.rowSpace;
                            charindex++;
                            break;
                        }
                        if(pcString[charindex]==0x0)
                            return(err);
                    }
                    //LineOver>=100����û�����������з������ش���
                    if( LineOver>=100 )   return( err );
                }else
                {
                    Lcd_GB2312Tr( byScreen, x,y,
                                  (uint16)(pcString[charindex]*256+pcString[charindex+1]) );
                    charindex+=2;y-=g_tCVsr->wGBFontWidth;
                }
            }
            else
            /*��ʾ�ַ�*/
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
                    {//С��0x10��������
                        charindex++;
                        continue;
                    }
                    if(y<g_tCVsr->wASCIIFontWidth-1)
                    /*�����겻��һ���ַ�*/
                    {
                        err=-1;
                        for(LineOver=0;LineOver<100;LineOver++)
                        {//�������з�
                            charindex++;
                            if(pcString[charindex]==0x0a)
                            {//�ҵ����з������С�
                                y=baky;
                                x+=g_tCVsr->wGBFontWidth+g_tDspAttrib.rowSpace;
                                charindex++;
                                break;
                            }
                            if(pcString[charindex]==0x0)
                                return(err);
                        }
                        //LineOver>=100����û�����������з������ش���
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
//���أ���ȷ��ʾʱ����0��x���곬����Χ����-2��y���곬����Χ����-1��byBitNum����-3
//������byScreen���޷����ַ�����������š�
//x��y���޷��Ŷ���������ʾ�ַ��ĵ�������λ�á�
//wAttrib����ʾ��������(�������ʾ��������˵��)��
//dwData���޷��ų���������Ҫ��ʾ�����ݡ�
//byBitNum�����ݵ��ַ����ȣ�λ��������ǰ�油�㡣
//byDotPos��С����λ�ã������������������ڵ���byBitNum����С���㣬������λ����
//���ܣ�ʹ��ϵͳȱʡ����͵�ǰϵͳ������ʮ���Ʒ�ʽ��ʾ����������С���㣬���У���
//��������Ļʱ����ʾ��Ļ��Χ�ڵĲ��֡�
//==============================================================================
uint16 Lcd_Dec ( uint8 byScreen, uint16 x, uint16 y,  uint32 dwData, uint8 byBitNum, uint8 byDotPos )
{
    char buf[20];
    uint16 wBitX, i,err,l,h;
    int strl;

    if( (byBitNum>12) || (byBitNum==0) ) return(-3);  // ���������10�������ݴ�
    if( (y+g_tCVsr->wASCIIFontHeight) >CN_LCD_SIZE_Y ) return(-1);  //������Խ��,ֱ�ӷ���.
    if( (x+g_tCVsr->wASCIIFontWidth ) >CN_LCD_SIZE_X ) return(-2);  //��Խ�磬ֱ�ӷ���

    Lcd_Pub_GetCurrentSet( byScreen );

    g_tDspAttrib.StringDirect=0;     // ���β���Ҫ�����ԣ���ֹ����
    g_tDspAttrib.Rotate=0;

    err=0;
    if( dwData<65536 )  sprintf(buf,"%u",(uint16)dwData);
    else                sprintf(buf,"%lu",(unsigned long)dwData);
    strl=strlen(buf)-1;

    wBitX = x + byBitNum * g_tCVsr->wASCIIFontWidth;    // ��ǰλ��X�����ʼֵ
    for( i=byBitNum; i>0; i-- )
    {
        wBitX = wBitX - g_tCVsr->wASCIIFontWidth;       // ��ǰλ��X����:x+(i-1)*g_tCVsr->wASCIIFontWidth
        if( (wBitX+g_tCVsr->wASCIIFontWidth) <= CN_LCD_SIZE_X )
        {// ��ǰλ���������δ������Ļ
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
        {// ������Ļ������ʾ������Ļ�ڵ�����һ���ַ������ʣ�ಿ�֡�
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
        if( i!=(byBitNum-byDotPos) )  strl--;       // С���㲻��������ʱ�ַ�����
    }

    return err;
}



//==============================================================================
//���أ���ȷ��ʾʱ����0��x���곬����Χ����-2��y���곬����Χ����-1��
//������byScreen���޷����ַ�����������š�
//x��y���޷��Ŷ���������ʾ�ַ��ĵ�������λ�á�
//wAttrib����ʾ��������(�������ʾ��������˵��)��
//dwData���޷��ų���������Ҫ��ʾ�����ݡ�
//byBitNum�����ݵ��ַ����ȣ�λ��������ǰ�油�㡣
//���ܣ�ʹ��ϵͳȱʡ����͵�ǰϵͳ������ʮ�����Ʒ�ʽ��ʾ���ݣ��ַ���д����0xǰ׺��
//��������Ļʱ����ʾ��Ļ��Χ�ڵĲ��֡�
//==============================================================================
uint16 Lcd_Hex ( uint8 byScreen, uint16 x, uint16 y,  uint32 dwData, uint8 byBitNum )
{
    char buf[20];
    uint16 wBitX, i,err,l,h;
    int  strl;

    if( (byBitNum>12) || (byBitNum==0) ) return(-3);  // ���������10�������ݴ�
    Lcd_Pub_GetCurrentSet(byScreen);
    if (y>CN_LCD_SIZE_Y-g_tCVsr->wASCIIFontHeight) return(-1); //������Խ��,ֱ�ӷ���.
    if(x>CN_LCD_SIZE_X-g_tCVsr->wASCIIFontWidth) return(-2);  //��Խ�磬ֱ�ӷ���
    g_tDspAttrib.StringDirect=0;     //���β���Ҫ�����ԣ���ֹ����
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

    wBitX = x + byBitNum * g_tCVsr->wASCIIFontWidth;    // ��ǰλ��X�����ʼֵ
    for( i=byBitNum;i>0;i-- )
    {
        wBitX = wBitX -g_tCVsr->wASCIIFontWidth;        // ��ǰλ��X����:x+(i-1)*g_tCVsr->wASCIIFontWidth
        if( (wBitX+g_tCVsr->wASCIIFontWidth) <=CN_LCD_SIZE_X )
        {// ��ǰλ���������δ������Ļ
            if(strl>=0)
                Lcd_Ascii(byScreen, wBitX,y,buf[strl]);
            else
                Lcd_Ascii(byScreen, wBitX,y,'0');

        }else
        {// ������Ļ������ʾ������Ļ�ڵ�����һ���ַ������ʣ�ಿ�֡�
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
//���أ��ޡ�
//������byScreen���޷����ַ�����������š�
//lx��ty���޷��Ŷ��������������Ͻǵ����ꡣ
//rx��by���޷��Ŷ��������������½ǵ����ꡣ
//dwRGB���߿���ɫ��
//���ܣ�ʹ��ϵͳ��ǰ���û�һ���Ρ�x��y���곬����Χ��ȡҺ����ɵ����ֵ��
//==============================================================================
void Lcd_Rect( uint8 byScreen, uint16 lx, uint16 ty, uint16 rx, uint16 by, uint32 dwRGB )
{
    uint32 dwRRGB;
    dwRRGB=Lcd_GetFontColor(byScreen);
    Lcd_SetFontColor(byScreen,dwRGB);   //���õ�ǰ��ɫ����
    Lcd_LineH( byScreen, lx, rx,ty, 1 );
    Lcd_LineH( byScreen, lx, rx + 1,by, 1 );
    Lcd_LineV( byScreen, ty, by,lx, 1 );
    Lcd_LineV( byScreen, ty, by,rx, 1 );
    Lcd_SetFontColor(byScreen,dwRRGB);  //�ָ���ǰ��ɫ����
}
//==============================================================================
//���أ��ޡ�
//������byScreen���޷����ַ�����������š�
//lx��ty���޷��Ŷ��������������Ͻǵ����ꡣ
//rx��by���޷��Ŷ��������������½ǵ����ꡣ
//dwRGB���޷��Ŷ��������������ʾ���ݡ�
//���ܣ��õ�����������������x��y���곬����Χ��ȡҺ����ɵ����ֵ��
//==============================================================================
void Lcd_FillRect( uint8 byScreen, uint16 lx, uint16 ty, uint16 rx, uint16 by, uint32 dwRGB)
{
    uint16 h,l,err;
    uint32 dwRRGB;
    dwRRGB=Lcd_GetFontColor(byScreen);
    Lcd_SetFontColor(byScreen,dwRGB);   //���õ�ǰ��ɫ����
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
    Lcd_SetFontColor(byScreen,dwRRGB);  //�ָ���ǰ��ɫ����
}

//==============================================================================
//���أ��ޡ�
//������byScreen���޷����ַ�����������š�
//lx��ty���޷��Ŷ��������������Ͻǵ����ꡣ
//rx��by���޷��Ŷ��������������½ǵ����ꡣ
//dwRGB����ť��ɫ��
//���ܣ�ʹ��ϵͳ��ǰ���û�һ��ɫ�߿���Ρ�x��y���곬����Χ��ȡҺ����ɵ����ֵ����ť��dwRGB��䡣
//==============================================================================
void Lcd_White_Button( uint8 byScreen, uint16 lx, uint16 ty, uint16 rx, uint16 by, uint32 dwRGB )
{
    uint16 h,l,err;
    uint32 dwRRGB;
    dwRRGB=Lcd_GetFontColor(byScreen);
    Lcd_SetFontColor(byScreen,dwRGB);   //���õ�ǰ��ɫ����
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
    Lcd_SetFontColor(byScreen,CN_RGB_WHITE);   //������༰�ϲ�����ɫ����Ϊ��ɫ
    Lcd_LineH( byScreen, lx, rx,ty, 1 );
    Lcd_LineV( byScreen, ty, by,lx, 1 );
    Lcd_SetFontColor(byScreen,CN_RGB_WHITE);   //�����²༰�Ҳ�����ɫ����Ϊ��ɫ
    Lcd_LineH( byScreen, lx, rx + 1,by, 1 );
    Lcd_LineV( byScreen, ty, by,rx, 1 );
    Lcd_SetFontColor(byScreen,dwRRGB);  //�ָ���ǰ��ɫ����
}

//==============================================================================
//���أ��ޡ�
//������byScreen���޷����ַ�����������š�
//lx��ty���޷��Ŷ��������������Ͻǵ����ꡣ
//rx��by���޷��Ŷ��������������½ǵ����ꡣ
//dwRGB����ť��ɫ��
//���ܣ�ʹ��ϵͳ��ǰ���û�һ��ɫ�߿���Ρ�x��y���곬����Χ��ȡҺ����ɵ����ֵ����ť��dwRGB��䡣
//==============================================================================
void Lcd_Black_Button( uint8 byScreen, uint16 lx, uint16 ty, uint16 rx, uint16 by, uint32 dwRGB )
{
    uint16 h,l,err;
    uint32 dwRRGB;
    dwRRGB=Lcd_GetFontColor(byScreen);
    Lcd_SetFontColor(byScreen,dwRGB);   //���õ�ǰ��ɫ����
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
    Lcd_SetFontColor(byScreen,CN_RGB_BLACK);   //������༰�ϲ�����ɫ����Ϊ��ɫ
    Lcd_LineH( byScreen, lx, rx,ty, 1 );
    Lcd_LineV( byScreen, ty, by,lx, 1 );
    Lcd_SetFontColor(byScreen,CN_RGB_BLACK);   //�����²༰�Ҳ�����ɫ����Ϊ��ɫ
    Lcd_LineH( byScreen, lx, rx + 1,by, 1 );
    Lcd_LineV( byScreen, ty, by,rx, 1 );
    Lcd_SetFontColor(byScreen,dwRRGB);  //�ָ���ǰ��ɫ����
}

//==============================================================================
//���أ��ޡ�
//������byScreen���޷����ַ�����������š�
//lx��ty���޷��Ŷ��������������Ͻǵ����ꡣ
//rx��by���޷��Ŷ��������������½ǵ����ꡣ
//dwRGB����ť��ɫ��
//���ܣ�ʹ��ϵͳ��ǰ���û�һ����ť��x��y���곬����Χ��ȡҺ����ɵ����ֵ����ť��dwRGB��䡣
//==============================================================================
void Lcd_Button( uint8 byScreen, uint16 lx, uint16 ty, uint16 rx, uint16 by, uint32 dwRGB )
{
    uint16 h,l,err;
    uint32 dwRRGB;
    dwRRGB=Lcd_GetFontColor(byScreen);
    Lcd_SetFontColor(byScreen,dwRGB);   //���õ�ǰ��ɫ����
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
    Lcd_SetFontColor(byScreen,CN_RGB_WHITE);   //������༰�ϲ�����ɫ����Ϊ��ɫ
    Lcd_LineH( byScreen, lx, rx,ty, 1 );
    Lcd_LineV( byScreen, ty, by,lx, 1 );
    Lcd_SetFontColor(byScreen,CN_RGB_BLACK);   //�����²༰�Ҳ�����ɫ����Ϊ��ɫ
    Lcd_LineH( byScreen, lx, rx,by, 1 );
    Lcd_LineV( byScreen, ty, by,rx, 1 );
    Lcd_SetFontColor(byScreen,dwRRGB);  //�ָ���ǰ��ɫ����
}

//==============================================================================
//���أ��ޡ�
//������byScreen���޷����ַ�����������š�
//lx��ty���޷��Ŷ��������������Ͻǵ����ꡣ
//rx��by���޷��Ŷ��������������½ǵ����ꡣ
//dwRGB����ť��ɫ��
//���ܣ�ʹ��ϵͳ��ǰ���û�һ���ݰ�ť��x��y���곬����Χ��ȡҺ����ɵ����ֵ����ť��dwRGB��䡣
//==============================================================================
void Lcd_Sunk( uint8 byScreen, uint16 lx, uint16 ty, uint16 rx, uint16 by, uint32 dwRGB )
{
    uint16 h,l,err;
    uint32 dwRRGB;
    dwRRGB=Lcd_GetFontColor(byScreen);
    Lcd_SetFontColor(byScreen,dwRGB);   //���õ�ǰ��ɫ����
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
    Lcd_SetFontColor(byScreen,CN_RGB_BLACK);   //������༰�ϲ�����ɫ����Ϊ��
    Lcd_LineH( byScreen, lx, rx,ty, 1 );
    Lcd_LineV( byScreen, ty, by,lx, 1 );
    Lcd_SetFontColor(byScreen,CN_RGB_WHITE);   //�����²༰�Ҳ�����ɫ����Ϊ��
    Lcd_LineH( byScreen, lx, rx,by, 1 );
    Lcd_LineV( byScreen, ty, by,rx, 1 );

    Lcd_SetFontColor(byScreen,dwRRGB);  //�ָ���ǰ��ɫ����
}

//==============================================================================
//����:��ָ���ľ��ο����ͻ���İ�ťЧ��������䰴ť��ɫ
//����ֵ:��
//����:byScreen���޷����ַ�����������š�
//     lx��ty���޷��Ŷ��������������Ͻǵ����ꡣ
//     rx��by���޷��Ŷ��������������½ǵ����ꡣ
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
    Lcd_SetFontColor(byScreen,CN_RGB_WHITE);   //������༰�ϲ�����ɫ����Ϊ��
    Lcd_LineH( byScreen, lx, rx,ty, 1 );
    Lcd_LineV( byScreen, ty, by,lx, 1 );
    Lcd_SetFontColor(byScreen,CN_RGB_BLACK);   //�����²༰�Ҳ�����ɫ����Ϊ��
    Lcd_LineH( byScreen, lx, rx,by, 1 );
    Lcd_LineV( byScreen, ty, by,rx, 1 );

    Lcd_SetFontColor(byScreen,dwRGB);  //�ָ���ǰ��ɫ����
}


//==============================================================================
//����:��ָ���ľ��ο����ѹ�µİ�ťЧ��������䰴ť��ɫ
//����ֵ:��
//����:byScreen���޷����ַ�����������š�
//     lx��ty���޷��Ŷ��������������Ͻǵ����ꡣ
//     rx��by���޷��Ŷ��������������½ǵ����ꡣ
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
    Lcd_SetFontColor(byScreen,CN_RGB_BLACK);   //������༰�ϲ�����ɫ����Ϊ��
    Lcd_LineH( byScreen, lx, rx,ty, 1 );
    Lcd_LineV( byScreen, ty, by,lx, 1 );
    Lcd_SetFontColor(byScreen,CN_RGB_WHITE);   //�����²༰�Ҳ�����ɫ����Ϊ��
    Lcd_LineH( byScreen, lx, rx,by, 1 );
    Lcd_LineV( byScreen, ty, by,rx, 1 );

    Lcd_SetFontColor(byScreen,dwRGB);  //�ָ���ǰ��ɫ����
}


//==============================================================================
//����:���ָ���ľ��ο�İ�ťЧ��,�������ɫ�ػ��߿�
//����ֵ:��
//����:byScreen���޷����ַ�����������š�
//     lx��ty���޷��Ŷ��������������Ͻǵ����ꡣ
//     rx��by���޷��Ŷ��������������½ǵ����ꡣ
//     dwRGB:��ť���ɫ
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
    Lcd_SetFontColor(byScreen,dwRGB);   //������༰�ϲ�����ɫ����Ϊ��ťɫ
    Lcd_LineH( byScreen, lx, rx,ty, 1 );
    Lcd_LineV( byScreen, ty, by,lx, 1 );
    Lcd_LineH( byScreen, lx, rx,by, 1 );
    Lcd_LineV( byScreen, ty, by,rx, 1 );

    Lcd_SetFontColor(byScreen,dwRRGB);  //�ָ���ǰ��ɫ����
}




/*
unsigned short *pawbyLCDMemory;      //�����Դ�
unsigned char LcdDispBuf[261120];//480*272
*/
//==============================================================================
//���Һ����ʾ��Ӳ����ʼ����������������Ĭ�ϵı���ɫ��ǰ��ɫ�ȡ�
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
// �������ƣ�Lcd_Icon
// �������ܣ���ʾICON ͼ��
//
// ���������byLength:  ��Ϣ�򳤶�
// ���������byWidth :  ��Ϣ����
// ���������byNameStr : ��Ϣ����
// ���������byNameStr : ��Ϣ����ʾ��Ϣ
// ���������g_tDspCtrl.bFirst
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

	Lcd_Icon(1, g_byIcon_ip,   10, 10, 100, 100 );Lcd_Str( 1, 36,  114, "IP����" );
	Lcd_Icon(1, g_byIcon_rtc, 130, 10, 100, 100 );Lcd_Str( 1, 148, 114, "ʱ������" );
	Lcd_Icon(1, g_byIcon_led, 250, 10, 100, 100 );Lcd_Str( 1, 264, 114, "LED��ˮ��");
	Lcd_Icon(1, g_byIcon_phot,370, 10, 100, 100 );Lcd_Str( 1, 388, 114, "ͼƬ����");
	Lcd_Icon(1, g_byIcon_uart, 10, 146,100, 100); Lcd_Str( 1, 44,  250, "help");
	Lcd_Icon(1, g_byIcon_sd,  130, 146,100, 100); Lcd_Str( 1, 155, 250, "SD CARD" );
	Lcd_Icon(1, g_byIcon_usbd,250, 146,100, 100); Lcd_Str( 1, 260, 250, "USB DEVICE" );
	Lcd_Icon(1, g_byIcon_version,370, 146,100, 100); Lcd_Str( 1, 388, 250, "version" );

}

void Dsp_Init(void)
{
	// ��ʼ���Դ�
	Lcd_InitMemory();

	g_awLibHZ16_16 = (void *)g_byHzk1616;


	Lcd_Clear(1);
	
	Lcd_SetFontColor(1, CN_RGB_BLACK);
	Lcd_SetFont(1,CN_GB16BIT,CN_ASCII168);

}


