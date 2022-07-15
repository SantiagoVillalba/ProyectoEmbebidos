#include "stdbool.h"
#include <math.h>
#include "FreeRTOS.h"
#include "../mcc_generated_files/memory/flash.h"
#include <stdint.h>

#include "../mcc_generated_files/pin_manager.h"
#include "../Platform/WS2812.h"

typedef enum{Red, Green, Blue, White, Black}colors;

void apagarLeds();

int cambiarBase4(int puntaje);

void GuardarEnFlash(int score);
int LeerEnFlash();

colors colorBase4(int color);

typedef struct {
    uint8_t led;
    colors color;
} prender_led;

void settingRGB(uint8_t led, colors color);


