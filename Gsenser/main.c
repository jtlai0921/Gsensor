#include "Nano100Series.h"
#include <stdio.h>
#include <math.h>

void i2c_start(void);
void i2c_stop(void);
uint8_t i2c_write(uint8_t d);
uint8_t i2c_read(uint8_t ack);

//I2C
#define RD 1 
#define WR 0
#define ACK 0
#define NACK 1

//---
//            aa,   us, rt, ch, bps, dbm, pw, nak
//  short cfg[]={1, 1000,  3,  5, 250, -12, 32,  0};
void nrf_init(short *cfg);
short nrf_input(char* p);
char nrf_output(char *p);
//---

//----------- UART0 --------------
void init_UART0(void){
  SYS_UnlockReg();
    //--- CLK
    CLK_EnableModuleClock(UART0_MODULE);
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART_S_HXT, CLK_UART_CLK_DIVIDER(1));
    //--- PIN
    SYS->PB_L_MFP &=~ SYS_PB_L_MFP_PB0_MFP_Msk;
    SYS->PB_L_MFP |=  SYS_PB_L_MFP_PB0_MFP_UART0_RX;
    SYS->PB_L_MFP &=~ SYS_PB_L_MFP_PB1_MFP_Msk;
    SYS->PB_L_MFP |=  SYS_PB_L_MFP_PB1_MFP_UART0_TX;
  SYS_LockReg();
    //--- OPEN
    UART_Open(UART0, 115200);
    //--- NVIC
    UART_ENABLE_INT(UART0, UART_IER_RDA_IE_Msk);
    NVIC_EnableIRQ(UART0_IRQn);
}
  //--- IRQ
void UART0_IRQHandler(void){
    UART0->THR = UART0->RBR;
}

//-------------- HCLK  -------------
void init_HCLK(void){
  SYS_UnlockReg();
    CLK_EnableXtalRC(CLK_PWRCTL_HXT_EN_Msk);
    CLK_WaitClockReady( CLK_CLKSTATUS_HXT_STB_Msk);
    CLK_SetHCLK(CLK_CLKSEL0_HCLK_S_HXT,CLK_HCLK_CLK_DIVIDER(1));
  SYS_LockReg();
}
//--------------- Systick ------------------
uint32_t tick=0;
void SysTick_Handler(void){
    tick++;
};
 
void delay_ms(uint32_t ms){
    ms += tick;
    while( ms != tick){}
}
 
//-------------- INIT  -------------
void init(void){
  init_HCLK();
		init_UART0();
		SystemCoreClockUpdate();
    SysTick_Config( SystemCoreClock /1000);
		PB->PUEN|=BIT2;
//            aa,   us, rt, ch, bps, dbm, pw, nak
  short cfg[]={1, 1000,  3,  5, 250, -12, 32,  0};
	nrf_init(cfg);
}
//-------------- SW2 ------------
char sta=3;
void sw2(void){
	sta <<=1;
	sta +=PB2;
	sta &=3;
	if(sta==2){
	}	
}


//-------------- HCLK  -------------

int i;
uint32_t ax,ay,az,gx,gy,gz;
uint32_t MPU6050data[14];
char OutLcd[32];
float D;
float oldD;
float dif;

void MPU6050_init(void);
int MPU6050_test(void);

int main(void){
  init();
	delay_ms(10);
	MPU6050_init();
	
  while(1){
		
	{	
		delay_ms(10);
		char rb[32]={0x2C,0x00};
		short ln;
		//delay_ms(1000);
		ln = MPU6050_test();
		if(ln==1){ nrf_output(rb); }
		//ln=nrf_output(rb);
	}
  }  //while
}  //main
