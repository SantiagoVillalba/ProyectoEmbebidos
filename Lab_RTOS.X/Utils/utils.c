#include "utils.h"



/* Have compiler allocate a page of flash from the NVM.  Aligned to a page. */
static const uint32_t __attribute((space(prog),aligned(FLASH_ERASE_PAGE_SIZE_IN_PC_UNITS))) flashTestPage[FLASH_ERASE_PAGE_SIZE_IN_PC_UNITS/4];

uint32_t flash_storage_address = (uint32_t) flashTestPage;

// We have detected a flash hardware error of some type.
static void FlashError()
{
    while (1) 
    { }
}

static void MiscompareError()
{
    while (1) 
    { }
}



void GuardarEnFlash(int score)
{
    uint32_t flashOffset, readData;
    uint16_t result;
    uint32_t write_data[2]= {0};

    // Program Valid Key for NVM Commands
    FLASH_Unlock(FLASH_UNLOCK_KEY);

    // Erase the page of flash at this address
    result = FLASH_ErasePage(flash_storage_address);
    
    if (result == false)
    {
        FlashError();
    }


    // Write 32 bit Data to the first DWORD locations.
    write_data[0] = score;
    write_data[1] = 0;
    
    result  = FLASH_WriteDoubleWord(flash_storage_address, write_data[0],write_data[1]);
    
    if (result == false)
    {
        FlashError();
    }

    // Clear Key for NVM Commands so accidental call to flash routines will not corrupt flash
    FLASH_Lock();
}

int LeerEnFlash(){
    // if flash write had no errors then read the flash to verify the data    
    return FLASH_ReadWord(flash_storage_address);
 
}

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
    int b = 0;
    int numero = 0;
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


