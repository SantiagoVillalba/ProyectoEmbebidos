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


