#ifndef BOTONES_H
#define BOTONES_H

#include "mbed.h"
//**********Declaracion de botones buzzer y ventilador**********************//
 extern DigitalOut buzzer;
 extern DigitalOut ventilador;
 extern DigitalIn boton_buzzer; 
 extern DigitalIn boton_ventilador;
//************************************************************************//

extern Ticker ticker1;                 //Instanciación de variable tipo Ticker para manejar la función de presioanr el botón del ventilador.
extern Ticker ticker2;                 //Instanciación de variable tipo Ticker para manejar la función de presioanr el botón del buzzer.
void chequeo_botones_presionados(void);
 
void check_boton_buzzer(void);
void check_boton_ventilador(void);

#endif //BOTONES_H