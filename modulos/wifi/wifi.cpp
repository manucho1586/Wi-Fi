#//=====[Libraries]=============================================================

#include "arm_book_lib.h"

#include "wifi.h"

#include "non_blocking_delay.h"
#include "pc_serial_com.h"
#include "temperature_sensor.h"
#include "fire_alarm.h"

//=====[Declaration of private defines]========================================

#define DELAY_10_SECONDS        4000
#define DELAY_5_SECONDS         2000
#define DELAY_15_SECONDS        30000

#define BEGIN_USER_LINE   "<p>"
#define END_USER_LINE     "</p>"

#define IP_MAX_LENGTH (15 + 1)

//=====[Declaration of private data types]=====================================

typedef enum {
   WIFI_STATE_INIT,
   WIFI_STATE_SEND_AT,
   WIFI_STATE_WAIT_AT,
   WIFI_STATE_SEND_CWMODE,
   WIFI_STATE_WAIT_CWMODE,
   WIFI_STATE_SEND_CWJAP_IS_SET,
   WIFI_STATE_WAIT_CWJAP_IS_SET,
   WIFI_STATE_SEND_CWJAP_SET,
   WIFI_STATE_WAIT_CWJAP_SET_1,
   WIFI_STATE_WAIT_CWJAP_SET_2,
   WIFI_STATE_SEND_CIFSR,
   WIFI_STATE_WAIT_CIFSR,
   WIFI_STATE_LOAD_IP,
   WIFI_STATE_SEND_CIPMUX,
   WIFI_STATE_WAIT_CIPMUX,
   WIFI_STATE_SEND_CIPSERVER,
   WIFI_STATE_WAIT_CIPSERVER,
   WIFI_STATE_SEND_CIPSTATUS,
   WIFI_STATE_WAIT_CIPSTATUS_STATUS_3,
   WIFI_STATE_WAIT_CIPSTATUS,
   WIFI_STATE_WAIT_GET_ID,
   WIFI_STATE_WAIT_CIPSTATUS_OK,
   WIFI_STATE_SEND_CIPSEND,
   WIFI_STATE_WAIT_CIPSEND,
   WIFI_STATE_SEND_HTML,
   WIFI_STATE_WAIT_HTML,
   WIFI_STATE_SEND_CIPCLOSE,
   WIFI_STATE_WAIT_CIPCLOSE,
   WIFI_STATE_IDLE,
   WIFI_STATE_ERROR
} wifiComState_t;

//=====[Declaration and initialization of public global objects]===============

UnbufferedSerial uartWifi( PA_11, PA_12, 115200 );

//=====[Declaration of external public global variables]=======================

//=====[Declaration and initialization of public global variables]=============

//=====[Declaration and initialization of private global variables]============

static const char responseOk[] = "OK";
static const char responseCwjapOk[] = "+CWJAP:";
static const char responseCwjap1[] = "WIFI CONNECTED";
static const char responseCwjap2[] = "WIFI GOT IP";
static const char responseCifsr[] = "+CIFSR:STAIP,\"";
static const char responseStatus3[] = "STATUS:3";
static const char responseCipstatus[] = "+CIPSTATUS:";
static const char responseSendOk[] = "SEND OK";
static const char responseCipclose[] = "CLOSED";
DigitalOut verif(D7);

static int currentConnectionId;
static char wifiComApSsid[AP_SSID_MAX_LENGTH] = "";
static char wifiComApPassword[AP_PASSWORD_MAX_LENGTH] = "";
static char wifiComIpAddress[IP_MAX_LENGTH];
static char stateString[4] = "";

static const char* wifiComExpectedResponse;
static wifiComState_t wifiComState;

static nonBlockingDelay_t wifiComDelay;

static const char htmlCodeHeader [] =
   "<!doctype html>"
   "<html> <head> <title>Sistema de control de invernadero</title> </head>"
   "<body style=\"text-align: center;\">"
   "<h1 style=\"color: #0000ff;\">Sistema de control de invernadero</h1>"
   "<div style=\"font-weight: bold\">"
   ;

static const char htmlCodeFooter [] = "</div> </body> </html>";

static char htmlCodeBody[450] = "";

//=====[Declarations (prototypes) of private functions]========================

