//=====[Libraries]=============================================================

#include "mbed.h"
#include "arm_book_lib.h"
#include "pc_serial_com.h"
#include "temperature_sensor.h"
#include "wifi.h"
#include "fire_alarm.h"

//=====[Declaration of private defines]========================================

//=====[Declaration of private data types]=====================================

typedef enum{
    PC_SERIAL_GET_FILE_NAME,
    PC_SERIAL_COMMANDS,
    PC_SERIAL_GET_CODE,
    PC_SERIAL_SAVE_NEW_CODE,
    PC_SERIAL_GET_WIFI_AP_CREDENTIALS_SSID,
    PC_SERIAL_GET_WIFI_AP_CREDENTIALS_PASSWORD,
} pcSerialComMode_t;

//=====[Declaration and initialization of public global objects]===============

UnbufferedSerial uartUsb(USBTX, USBRX, 115200);

//=====[Declaration of external public global variables]=======================

//=====[Declaration and initialization of public global variables]=============


//=====[Declaration and initialization of private global variables]============

static pcSerialComMode_t pcSerialComMode = PC_SERIAL_COMMANDS;
static bool codeComplete = false;
static int numberOfCodeChars = 0;
static int numberOfCharsInFileName = 0;
static int numberOfCharsInApCredentials = 0;

static char ApSsid[AP_SSID_MAX_LENGTH] = "";
static char ApPassword[AP_PASSWORD_MAX_LENGTH] = "";

//=====[Declarations (prototypes) of private functions]========================

static void pcSerialComCharWrite( char chr );
static void pcSerialComStringRead( char* str, int strLength );
static void pcSerialComGetWiFiComApSsid( char receivedChar );
static void pcSerialComGetWiFiComApPassword( char receivedChar );
static void pcSerialComCommandUpdate( char receivedChar );
static void availableCommands();
static void commandShowCurrentOverTemperatureDetectorState();
static void commandShowCurrentTemperatureInCelsius();
static void commandRestartWifiCom();
static void commandSetWifiComApSsid();
static void commandSetWifiComApPassword();
static void commandGetWifiComAssignedIp();

//=====[Implementations of public functions]===================================

void pcSerialComInit()
{
    availableCommands();
}

char pcSerialComCharRead()
{
    char receivedChar = '\0';
    if( uartUsb.readable() ) {
        uartUsb.read( &receivedChar, 1 );
    }
    return receivedChar;
}

void pcSerialComStringWrite( const char* str )
{
    uartUsb.write( str, len(str) );
}

void pcSerialComIntWrite( int number )
{
    char str[4] = "";
    sprintf( str, "%d", number );
    pcSerialComStringWrite( str );
}

void pcSerialComUpdate()
{
    char receivedChar = pcSerialComCharRead();
    if( receivedChar != '\0' ) {
        switch ( pcSerialComMode ) 
        {
            case PC_SERIAL_COMMANDS:
                pcSerialComCommandUpdate( receivedChar );
            break;
            case PC_SERIAL_GET_WIFI_AP_CREDENTIALS_SSID:
                pcSerialComGetWiFiComApSsid( receivedChar );
            break;
            case PC_SERIAL_GET_WIFI_AP_CREDENTIALS_PASSWORD:
                pcSerialComGetWiFiComApPassword( receivedChar );
            break;
            default:
                pcSerialComMode = PC_SERIAL_COMMANDS;
            break;
        }
    }    
}

//=====[Implementations of private functions]==================================

static void pcSerialComCharWrite( char chr )
{
    char str[2] = "";
    sprintf (str, "%c", chr);
    uartUsb.write( str, len(str) );
}

static void pcSerialComStringRead( char* str, int strLength )
{
    int strIndex;
    for ( strIndex = 0; strIndex < strLength; strIndex++) {
        uartUsb.read( &str[strIndex] , 1 );
        uartUsb.write( &str[strIndex] ,1 );
    }
    str[strLength]='\0';
}

static void pcSerialComCommandUpdate( char receivedChar )
{
    switch (receivedChar) {
        case '3': commandShowCurrentOverTemperatureDetectorState(); break;
        case 'c': case 'C': commandShowCurrentTemperatureInCelsius(); break;
        case 'a': case 'A': commandRestartWifiCom(); break;
        case 'd': case 'D': commandSetWifiComApSsid(); break;
        case 'r': case 'R': commandSetWifiComApPassword(); break;
        case 'p': case 'P': commandGetWifiComAssignedIp(); break;
        default: availableCommands(); break;
    } 
}

