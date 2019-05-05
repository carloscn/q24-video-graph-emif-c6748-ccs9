/*========================================================*
 * 文件模块说明:
 * Lcdmacro.c - TFT LCD显示宏定义
 * 文件版本: V1.0.0
 * 开发人员: Rock
 * 创建时间: 2013-01-01 
 * Copyright(c) 2013-2015  Rock-Embed Limited Co.
 *========================================================*
 * 程序修改记录：
 * <版本号> <修改日期>, <修改人员>: <修改功能概述>
 *      V1.0.0           2013-01-01              Rock                        创建
 *========================================================*/
#ifndef _LCD_MACRO_H_
#define _LCD_MACRO_H_

#define CN_LCD_SIZE_X           (640)
#define CN_LCD_SIZE_Y           (480)
#define CN_LCD_PIXElWIDE        ( 16)
#define CN_DISPLAYMEMORYSIZE    ( (CN_LCD_SIZE_X<<1)*CN_LCD_SIZE_Y )
#define CN_NUM_SCREEN           ( 2 )


typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef unsigned int        uint32;
typedef unsigned short      uint16;
typedef int                 sint32_t;
typedef short int           sint16_t;
typedef unsigned  char      uint8 ;
typedef signed  char        sint8_t;
typedef unsigned            bool_t;

#ifndef true
  #define false 0
#endif
#ifndef true
  #define true 1
#endif


#define CN_SCR_ALARM    (0)
#define CN_SCR_RUN      (1)
#define CN_SCR_MENU     (2)
#define CN_SCR_USER     (3)

#define CN_RGB_WHITE    (0x00FFFFFF)
#define CN_RGB_BLACK    (0x00000000)
#define CN_RGB_RED      (0x00FF0000)
#define CN_RGB_GREEN    (0x0000FF00)
#define CN_RGB_BLUE     (0x000000FF)
#define CN_RGB_CYAN     (0x0000C0C0)
#define CN_RGB_YELLOW   (0xE0E000)          // 黄色RGB

//字体定义
#define CN_GB16BIT      (16)
#define CN_ASCII168     (16)

enum _MENU_STATUS_
{                          
    Menu_byIcon_ip = 1,
    Menu_byIcon_rtc,
    Menu_byIcon_led,
    Menu_byIcon_phot,
    Menu_byIcon_help,
    Menu_byIcon_sd,
    Menu_byIcon_ushd,
    Menu_byIcon_usbh,
};

//显示控制结构，本结构内的元素只有界面驱动模块可以读取和修改，
//应用程序不得使用，可通过驱动程序提供的函数获得。
//屏幕颜色的控制，用户只能用24位真彩值控制，不得使用6位和16位的颜色值。
typedef struct
{
    uint16   *pwbyLCDMemory;      //定义显存
    uint16    wFontColor;         // 字体颜色
    uint16    wBackColor;         // 字符显示背景颜色
    uint16    wGBFontWidth;       // 汉字字体宽度
    uint16    wGBFontHeight;      // 汉字字体高度
    uint16    wASCIIFontWidth;    // 字符字体宽度
    uint16    wASCIIFontHeight;   // 字符字体高度
    uint16    *pwLibHZ;           // 汉字库地址
    uint8     *pbyLibAscii;       // ASCII库地址
} tagScreenControl,*tagPScreenControl;


typedef struct {                      // 显示属性数据结构
    uint16        style:1;            // 显示方式 <0=正常显示; 1=返显>
    uint16        StringDirect:1;     // 字符显示方向<0=横向显示, 1=竖向显示>
    uint16        fillZero:1;         // 10进制数显示前面是否补0。0=不补0,1=补0;
    uint16        Rotate:1;           // 字符是否旋转显示，目前只支持逆时针转90度
    uint16        res:6;              // 保留
    uint16        rowSpace:4;         // 行距
}tagDspAttrib, *tagPDspAttrib;


#define Lcd_Pub_PixelC( x, y, wColor )  (g_tCVsr->pwbyLCDMemory[(y*CN_LCD_SIZE_X+x)] = wColor)
#define Lcd_Pub_PixelFT( x, y )    Lcd_Pub_PixelC( (x), (y), g_tCVsr->wFontColor )
#define Lcd_Pub_PixelBK( x, y )    Lcd_Pub_PixelC( (x), (y), g_tCVsr->wBackColor )
#define M_GuiSwap(a, b) 		   { a^=b; b^=a; a^=b; }

tagDspAttrib        g_tDspAttrib;                   // 显示属性
tagScreenControl    g_tVsr[CN_NUM_SCREEN];          // 定义界面控制块
tagPScreenControl   g_tCVsr;                        // 当前界面结构指针

unsigned short (*g_awLibHZ16_16)[16];                       // 16×16汉字库


extern uint8 g_byHzk1616[261696];
extern uint8 g_byASCII168[][16];

//触摸屏显示图标
extern unsigned char g_byIcon_ip[ ];
extern unsigned char g_byIcon_rtc[ ];
extern unsigned char g_byIcon_led[ ];
extern unsigned char g_byIcon_phot[ ];
extern unsigned char g_byIcon_uart[ ];
extern unsigned char g_byIcon_sd[ ];
extern unsigned char g_byIcon_usbd[ ];
extern unsigned char g_byIcon_version[ ];
extern unsigned char g_byIcon_gray[ ];

#endif /* _lcd_drv_H_*/


