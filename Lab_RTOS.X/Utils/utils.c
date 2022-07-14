#include "utils.h"


void settingRGB(uint8_t led, colors color){
    static ws2812_t leds[8];
    // restamos uno para que vaya del 1 al 8 los leds
    led --;
    if(led >= 0 && led < 8){
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
    
}

colors colorBase4(int color){
    colors new;
    switch(color){
        case 0:
            new = White;
            break;
        case 1:
            new = Red;
            break;
        case 2:
            new = Blue;
            break;
        case 3:
            new = Green;
            break;
    }
    return new;
}

int cambiarBase4(int puntaje){
    int b;
    int numero;
    int contador = 0;
 
    while(puntaje!=0)
    {
        b = puntaje%4;
        puntaje = puntaje/4;
        numero += b* pow(10,contador);
        contador++;
    }
    return numero;
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


