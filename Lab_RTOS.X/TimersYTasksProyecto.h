/**
  Section: Included Files
 */
#include "mcc_generated_files/rtcc.h"
#include "mcc_generated_files/system.h"
#include "mcc_generated_files/usb/usb_device.h"
#include "mcc_generated_files/usb/usb_device_cdc.h"

#include "Utils/utils.h"
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"
#include "queue.h"

#include "mcc_generated_files/system.h"
#include "mcc_generated_files/pin_manager.h"
#include "Framework/Accelerometer.h"


/*
                    Variables del proyecto:
*/
extern short cuadrantePlayer;
extern short cuadranteMaquina;
// cuenta el highscore de la sesion actual de juego
extern int volatile highScoreCounter;
// cuenta el highscore salido de la memoria flash
extern int volatile historicHighScoreCounter;
// bandera para sincronizar las tasks
extern bool jugando;

// Variables globales sobre la posicion del player
extern float radio;
extern float anguloR;
extern float x;
extern float y;
extern float velocidadX; 
extern float velocidadY;
extern float velocidadTan;
extern float velocidadRad;

// La seed es una variable usada para cambiar el random
extern int volatile seed;

// counterDif es un contador que se fija cuanto tiempo paso
// para asi aumentar la dificultad o no
extern int counterDif;

// Cuanto es el delay que se hace antes de avanzar otra vez al enemigo
extern int dificultadDelayMS;

// Semaforos binarios para controlar el flujo del juego
extern SemaphoreHandle_t semTerminoJuego;
extern SemaphoreHandle_t semIniciarJuego;
// Semaforo para proteger los cuadrantes
extern SemaphoreHandle_t semCuadrante ;
// Semaforo para proteger el cambio de dificultad
extern SemaphoreHandle_t semDificultadDelay;
// Semaforo para señalizar cuando tiene que arrancar el highscore a contar
extern SemaphoreHandle_t arrancoElEnemigo;
// Semaforo para proteger el highScore para hacer cambios en el historico
extern SemaphoreHandle_t semHighScore;
// Semaforos binarios para señalizar arranque de ambas tareas
extern SemaphoreHandle_t semArrancarIA;
extern SemaphoreHandle_t semArrancarHighScore;

// Funciones declaradas para ser usadas en la instanciacion de tareas y timers
void InterfazGeneral( void *p_param );
void vUpdatePosition (TimerHandle_t xTimer);
void IAEnemiga( void *p_param );
void HighScore( void *p_param );
void prenderLed();
void vFinishWaiting (TimerHandle_t xTimer);

// Funcion para atender la interrupcion del BotonS2
void BotonS2();
// declaracion de funcion que es llamada cuando termina el timer para resetear el historicHighScoreCounter
void vFinishWaiting (TimerHandle_t xTimer);
// Bandera para llevar tracking de el estado del boton
extern bool apretoS2;
// Handle del timer para resetear el historicHighScoreCounter para poder parar o resetear este timer
extern TimerHandle_t PasoDeSec;
// Bandera para controlar el inicio del timer o no
extern bool inicioTimer;


