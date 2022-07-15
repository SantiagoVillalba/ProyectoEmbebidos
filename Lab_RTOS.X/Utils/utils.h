#include "stdbool.h"
#include <math.h>
#include "../mcc_generated_files/memory/flash.h"
#include <stdint.h>

#include "../mcc_generated_files/pin_manager.h"
#include "../Platform/WS2812.h"

// Enum de colores
typedef enum{Red, Green, Blue, White, Black}colors;

// Funcion que hace un settingRGB a negro de cada uno de los leds.
void apagarLeds();

// Funcion que dado un numero en base 10 lo cambia a base 4
int cambiarBase4(int puntaje);

// Funciones para guardar y leer en memoria no volatil
void GuardarEnFlash(int score);
int LeerEnFlash();

// Tomando un numero en base 4, se devuelve un color correspondiente para representar
// en el modo highscore de la placa
colors colorBase4(int color);

// Funcion rehusada de los anteriores laboratorios, que a partir de un led y un 
// color, lo prende.
void settingRGB(uint8_t led, colors color);

// Uso chequear angulo para ver que este dentro de los rangos deseados el angulo
void ChequearAnguloR(float *anguloR);

// Funcion arcoTangente ampliada para que ande para todos los valores
float arcoTangente(float y, float x);

// Funcion simple para prender el buzzer el doble de tiempo que el x entrado por parametro
void PrenderBuzzer(int x);

void mostrarHighScore(int puntajeBase4);