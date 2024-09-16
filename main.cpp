#define SYSTEM_TIME_INCREMENT_MS   10

#include "mbed.h"
#include "wifi.h"
#include "arm_book_lib.h"
#include "fire_alarm.h"
#include "pc_serial_com.h"
#include "non_blocking_delay.h"

//=====[Main function, the program entry point after power on or reset]========
static nonBlockingDelay_t smartHomeSystemDelay;

void smartHomeSystemInit();
void smartHomeSystemUpdate();

int main()
{
    smartHomeSystemInit();
    while (true) 
    {
        smartHomeSystemUpdate();
    }
}

void smartHomeSystemInit()
{
    tickInit();
    fireAlarmInit();
    pcSerialComInit();
    wifiComInit();
    nonBlockingDelayInit(&smartHomeSystemDelay, SYSTEM_TIME_INCREMENT_MS );
}

void smartHomeSystemUpdate()
{
    if( nonBlockingDelayRead(&smartHomeSystemDelay) ) 
    {
        fireAlarmUpdate();
        pcSerialComUpdate();
    }    
    wifiComUpdate();
}
