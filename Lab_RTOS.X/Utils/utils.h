#include "stdbool.h"
#include "../stdint.h"

#include "../mcc_generated_files/pin_manager.h"
#include "../Platform/WS2812.h"

typedef enum{Red, Green, Blue, White, Black}colors;

typedef struct {
    uint8_t led;
    uint8_t color;
    colors colr;
} prender_led;

typedef struct {
    uint8_t led;
    uint8_t color;
    uint32_t time;
} app_register_t;

void settingRGB(uint8_t led, colors color);

bool setearColor(uint8_t cl,colors *color);

bool horaYfechaCorrecta(int hh,int mm,int ss,int dd,int mth,int yy);
