//=====[Libraries]=============================================================

#include "mbed.h"
#include "arm_book_lib.h"
#include "fire_alarm.h"
#include "temperature_sensor.h"

//=====[Declaration of private defines]========================================

#define TEMPERATURE_C_LIMIT_ALARM               25.0

//=====[Declaration of private data types]=====================================

//=====[Declaration and initialization of public global objects]===============

DigitalIn fireAlarmTestButton(BUTTON1);

//=====[Declaration of external public global variables]=======================

//=====[Declaration and initialization of public global variables]=============

//=====[Declaration and initialization of private global variables]============

static bool overTemperatureDetected      = OFF;
static bool overTemperatureDetectorState = OFF;

//=====[Declarations (prototypes) of private functions]========================

//=====[Implementations of public functions]===================================

void fireAlarmInit()
{
    temperatureSensorInit();
    fireAlarmTestButton.mode(PullDown); 
}

void fireAlarmUpdate()
{
    temperatureSensorUpdate();

    overTemperatureDetectorState = temperatureSensorReadCelsius() > 
                                   TEMPERATURE_C_LIMIT_ALARM;

    if ( overTemperatureDetectorState ) {
        overTemperatureDetected = ON;
    }

    if( fireAlarmTestButton ) {             
        overTemperatureDetected = ON;
    }
}

bool overTemperatureDetectorStateRead()
{
    return overTemperatureDetectorState;
}

bool overTemperatureDetectedRead()
{
    return overTemperatureDetected;
}

void fireAlarmDeactivate()
{
    overTemperatureDetected = OFF; 
}

//=====[Implementations of private functions]==================================