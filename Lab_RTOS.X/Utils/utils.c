#include "utils.h"



void settingRGB(uint8_t led, colors color){
    static ws2812_t leds[8];
    // restamos uno para que vaya del 1 al 8 los leds
    led --;
    switch(color){
        case Red:
            leds[led] = RED;
            break;
        case Green:
            leds[led] = GREEN;
            break;
        case Blue:
            leds[led] = BLUE;
            break;
        case White:
            leds[led] = WHITE;
            break;
        case Black:
            leds[led] = (ws2812_t){0, 0, 0};
            break;
    }
    WS2812_send( leds , 8);
}

void apagarLeds(){
    settingRGB(1,Black);
    settingRGB(2,Black);
    settingRGB(3,Black);
    settingRGB(4,Black);
    settingRGB(5,Black);
    settingRGB(6,Black);
    settingRGB(7,Black);
    settingRGB(8,Black);
}


