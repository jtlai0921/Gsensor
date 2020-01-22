#include "Nano100Series.h"

void i2c_start(void);
void i2c_stop(void);
uint8_t i2c_write(uint8_t d);
uint8_t i2c_read(uint8_t ack);

#pragma anon_unions
//  SCL -- PF5
//  SDA -- PF4
// ADDR -- PB8
 
#define     SCL     PF5
#define     SDA     PF4
#define     Din     PB8
 
#define     DIR_OUT(p, n)       p->PMD = (p->PMD & ~(3<<(n<<1)))|(1<<(n<<1))
#define     DIR_IN(p,n)         p->PMD = (p->PMD & ~(3<<(n<<1)))
#define     DIR_OD(p,n)         p->PMD = (p->PMD & ~(3<<(n<<1)))|(2<<(n<<1))
 
typedef union{
    unsigned char v;
    struct{
        unsigned b0:1;
        unsigned b1:1;
        unsigned b2:1;
        unsigned b3:1;
        unsigned b4:1;
        unsigned b5:1;
        unsigned b6:1;
        unsigned b7:1;
    };
}TPU8;
 
//---
static void delay(int n){
    int i;
    for(i=0;i < n;i++){}
}
void i2c_start(void){
    DIR_OD(PF,4); DIR_OD(PF,5);
    PF->PUEN |=  BIT4+BIT5;
 
    if(SDA==0){
        SCL=0; delay(50);
    }
    SDA=1; delay(50);
    SCL=1; delay(50);
    SDA=0; delay(50);
    SCL=0; delay(50);
}
 
void i2c_stop(void){
    SDA=0;  delay(50);
    SCL=1;  delay(50);
    SDA=1;  delay(50);
}
 
uint8_t i2c_write(uint8_t d){
    TPU8 r;
    r.v = d;
    SDA=r.b7; delay(0); SCL=1; delay(0); SCL=0;
    SDA=r.b6; delay(0); SCL=1; delay(0); SCL=0;
    SDA=r.b5; delay(0); SCL=1; delay(0); SCL=0;
    SDA=r.b4; delay(0); SCL=1; delay(0); SCL=0;
    SDA=r.b3; delay(0); SCL=1; delay(0); SCL=0;
    SDA=r.b2; delay(0); SCL=1; delay(0); SCL=0;
    SDA=r.b1; delay(0); SCL=1; delay(0); SCL=0;
    SDA=r.b0; delay(0); SCL=1; delay(0); SCL=0;
    //--- ACK
    SDA=1; delay(20); SCL=1; delay(20); r.v=Din; SCL=0;
     
    return r.v;
}
 
uint8_t i2c_read(uint8_t ack){
    TPU8 r;
    SDA=1;
    delay(0); SCL=1; delay(0); r.b7 = SDA; SCL=0;
    delay(0); SCL=1; delay(0); r.b6 = SDA; SCL=0;
    delay(0); SCL=1; delay(0); r.b5 = SDA; SCL=0;
    delay(0); SCL=1; delay(0); r.b4 = SDA; SCL=0;
    delay(0); SCL=1; delay(0); r.b3 = SDA; SCL=0;
    delay(0); SCL=1; delay(0); r.b2 = SDA; SCL=0;
    delay(0); SCL=1; delay(0); r.b1 = SDA; SCL=0;
    delay(0); SCL=1; delay(0); r.b0 = SDA; SCL=0;
    //--- ACK
    SDA = ack; delay(20); SCL=1; delay(20); SCL=0;
     
    SDA=1;
    return r.v;
}
//---


#include "Nano100Series.h"
#include <stdio.h>
#pragma anon_unions
 
//-- ???? I2C.c
 
void i2c_start(void);
void i2c_stop(void);
uint8_t i2c_write(uint8_t d);
uint8_t i2c_read(uint8_t ack);
 
#define   ACK    0
#define   NACK   1
#define   RD     1
#define   WR     0
 
//---------------- MPU6050 -------------------
int MPU6050_setreg(uint8_t addr, uint8_t val){
    uint8_t r;
    i2c_start();
    r = i2c_write(0xD0+WR);
    r+= i2c_write(addr);
    r+= i2c_write(val);
    i2c_stop();
    return r;
}
 
void MPU6050_getreg(int addr, short *dst, int ln){
    int i;
    union {
        short v;
        struct{ uint8_t L; uint8_t H; };
    }r;
    i2c_start();
        r.v  = i2c_write(0xD0+WR);
        r.v += i2c_write(addr);
    i2c_start();
        r.v  = i2c_write(0xD0+RD);
        for(i=1;i < ln;i++){
            r.H = i2c_read(ACK); r.L = i2c_read(ACK);
            *dst++ = r.v;
        }
        r.H = i2c_read(ACK); r.L = i2c_read(NACK);
        *dst++ = r.v;
    i2c_stop();
}
void MPU6050_init(void){
    MPU6050_setreg(0x6B, 0x00);
    MPU6050_setreg(0x19, 0x07);
    MPU6050_setreg(0x1A, 0x06);
    MPU6050_setreg(0x1B, 0x18);
    MPU6050_setreg(0x1C, 0x01);
}
//---
#include "math.h"
short mp[7];
float x,y,z,d;
float ar[24];
int ai=0;
float sum;
char msta=0;
int MPU6050_test(void){
    //---
    MPU6050_getreg(0x3B, mp, 7);
	x=mp[4];
	y=mp[5];
	z=mp[6];
	d = x*x+y*y+z*z;
	d = sqrt(d);
	ar[ai++]=d;
	if(ai>23){ ai=0; }
	{
		int i;
		for(i=0;i<24;i++){ sum += ar[i];}
		sum /= 24;
	}
	{
		msta <<=1;
		msta += sum>1000;
		msta &= 3;
	}
	printf("%d %d\n", (int)d, (int)sum);
	return msta;
    printf("AX:%d AY:%d AZ:%d\n",mp[0],mp[1],mp[2]);
    printf("Temp:%d\n",mp[3]);
    printf("GX:%d GY:%d GZ:%d\n",mp[4],mp[5],mp[6]);
	return 1;
}
 