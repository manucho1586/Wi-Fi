#include "botones.h"

Ticker ticker1;  //Instanciación de variable tipo Ticker para manejar la función de presioanr el botón del ventilador.
Ticker ticker2;  //Instanciación de variable tipo Ticker para manejar la función de presioanr el botón del buzzer.

//**********Declaracion de botones buzzer y ventilador*********************************************//
DigitalOut buzzer(D5);  //Tengo que ver porque aca esta el LED de depuración de wifi
DigitalOut ventilador(D4);
DigitalIn boton_buzzer(BUTTON1); 
DigitalIn boton_ventilador(D6);
 //************************************************************************************************//

 // *********Variables de debounce*****************************************************************//
volatile int cont1 = 0;             //Contador de retardo cuando se presiona el botón ventilador.
volatile bool checking1 = false;    //Variable que chequea el estado del botón que se presiona.
volatile int cont2 = 0;             //Contador de retardo cuando se presiona el botón buzzer.
volatile bool checking2 = false;    //Variable que chequea el estado del botoón que se presiona.
 //************************************************************************************************//

 // *********Deteccion de estado y activacion/desactivacion de buzzer******************************//
void check_boton_buzzer()
 {
    if (boton_buzzer == 0) {                                // Botón presionado
        if (checking1) {
            cont1++;
            if (cont1 >= 150) {
                buzzer = !buzzer;                           // Alternar el estado del buzzer
                cont1 = 0;                                  // Reiniciar el contador
                checking1 = false;                          // Dejar de chequear
            }
        } else {
            checking1 = true;                               // Comenzar a chequear
            cont1 = 0;                                      // Reiniciar el contador
        }
    } else {
        checking1 = false;                                  // Botón no presionado
        cont1 = 0;                                          // Reiniciar el contador
    }
}
//*************************************************************************************************//

// ******Deteccion de estado y activacion/desactivacion de ventilador******************************//
void check_boton_ventilador() 
{
    if (boton_ventilador != 0) {                            // Botón presionado
        if (checking2) {
            cont2++;
            if (cont2 >= 150) {
                ventilador=!ventilador;                   // Alternar el estado del buzzer
                cont2 = 0;                                  // Reiniciar el contador
                checking2 = false;                          // Dejar de chequear
            }
        } else {
            checking2 = true;                               // Comenzar a chequear
            cont2 = 0;                                      // Reiniciar el contador
        }
    } else {
        checking2 = false;                                  // Botón no presionado
        cont2 = 0;                                          // Reiniciar el contador
    }
}
//*************************************************************************************************//

void chequeo_botones_presionados(void)
{
//***********LLamado a las funciones cada 1 ms*****************************************************//
    ticker1.attach(&check_boton_ventilador, 0.001);   //Se configura ticker1 para llamar a la funcion cada 1 ms.
    ticker2.attach(&check_boton_buzzer, 0.001);       //Se configura ticker2 para llamar a la funcion cada 1 ms.
//*************************************************************************************************//

}