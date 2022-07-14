#include "stdbool.h"
#include "../stdint.h"

#include "../mcc_generated_files/pin_manager.h"
#include "../Platform/WS2812.h"

typedef enum{Red, Green, Blue, White, Black}colors;

void apagarLeds();

typedef struct {
    uint8_t led;
    colors color;
} prender_led;

void settingRGB(uint8_t led, colors color);


