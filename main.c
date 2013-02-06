#include "launchlib.h"

#define RAM_BACKUP_LOC 0xE008
#define RAM_LOC 0x1C00
#define RAM_SIZE 1024
#define SFR_BACKUP_LOC (RAM_BACKUP_LOC + RAM_SIZE)
#define SFR_LOC 0x0200
#define SFR_SIZE 0x013F

__no_init volatile uint8_t* f_from    @ 0xE002;
__no_init volatile uint8_t* f_to      @ 0xE004;
__no_init volatile size_t   f_mem_cnt @ 0xE006;
__no_init volatile uint8_t  f_asleep  @ 0xE000;

void HardwareInit(void)
{
    IO_DIRECTION(SW1,INPUT);
    IO_DIRECTION(SW2,INPUT);
    IO_PULL(SW1,PULL_UP);
    IO_PULL(SW2,PULL_UP);
    IO_DIRECTION(LED1,OUTPUT);
    IO_DIRECTION(LED2,OUTPUT);
    IO_DIRECTION(LED3,OUTPUT);
    IO_DIRECTION(LED4,OUTPUT);
    IO_DIRECTION(LED5,OUTPUT);
    IO_DIRECTION(LED6,OUTPUT);
    IO_DIRECTION(LED7,OUTPUT);
    IO_DIRECTION(LED8,OUTPUT);

    LED_OFF(1);
    LED_OFF(2);
    LED_OFF(3);
    LED_OFF(4);
    LED_OFF(5);
    LED_OFF(6);
    LED_OFF(7);
    LED_OFF(8);
}

void Hibernate(void)
{
    // save sram
    memcpy((uint16_t*)RAM_BACKUP_LOC,(uint16_t*)RAM_LOC,RAM_SIZE);
    // save sfr
    memcpy((uint16_t*)SFR_BACKUP_LOC,(uint16_t*)SFR_LOC,SFR_SIZE);
    f_asleep = TRUE;
    while(1);
}

void Restore(void)
{
    f_asleep = FALSE;
    // restore sram
    f_to = (uint8_t*)RAM_LOC;
    f_from = (uint8_t*)RAM_BACKUP_LOC;
    f_mem_cnt = RAM_SIZE;
    while(--f_mem_cnt != 0)
    {
        *f_to++ = *f_from++;
    }
    // restore SFR
    f_to = (uint8_t*)SFR_LOC;
    f_from = (uint8_t*)SFR_BACKUP_LOC;
    f_mem_cnt = SFR_SIZE;
    while(--f_mem_cnt != 0)
    {
        *f_to++ = *f_from++;
    }
}

void main(void)
{
    WD_STOP();

    HardwareInit();
    ClockConfig(16);
    ScheduleInit();
    InterruptAttach(SW1_PORT,SW1_PIN,Hibernate,FALLING_EDGE);
    _EINT();

    if (f_asleep == TRUE)
    {
        Restore();
    }

    while(1)
    {
        LED_TOGGLE(1);
        Delay(_500_MILLISECOND);
        LED_TOGGLE(2);
        Delay(_500_MILLISECOND);
        LED_TOGGLE(3);
        Delay(_500_MILLISECOND);
        LED_TOGGLE(4);
        Delay(_500_MILLISECOND);
        LED_TOGGLE(5);
        Delay(_500_MILLISECOND);
        LED_TOGGLE(6);
        Delay(_500_MILLISECOND);
        LED_TOGGLE(7);
        Delay(_500_MILLISECOND);
        LED_TOGGLE(8);
        Delay(_500_MILLISECOND);
    }
}