static bool isExpectedResponse();
bool wifiComCharRead( char* receivedChar );
void wifiComStringWrite( const char* str );
void wifiComWebPageDataUpdate();
char * stateToString( bool state );

//=====[Implementations of public functions]===================================

void wifiComSetWiFiComApSsid( char * ApSsid )
{
    strncpy(wifiComApSsid, ApSsid, AP_SSID_MAX_LENGTH);
}

void wifiComSetWiFiComApPassword( char * ApPassword )
{
    strncpy(wifiComApPassword, ApPassword, AP_PASSWORD_MAX_LENGTH );
}

char * wifiComGetIpAddress()
{
   return wifiComIpAddress;
}

void wifiComRestart()
{
    wifiComState = WIFI_STATE_INIT;
}

void wifiComInit()
{
    wifiComState = WIFI_STATE_INIT;
}

void wifiComUpdate()
{
   int lengthOfHtmlCode;
   static char receivedCharWifiCom;
   static int IpStringPositionIndex;
   char strToSend[50] = "";

   switch (wifiComState) {

      case WIFI_STATE_INIT:
         nonBlockingDelayWrite(&wifiComDelay, DELAY_5_SECONDS);
         wifiComState = WIFI_STATE_SEND_AT;
         verif=HIGH;
      break;

      case WIFI_STATE_SEND_AT:
         if (nonBlockingDelayRead(&wifiComDelay)) {
            wifiComStringWrite( "AT\r\n" );
            wifiComExpectedResponse = responseOk;
            nonBlockingDelayWrite(&wifiComDelay, DELAY_5_SECONDS);
            wifiComState = WIFI_STATE_WAIT_AT;
         }
      break;

      case WIFI_STATE_WAIT_AT:
         if (isExpectedResponse()) {
            nonBlockingDelayWrite(&wifiComDelay, DELAY_5_SECONDS);
            wifiComState = WIFI_STATE_SEND_CWMODE;
         }
         if (nonBlockingDelayRead(&wifiComDelay)) {
            pcSerialComStringWrite("AT command not responded ");
            pcSerialComStringWrite("correctly\r\n");
            wifiComState = WIFI_STATE_ERROR;
         }
      break;

      case WIFI_STATE_SEND_CWMODE:
         if (nonBlockingDelayRead(&wifiComDelay)) {
            wifiComStringWrite( "AT+CWMODE=1\r\n" );
            wifiComExpectedResponse = responseOk;
            nonBlockingDelayWrite(&wifiComDelay, DELAY_5_SECONDS);
            wifiComState = WIFI_STATE_WAIT_CWMODE;
         }
      break;

      case WIFI_STATE_WAIT_CWMODE:
         if (isExpectedResponse()) {
            nonBlockingDelayWrite(&wifiComDelay, DELAY_5_SECONDS);
            wifiComState = WIFI_STATE_SEND_CWJAP_IS_SET;
         }
         if (nonBlockingDelayRead(&wifiComDelay)) {
            pcSerialComStringWrite("AT+CWMODE=1 command not ");
            pcSerialComStringWrite("responded correctly\r\n");
            wifiComState = WIFI_STATE_ERROR;
         }
      break;

      case WIFI_STATE_SEND_CWJAP_IS_SET:
         if (nonBlockingDelayRead(&wifiComDelay)) {
            wifiComStringWrite( "AT+CWJAP?\r\n" );
            wifiComExpectedResponse = responseCwjapOk;
            nonBlockingDelayWrite(&wifiComDelay, DELAY_5_SECONDS);
            wifiComState = WIFI_STATE_WAIT_CWJAP_IS_SET;
            verif=LOW;
         }
      break;

      case WIFI_STATE_WAIT_CWJAP_IS_SET:
         if (isExpectedResponse()) {
            wifiComExpectedResponse = responseOk;
            wifiComState = WIFI_STATE_SEND_CIFSR;
         }
         if (nonBlockingDelayRead(&wifiComDelay)) {
            nonBlockingDelayWrite(&wifiComDelay, DELAY_5_SECONDS);
            wifiComState = WIFI_STATE_SEND_CWJAP_SET;
         }
      break;

      case WIFI_STATE_SEND_CWJAP_SET:
         if (nonBlockingDelayRead(&wifiComDelay)) {
            wifiComStringWrite( "AT+CWJAP=\"" );
            wifiComStringWrite( "Santi 2.4 Ghz");
            wifiComStringWrite( "\",\"" );
            wifiComStringWrite( "tangoyfrida");
            wifiComStringWrite( "\"" );
            wifiComStringWrite( "\r\n" );
            wifiComExpectedResponse = responseCwjap1;
            nonBlockingDelayWrite(&wifiComDelay, DELAY_10_SECONDS);
            wifiComState = WIFI_STATE_WAIT_CWJAP_SET_1;
         }
      break;

      case WIFI_STATE_WAIT_CWJAP_SET_1:
         if (isExpectedResponse()) {
            wifiComExpectedResponse = responseCwjap2;
            wifiComState = WIFI_STATE_WAIT_CWJAP_SET_2;
         }
         if (nonBlockingDelayRead(&wifiComDelay)) 
         {
            pcSerialComStringWrite("Error in state: ");
            pcSerialComStringWrite("WIFI_STATE_WAIT_CWJAP_SET_1\r\n");
            pcSerialComStringWrite("Check Wi-Fi AP credentials ");
            pcSerialComStringWrite("and restart\r\n");
            wifiComState = WIFI_STATE_ERROR;
         }
         break;

      case WIFI_STATE_WAIT_CWJAP_SET_2:
         if (isExpectedResponse()) {
            wifiComState = WIFI_STATE_SEND_CIFSR;
         }
         if (nonBlockingDelayRead(&wifiComDelay)) {
            pcSerialComStringWrite("Error in state: ");
            pcSerialComStringWrite("WIFI_STATE_WAIT_CWJAP_SET_2\r\n");
            pcSerialComStringWrite("Check Wi-Fi AP credentials ");
            pcSerialComStringWrite("and restart\r\n");
            wifiComState = WIFI_STATE_ERROR;
         }
      break;

      case WIFI_STATE_SEND_CIFSR:
         if (nonBlockingDelayRead(&wifiComDelay)) {
            wifiComStringWrite( "AT+CIFSR\r\n" );
            wifiComExpectedResponse = responseCifsr;
            nonBlockingDelayWrite(&wifiComDelay, DELAY_5_SECONDS);
            wifiComState = WIFI_STATE_WAIT_CIFSR;
         }
      break;

      case WIFI_STATE_WAIT_CIFSR:
         if (isExpectedResponse()) {
            wifiComState = WIFI_STATE_LOAD_IP;
            IpStringPositionIndex = 0;
         }
         if (nonBlockingDelayRead(&wifiComDelay)) {
            pcSerialComStringWrite("AT+CIFSR command not responded ");
            pcSerialComStringWrite("correctly\r\n");
            wifiComState = WIFI_STATE_ERROR;
         }
      break;

      case WIFI_STATE_LOAD_IP:
         if (wifiComCharRead(&receivedCharWifiCom)) {
            if ( (receivedCharWifiCom != '"') && 
               (IpStringPositionIndex < IP_MAX_LENGTH) ) {
               wifiComIpAddress[IpStringPositionIndex] = receivedCharWifiCom;
               IpStringPositionIndex++;
            } else {
               wifiComIpAddress[IpStringPositionIndex] = '\0';
               pcSerialComStringWrite("IP address assigned correctly\r\n\r\n");
               wifiComState = WIFI_STATE_SEND_CIPMUX;
            }
         }
      break;

      case WIFI_STATE_SEND_CIPMUX:
         if (nonBlockingDelayRead(&wifiComDelay)) {
            wifiComStringWrite( "AT+CIPMUX=1\r\n" );
            wifiComExpectedResponse = responseOk;
            nonBlockingDelayWrite(&wifiComDelay, DELAY_5_SECONDS);
            wifiComState = WIFI_STATE_WAIT_CIPMUX;
         }
         
      break;

      case WIFI_STATE_WAIT_CIPMUX:
         if (isExpectedResponse()) 
         {
            nonBlockingDelayWrite(&wifiComDelay, DELAY_5_SECONDS);
            wifiComState = WIFI_STATE_SEND_CIPSERVER;
         }
         if (nonBlockingDelayRead(&wifiComDelay)) {
            pcSerialComStringWrite("AT+CIPMUX=1 command not ");
            pcSerialComStringWrite("responded correctly\r\n");
            wifiComState = WIFI_STATE_ERROR;
         }
      break;

      case WIFI_STATE_SEND_CIPSERVER:
         if (nonBlockingDelayRead(&wifiComDelay)) 
         {
            wifiComStringWrite( "AT+CIPSERVER=1,80\r\n" );
            wifiComExpectedResponse = responseOk;
            nonBlockingDelayWrite(&wifiComDelay, DELAY_5_SECONDS);
            wifiComState = WIFI_STATE_WAIT_CIPSERVER;
         }
      break;

      case WIFI_STATE_WAIT_CIPSERVER:
         if (isExpectedResponse()) {
            nonBlockingDelayWrite(&wifiComDelay, DELAY_5_SECONDS);
            wifiComState = WIFI_STATE_SEND_CIPSTATUS;
         }
         if (nonBlockingDelayRead(&wifiComDelay)) {
         pcSerialComStringWrite("AT+CIPSERVER=1,80 command not ");
         pcSerialComStringWrite("responded correctly\r\n");
         wifiComState = WIFI_STATE_ERROR;
         }
      break;

      case WIFI_STATE_SEND_CIPSTATUS:
         if (nonBlockingDelayRead(&wifiComDelay)) 
         {
            wifiComStringWrite( "AT+CIPSTATUS\r\n" );
            wifiComExpectedResponse = responseStatus3;
            nonBlockingDelayWrite(&wifiComDelay, DELAY_5_SECONDS);
            wifiComState = WIFI_STATE_WAIT_CIPSTATUS_STATUS_3;
         }
      break;

      case WIFI_STATE_WAIT_CIPSTATUS_STATUS_3:
         if (isExpectedResponse()) //Aca entra cuando hay una solicitud de ingreso a la dir IP del web server del esp y ahi sigue hasta donde pongo donde no entra.
         {
            nonBlockingDelayWrite(&wifiComDelay, DELAY_5_SECONDS);
            wifiComExpectedResponse = responseCipstatus;
            wifiComState = WIFI_STATE_WAIT_CIPSTATUS;
         }
         if (nonBlockingDelayRead(&wifiComDelay))
         {
            nonBlockingDelayWrite(&wifiComDelay, DELAY_5_SECONDS);
            wifiComState = WIFI_STATE_SEND_CIPSTATUS;
         }
      break;

      case WIFI_STATE_WAIT_CIPSTATUS:
         if (isExpectedResponse()) 
         {
            wifiComState = WIFI_STATE_WAIT_GET_ID;
         }
         if (nonBlockingDelayRead(&wifiComDelay)) {
            nonBlockingDelayWrite(&wifiComDelay, DELAY_5_SECONDS);
            wifiComState = WIFI_STATE_SEND_CIPSTATUS;
         }
      break;

      case WIFI_STATE_WAIT_GET_ID:
         if( wifiComCharRead(&receivedCharWifiCom))
         {
            currentConnectionId = receivedCharWifiCom;
            wifiComExpectedResponse = responseOk;
            wifiComState = WIFI_STATE_WAIT_CIPSTATUS_OK;
         }
      break;

      case WIFI_STATE_WAIT_CIPSTATUS_OK:
         if (isExpectedResponse()) 
         {
            wifiComState = WIFI_STATE_SEND_CIPSEND;
            wifiComWebPageDataUpdate();
            
         }
         if (nonBlockingDelayRead(&wifiComDelay)) 
         {
            nonBlockingDelayWrite(&wifiComDelay, DELAY_5_SECONDS);
            wifiComState = WIFI_STATE_SEND_CIPSTATUS;
         }
      break;

        case WIFI_STATE_SEND_CIPSEND: //No entra aca, es decir no enciende el LED
        verif=HIGH;
        lengthOfHtmlCode = ( len(htmlCodeHeader) 
                               + len(htmlCodeBody) 
                               + len(htmlCodeFooter) );
        sprintf( strToSend, "AT+CIPSEND=%c,%d\r\n", 
                  currentConnectionId, lengthOfHtmlCode );
        wifiComStringWrite( strToSend );
        wifiComState = WIFI_STATE_WAIT_CIPSEND;
        wifiComExpectedResponse = responseOk;
        break;

      case WIFI_STATE_WAIT_CIPSEND:
         if (isExpectedResponse()) {
            nonBlockingDelayWrite(&wifiComDelay, DELAY_15_SECONDS);  //DELAY_15_SECONDS
            wifiComState = WIFI_STATE_SEND_HTML;
         }
         if (nonBlockingDelayRead(&wifiComDelay)) {
            nonBlockingDelayWrite(&wifiComDelay, DELAY_5_SECONDS);
            wifiComState = WIFI_STATE_SEND_CIPSTATUS;
         }
      break;

      case WIFI_STATE_SEND_HTML:
        wifiComStringWrite( htmlCodeHeader );
        wifiComStringWrite( htmlCodeBody );
        wifiComStringWrite( htmlCodeFooter );
        wifiComState = WIFI_STATE_WAIT_HTML;
        wifiComExpectedResponse = responseSendOk;
      break;

      case WIFI_STATE_WAIT_HTML:
         if (isExpectedResponse()) 
         {
            nonBlockingDelayWrite(&wifiComDelay, DELAY_5_SECONDS);
            wifiComState = WIFI_STATE_SEND_CIPCLOSE;
            
         }
         if (nonBlockingDelayRead(&wifiComDelay)) {
            nonBlockingDelayWrite(&wifiComDelay, DELAY_5_SECONDS);
            wifiComState = WIFI_STATE_SEND_CIPSEND;
         }
         verif=LOW;
      break;

      case WIFI_STATE_SEND_CIPCLOSE:
         if (nonBlockingDelayRead(&wifiComDelay)) {
            sprintf( strToSend, "AT+CIPCLOSE=%c\r\n", currentConnectionId );
            wifiComStringWrite( strToSend );
            wifiComExpectedResponse  = responseCipclose;
            nonBlockingDelayWrite(&wifiComDelay, DELAY_5_SECONDS);
            wifiComState = WIFI_STATE_WAIT_CIPCLOSE;
         }
      break;

      case WIFI_STATE_WAIT_CIPCLOSE:
         if (isExpectedResponse()) {
            nonBlockingDelayWrite(&wifiComDelay, DELAY_5_SECONDS);
            wifiComState = WIFI_STATE_SEND_CIPSTATUS;
         }
         if (nonBlockingDelayRead(&wifiComDelay)) {
            nonBlockingDelayWrite(&wifiComDelay, DELAY_5_SECONDS);
            wifiComState = WIFI_STATE_SEND_CIPSTATUS;
         }
      break;

      case WIFI_STATE_IDLE:
      case WIFI_STATE_ERROR:
      break;
   }
}

