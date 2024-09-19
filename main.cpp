#define SYSTEM_TIME_INCREMENT_MS   10
#define SYSTEM_TIME_DISPLAY_REFRESH 200

#include "mbed.h"
#include "wifi.h"
#include "arm_book_lib.h"
#include "fire_alarm.h"
#include "pc_serial_com.h"
#include "non_blocking_delay.h"
#include "botones.h"
#include "display.h"
#include "lectura_temp.h"
#include "temperature_sensor.h"

//=====[Main function, the program entry point after power on or reset]========
static nonBlockingDelay_t smartHomeSystemDelay;
static nonBlockingDelay_t displayRefresh;

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


void userInterfaceDisplayInit()
{
    displayInit( DISPLAY_TYPE_LCD_HD44780, DISPLAY_CONNECTION_I2C_PCF8574_IO_EXPANDER);
    //displayModeWrite( DISPLAY_MODE_CHAR );
    displayCharPositionWrite ( 0,0 );
    displayStringWrite( "Temperatura:" );
}

void userInterfaceDisplayReportStateUpdate()
{
    char temperatureString[3] = "";

    int temperature = (int)(temperatureSensorReadCelsius() * 100);
    sprintf(temperatureString, "%d", (temperature / 100));
    
    displayCharPositionWrite ( 12,0 );
    displayStringWrite( temperatureString );
    displayCharPositionWrite ( 14,0 );
    displayStringWrite( "'C" );
}


void smartHomeSystemInit()
{
    tickInit();
    fireAlarmInit();
    pcSerialComInit();
    wifiComInit();
    nonBlockingDelayInit(&smartHomeSystemDelay, SYSTEM_TIME_INCREMENT_MS);
    nonBlockingDelayInit(&displayRefresh, SYSTEM_TIME_DISPLAY_REFRESH);
    chequeo_botones_presionados();      //Función que contiene la asociación de las funciones de
                                        //chequeo de botones presionados con la función Ticker.
    userInterfaceDisplayInit();
}

void smartHomeSystemUpdate()
{
    if( nonBlockingDelayRead(&smartHomeSystemDelay) ) 
    {
        fireAlarmUpdate();
        pcSerialComUpdate();
        control_invernadero();          //Función que permite cambiar de un estado a otro en función
    }  
    if(nonBlockingDelayRead(&displayRefresh))
    {
        userInterfaceDisplayReportStateUpdate();
    }
    wifiComUpdate();  
}