static void availableCommands()
{
    pcSerialComStringWrite( "Available commands:\r\n" );
    pcSerialComStringWrite( "Press '3' to get the over temperature detector state\r\n" );
    pcSerialComStringWrite( "Press 'c' or 'C' to get lm35 reading in Celsius\r\n" );
    pcSerialComStringWrite( "Press 'a' or 'A' to restart the Wi-Fi communication\r\n" );
    pcSerialComStringWrite( "Press 'd' or 'D' to set Wi-Fi AP SSID\r\n" );
    pcSerialComStringWrite( "Press 'r' or 'R' to set Wi-Fi AP Password\r\n" );
    pcSerialComStringWrite( "Press 'p' or 'P' to get Wi-Fi assigned IP\r\n" );
    pcSerialComStringWrite( "\r\n" );
}

static void commandShowCurrentOverTemperatureDetectorState()
{
    if ( overTemperatureDetectorStateRead() ) { 
        pcSerialComStringWrite( "Temperature is above the maximum level\r\n");
    } else {
        pcSerialComStringWrite( "Temperature is below the maximum level\r\n");
    }
}

static void commandShowCurrentTemperatureInCelsius()
{
    char str[100] = "";
    sprintf ( str, "Temperature: %.2f \xB0 C\r\n",
                    temperatureSensorReadCelsius() );
    pcSerialComStringWrite( str );  
}

static void commandRestartWifiCom()
{
    pcSerialComStringWrite( "Wi-Fi communication restarted \r\n" );
    wifiComRestart();
}

static void commandSetWifiComApSsid()
{
    pcSerialComStringWrite("\r\nPlease provide the SSID of the Wi-Fi "); 
    pcSerialComStringWrite("Access Point and press the Enter key\r\n");
    pcSerialComStringWrite("> ");
    pcSerialComMode = PC_SERIAL_GET_WIFI_AP_CREDENTIALS_SSID;
    numberOfCharsInApCredentials = 0;
}

static void commandSetWifiComApPassword()
{
    pcSerialComStringWrite("\r\nPlease provide the Password of the Wi-Fi "); 
    pcSerialComStringWrite("Access Point and press the Enter key\r\n");
    pcSerialComStringWrite("> ");
    pcSerialComMode = PC_SERIAL_GET_WIFI_AP_CREDENTIALS_PASSWORD;
    numberOfCharsInApCredentials = 0;
}

static void commandGetWifiComAssignedIp()
{
    pcSerialComStringWrite( "The assigned IP is: " );
    pcSerialComStringWrite( wifiComGetIpAddress() );
    pcSerialComStringWrite( "\r\n" );
}

static void pcSerialComGetWiFiComApSsid( char receivedChar )
{
    if ( (receivedChar == '\r') && 
         (numberOfCharsInApCredentials < AP_SSID_MAX_LENGTH) ) {
        pcSerialComMode = PC_SERIAL_COMMANDS;
        ApSsid[numberOfCharsInApCredentials] = '\0';
        wifiComSetWiFiComApSsid(ApSsid);
        pcSerialComStringWrite( "\r\nWi-Fi Access Point SSID configured\r\n\r\n" );
    } else {
        ApSsid[numberOfCharsInApCredentials] = receivedChar;
        pcSerialComCharWrite( receivedChar );
        numberOfCharsInApCredentials++;
    }
}

static void pcSerialComGetWiFiComApPassword( char receivedChar )
{
    if ( (receivedChar == '\r') && 
         (numberOfCharsInApCredentials < AP_PASSWORD_MAX_LENGTH) ) {
        pcSerialComMode = PC_SERIAL_COMMANDS;
        ApPassword[numberOfCharsInApCredentials] = '\0';
        wifiComSetWiFiComApPassword(ApPassword);
        pcSerialComStringWrite( "\r\nWi-Fi Access Point password configured\r\n\r\n" );
    } else {
        ApPassword[numberOfCharsInApCredentials] = receivedChar;
        pcSerialComStringWrite( "*" );
        numberOfCharsInApCredentials++;
    }
}