//=====[Implementations of private functions]==================================

bool wifiComCharRead( char* receivedChar )
{
    char receivedCharLocal = '\0';
    if( uartWifi.readable() ) {
        uartWifi.read(&receivedCharLocal,1);
        *receivedChar = receivedCharLocal;
        return true;
    }
    return false;
}

void wifiComStringWrite( const char* str )
{
    uartWifi.write( str, len(str) );
}

static bool isExpectedResponse()
{
   static int responseStringPositionIndex = 0;
   char charReceived;
   bool moduleResponse = false;

   if( wifiComCharRead(&charReceived) ){
      if (charReceived == wifiComExpectedResponse[responseStringPositionIndex]) {
         responseStringPositionIndex++;
         if (wifiComExpectedResponse[responseStringPositionIndex] == '\0') {
            responseStringPositionIndex = 0;
            moduleResponse = true;
         }
      } else {
         responseStringPositionIndex = 0;
      }
   }
   return moduleResponse;
}

void wifiComWebPageDataUpdate()
{
    int temperature = (int)(temperatureSensorReadCelsius() * 100);
    sprintf( htmlCodeBody, "%s Temperatura: %d.%02d &ordm;C %s", 
         BEGIN_USER_LINE, temperature / 100, temperature % 100, END_USER_LINE );

    
    sprintf( htmlCodeBody + len(htmlCodeBody), 
             "%s Sobre temperatura detectada: %s %s", BEGIN_USER_LINE, 
             stateToString( overTemperatureDetectorStateRead() ), END_USER_LINE );
}

char * stateToString( bool state )
{    
    if ( state ) {
        strcpy( stateString, "ON");
    } else {
        strcpy( stateString, "OFF");
    }
    return stateString;
}