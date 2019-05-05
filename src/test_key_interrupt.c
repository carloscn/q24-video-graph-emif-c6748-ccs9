//-----------------------------------------------------------------------------
// \file    rk6748_timer.c
// \brief   implementation of a timer driver for RK6748.
//
//-----------------------------------------------------------------------------
#include "types.h"
#include "rk6748.h"

#include "rk6748_gpio.h"
#include "interrupt.h"
#include "interrupt_gpio.h"

#include <stdio.h>
#include <string.h>

#define             DMA_ADRR            ( unsigned short * ) 0xc11F0020
#define             PIC1_ADDR            ( unsigned short * ) 0xc1000000
#define             PIC2_ADDR            ( unsigned short * ) 0xc2000000
#define             PIC3_ADDR            ( unsigned short * ) 0xc3000000


unsigned char key_done_flag = 0;
unsigned char Flag=0;
Uint16  count = 0;
void delays( long time )
{
    long i;
    for( i = 0; i < time ; i ++) {
        asm( " nop" );
        asm( " nop" );
        asm( " nop" );
        asm( " nop" );
        asm( " nop" );
        asm( " nop" );
        asm( " nop" );
        asm( " nop" );

    }
}

//LED3�װ�λ��LED1
void LED3_ON(void)//LED3--GP0_15
{
    GPIO_setOutput(0,15,0);
}

void LED3_OFF(void)//LED3--GP0_15 
{
    GPIO_setOutput(0,15,1);
}

void LED3_overturn( void )//LED3--GP0_15 
{
    UINT32 gpiodata;
    gpiodata=*((volatile UINT32 *)(GPIO_OUT_DATA01));
    gpiodata&=(1<<15);

    if(gpiodata>0)
        *((volatile UINT32 *)(GPIO_OUT_DATA01))&=~(UINT32)(1<<15);
    else
        *((volatile UINT32 *)(GPIO_OUT_DATA01))|=(1<<15);
}

//LED4�װ�λ��LED2
void LED4_ON(void)//LED4--GP6_11
{
    GPIO_setOutput(6,11,0);
}

void LED4_OFF(void)//LED4--GP6_11 
{
    GPIO_setOutput(6,11,1);
}

void LED4_overturn( void )//LED4--GP6_11
{
    UINT32 gpiodata;
    gpiodata=*((volatile UINT32 *)(GPIO_OUT_DATA67));
    gpiodata&=(1<<11);

    if(gpiodata>0)
        *((volatile UINT32 *)(GPIO_OUT_DATA67))&=~(UINT32)(1<<11);
    else
        *((volatile UINT32 *)(GPIO_OUT_DATA67))|=(1<<11);
}

//-----------------------------------------------------------------------------
// \brief   initialize the leds for use.
//
// \param   none.
//
// \return  UINT32
//    ERR_NO_ERROR - everything is ok...leds ready to use.
//    ERR_INIT_FAIL - something happened during initialization.
//-----------------------------------------------------------------------------
void KEY_init(void)
{
    RK6748_lpscTransition(PSC1, DOMAIN0, LPSC_GPIO, PSC_ENABLE);

    RK6748_pinmuxConfig(0, 0x0000000F, 0x00000008);//GP0_15
    GPIO_setDir(GPIO_BANK0,GPIO_PIN15,GPIO_OUTPUT);//GP0_15-LED3

    RK6748_pinmuxConfig(13, 0x000F0000, 0x00080000);//GP6_11
    GPIO_setDir(GPIO_BANK6,GPIO_PIN11,GPIO_OUTPUT);//GP6_11-LED4


    RK6748_pinmuxConfig(1, 0x0F000000, 0x08000000);//GP0_1
    GPIO_setDir(GPIO_BANK0,GPIO_PIN1,GPIO_INPUT);  //GP0_1-KEY2

    LED3_OFF();
    LED4_ON();
}


/****************************************************************************/
/*                                                                          */
/*              DSP �жϳ�ʼ��                                              */
/*                                                                          */
/****************************************************************************/
void InterruptInit(void)
{
    // ��ʼ�� DSP �жϿ�����
    IntDSPINTCInit();

    // ʹ�� DSP ȫ���ж�
    IntGlobalEnable();
}

/****************************************************************************/
/*                                                                          */
/*              �жϷ�����                                                */
/*                                                                          */
/****************************************************************************/
void KEYIsr(void)
{
    // ���� GPIO BANK 0 �ж�
    GPIOBankIntDisable(SOC_GPIO_0_REGS, 0);

    // ��� GPIO BANK 0 �ж�״̬
    IntEventClear(SYS_INT_GPIO_B0INT);

    // ��� GPIO0[9] �ж�״̬
    GPIOPinIntClear(SOC_GPIO_0_REGS, 2);


    // ʹ�� GPIO BANK 0 �ж�
    GPIOBankIntEnable(SOC_GPIO_0_REGS, 0);


    if( key_done_flag == 0 ) {
        count ++;
        key_done_flag = 1;
        if( count >=  8 ) {
            count = 1;
        }
        // ��ת��־λ
        Flag=!Flag;

        // �ı� LED ״̬
        if(Flag==0) LED4_ON();
        else LED4_OFF();
    }
}


/****************************************************************************/
/*                                                                          */
/*              GPIO �ܽ��жϳ�ʼ��                                         */
/*                                                                          */
/****************************************************d************************/
void GPIOBankPinInterruptInit(void)
{
    // �װ尴���ж�
    // ���� KEY GPIO0[1] Ϊ�½��ش���
    GPIOIntTypeSet(SOC_GPIO_0_REGS, 2, GPIO_INT_TYPE_FALLEDGE);

    // ʹ�� GPIO BANK �ж�
    GPIOBankIntEnable(SOC_GPIO_0_REGS, 0);

    // ע���жϷ�����
    IntRegister(C674X_MASK_INT8, KEYIsr);

    // ӳ���жϵ� DSP �������ж�
    IntEventMap(C674X_MASK_INT8, SYS_INT_GPIO_B0INT);

    // ʹ�� DSP �������ж�
    IntEnable(C674X_MASK_INT8);
}

UINT32 TEST_key_interrupt(void)
{

    printf("------------------------------------------------------------\r\n");
    printf("              C6748 KEY_INTERRUPT Test start...\r\n\r\n");

    KEY_init();
    LED3_OFF();
    LED4_OFF();
    InterruptInit();
    GPIOBankPinInterruptInit();

    printf("              C6748 KEY_INTERRUPT Test Over!\r\n\r\n");
    printf("------------------------------------------------------------\r\n");
    LED3_ON();
    LED4_ON();
    //while(1);
    return 1;
}
extern UINT16 pic;
void start_keys_scan( void )
{
    long delay_t = 975000;
    if( key_done_flag == 1 ) {

        Lcd_bmp((UINT16 *)(0xc1000000), 0, 0,640, 480);
        printf(" image1 \n");
        delays(delay_t);
        key_done_flag = 0;
        pic = ~pic;


    }
}

void select_delay_display( void )
{

}


