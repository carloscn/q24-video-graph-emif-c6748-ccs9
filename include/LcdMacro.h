/*========================================================*
 * �ļ�ģ��˵��:
 * Lcdmacro.c - TFT LCD��ʾ�궨��
 * �ļ��汾: V1.0.0
 * ������Ա: Rock
 * ����ʱ��: 2013-01-01 
 * Copyright(c) 2013-2015  Rock-Embed Limited Co.
 *========================================================*
 * �����޸ļ�¼��
 * <�汾��> <�޸�����>, <�޸���Ա>: <�޸Ĺ��ܸ���>
 *      V1.0.0           2013-01-01              Rock                        ����
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
#define CN_RGB_YELLOW   (0xE0E000)          // ��ɫRGB

//���嶨��
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

//��ʾ���ƽṹ�����ṹ�ڵ�Ԫ��ֻ�н�������ģ����Զ�ȡ���޸ģ�
//Ӧ�ó��򲻵�ʹ�ã���ͨ�����������ṩ�ĺ�����á�
//��Ļ��ɫ�Ŀ��ƣ��û�ֻ����24λ���ֵ���ƣ�����ʹ��6λ��16λ����ɫֵ��
typedef struct
{
    uint16   *pwbyLCDMemory;      //�����Դ�
    uint16    wFontColor;         // ������ɫ
    uint16    wBackColor;         // �ַ���ʾ������ɫ
    uint16    wGBFontWidth;       // ����������
    uint16    wGBFontHeight;      // ��������߶�
    uint16    wASCIIFontWidth;    // �ַ�������
    uint16    wASCIIFontHeight;   // �ַ�����߶�
    uint16    *pwLibHZ;           // ���ֿ��ַ
    uint8     *pbyLibAscii;       // ASCII���ַ
} tagScreenControl,*tagPScreenControl;


typedef struct {                      // ��ʾ�������ݽṹ
    uint16        style:1;            // ��ʾ��ʽ <0=������ʾ; 1=����>
    uint16        StringDirect:1;     // �ַ���ʾ����<0=������ʾ, 1=������ʾ>
    uint16        fillZero:1;         // 10��������ʾǰ���Ƿ�0��0=����0,1=��0;
    uint16        Rotate:1;           // �ַ��Ƿ���ת��ʾ��Ŀǰֻ֧����ʱ��ת90��
    uint16        res:6;              // ����
    uint16        rowSpace:4;         // �о�
}tagDspAttrib, *tagPDspAttrib;


#define Lcd_Pub_PixelC( x, y, wColor )  (g_tCVsr->pwbyLCDMemory[(y*CN_LCD_SIZE_X+x)] = wColor)
#define Lcd_Pub_PixelFT( x, y )    Lcd_Pub_PixelC( (x), (y), g_tCVsr->wFontColor )
#define Lcd_Pub_PixelBK( x, y )    Lcd_Pub_PixelC( (x), (y), g_tCVsr->wBackColor )
#define M_GuiSwap(a, b) 		   { a^=b; b^=a; a^=b; }

tagDspAttrib        g_tDspAttrib;                   // ��ʾ����
tagScreenControl    g_tVsr[CN_NUM_SCREEN];          // ���������ƿ�
tagPScreenControl   g_tCVsr;                        // ��ǰ����ṹָ��

unsigned short (*g_awLibHZ16_16)[16];                       // 16��16���ֿ�


extern uint8 g_byHzk1616[261696];
extern uint8 g_byASCII168[][16];

//��������ʾͼ��
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


