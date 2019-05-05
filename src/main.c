//-----------------------------------------------------------------------------
// \file    main.c
// \brief   implementation of main() to test bsl drivers.
//
//-----------------------------------------------------------------------------
#include "types.h"
#include "stdio.h"
#define IMAGE_SIZE_X       (640)
#define IMAGE_SIZE_Y       (480)
#include <stdio.h>
#include <string.h>


#define             DMA_ADRR             ( unsigned short * ) 0xc11F0020
#define             PIC1_ADDR            ( unsigned short * ) 0xc1000000
#define             PIC2_ADDR            ( unsigned short * ) 0xc2000000
#define             PIC3_ADDR            ( unsigned short * ) 0xc3000000

void    get_camera_out( unsigned short *photo_datas );
extern void start_keys_scan( void );
extern UINT32 TEST_key_interrupt(void);
void pic_deal_gray( unsigned short *origin, unsigned short *gray );
UINT16  pic = 0;
//-----------------------------------------------------------------------------
// \brief   entry point for bsl test code.
//
// \param   none.
//
// \return  none.
//-----------------------------------------------------------------------------
void main(void)
{	

    unsigned short *test = PIC2_ADDR,*test_g = PIC3_ADDR;
    //TEST_key_interrupt();
    USTIMER_init();
    LED_init();
    DebugUartInit();
    TEST_key_interrupt();
    RK6748_lcdInit();
    //get_camera_out(test);
    //pic_deal_gray(test,test_g);
    //pic_set_gray(test,test_g);
    //Lcd_bmp((UINT16 *)test_g, 0, 0,640, 480);
    TEST_lcd();
    TEST_imageSensor();
    //rgb565_test();
    TEST_lcd();
    while( 1 ) {

        start_keys_scan();
        if( pic == 0 ) {
            Lcd_bmp((UINT16 *)(0xc1000000), 0, 0,640, 480);
            //rgb565_test();
        }
    }
}

void    get_camera_out( unsigned short *photo_datas )
{
    unsigned short *input_raw_bits;         // Linux Programmer

    unsigned long   i,j;

    input_raw_bits      =   PIC1_ADDR;

    for( i = 0; i < IMAGE_SIZE_Y; i++ ) {
        for( j = 0; j < IMAGE_SIZE_X; j++ ) {
            *photo_datas  ++= *input_raw_bits++;
        }
    }

}






