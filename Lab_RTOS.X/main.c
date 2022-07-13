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
short cuadrantePlayer;
short cuadranteMaquina;
int volatile highScoreCounter = 0;
int volatile historicHighScoreCounter = 0;
bool finishWaiting = false;

float radio;
float anguloR;
float x,y;
float velocidadX = 0; 
float velocidadY = 0;
float velocidadTan = 0;
float velocidadRad = 0;

int counterDif = 1;
int dificultadDelayMS = 3000;

SemaphoreHandle_t semTerminoJuego;
SemaphoreHandle_t semIniciarJuego;
SemaphoreHandle_t semCuadrante ;
SemaphoreHandle_t semDificultadDelay;
SemaphoreHandle_t arrancoElEnemigo;
SemaphoreHandle_t semHighScore;
SemaphoreHandle_t semFinishWaiting;

void InterfazGeneral( void *p_param );
void VerBotones( void *p_param );
void vUpdatePosition (TimerHandle_t xTimer);
void IAEnemiga( void *p_param );
void HighScore( void *p_param );
void prenderLed();
/*
                         Main application
 */
int main(void)
{
    // initialize the device
    SYSTEM_Initialize( );
    
    settingRGB(1,Black);
    settingRGB(2,Black);
    settingRGB(3,Black);
    settingRGB(4,Black);
    settingRGB(5,Black);
    settingRGB(6,Black);
    settingRGB(7,Black);
    settingRGB(8,Black);
    
    while(!ACCEL_init());
    
    semCuadrante = xSemaphoreCreateBinary();
    xSemaphoreGive( semCuadrante );
    
    arrancoElEnemigo = xSemaphoreCreateBinary();
    //xSemaphoreGive( arrancoElEnemigo );
    
    semDificultadDelay = xSemaphoreCreateBinary();
    xSemaphoreGive( semDificultadDelay );
    
    semHighScore = xSemaphoreCreateBinary();
    xSemaphoreGive( semHighScore );
    
    semFinishWaiting = xSemaphoreCreateBinary();
    xSemaphoreGive( semFinishWaiting );
    
    semTerminoJuego = xSemaphoreCreateCounting(1,0);
    
    semIniciarJuego= xSemaphoreCreateCounting(1,0);
            
    xTaskCreate( InterfazGeneral, "task0", configMINIMAL_STACK_SIZE+200, NULL, tskIDLE_PRIORITY+10, NULL );
    xTaskCreate( VerBotones, "task-1", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, NULL );
    /* Finally start the scheduler. */
    vTaskStartScheduler( );

    /* If all is well, the scheduler will now be running, and the following line
    will never be reached.  If the following line does execute, then there was
    insufficient FreeRTOS heap memory available for the idle and/or timer tasks
    to be created.  See the memory management section on the FreeRTOS web site
    for more details. */
    for(;;);
}

void vFinishWaiting (TimerHandle_t xTimer){
    xSemaphoreTake( semFinishWaiting, portMAX_DELAY);
    finishWaiting = true;
    xSemaphoreGive( semFinishWaiting );
}

void VerBotones( void *p_param ){
    bool timerCreado = false;
    TimerHandle_t PasoDeSec;
    while(1){
        if(BTN1_GetValue() && BTN2_GetValue()){
            if(!timerCreado){
                timerCreado = true;
                PasoDeSec = xTimerCreate ("Waiting 3 sec",pdMS_TO_TICKS(3000UL),pdFALSE, NULL , vFinishWaiting);
                xTimerStart( PasoDeSec, 0 );
            }
            xSemaphoreTake( semFinishWaiting, portMAX_DELAY);
            if(timerCreado && finishWaiting){
                //proteger valor HighScore
                xSemaphoreTake( semHighScore, portMAX_DELAY);
                int volatile highScoreCounter = 0;
                int volatile historicHighScoreCounter = 0;
                timerCreado = false;
                finishWaiting = false;
                xSemaphoreGive( semHighScore );
            }
            xSemaphoreGive( semFinishWaiting );
        }else if(BTN1_GetValue()&& !BTN2_GetValue()){
            xSemaphoreGive( semTerminoJuego );
            xSemaphoreGive( semIniciarJuego );
        }
        if(timerCreado && !BTN2_GetValue()){
            timerCreado = false;
            xTimerDelete( PasoDeSec , 0 );
            finishWaiting = false;
        }
    }
}

void InterfazGeneral( void *p_param ){
    TaskHandle_t IA;
    TaskHandle_t HighScore;
    while(1){
        xSemaphoreTake( semIniciarJuego, portMAX_DELAY);
        //radio = rand() % 0.05;
        //while(radio<110){
            //radio = rand() % 0.05;
        //}
        //anguloR = rand() * ((float)M_PI*2) / RAND_MAX  ;
        
        // esta hardcoded
        radio = 0.03;
        anguloR = (float)3*M_PI/4;
        
        // Convertir esto en cartesiana.
        x = radio * cos(anguloR);
        y = radio * sin(anguloR);

        prenderLed();
        
        xTaskCreate( IAEnemiga, "task1", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+3, &IA );
    
        xTaskCreate( HighScore, "task2", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+2, &HighScore );

        TimerHandle_t TimerPosicion = xTimerCreate ("Update Position",pdMS_TO_TICKS(1UL),pdTRUE, NULL , vUpdatePosition);
        xTimerStart( TimerPosicion, 0 );
        
        xSemaphoreTake( semTerminoJuego, portMAX_DELAY);
        vTaskDelete( IA );
        vTaskDelete( HighScore );
        xTimerDelete( TimerPosicion , 0 );
        
        xSemaphoreTake( semHighScore, portMAX_DELAY);
        if(highScoreCounter > historicHighScoreCounter){
            historicHighScoreCounter = highScoreCounter;
        }
        xSemaphoreGive( semHighScore );
        
    }
}

void HighScore( void *p_param ){
    // Se espera a que arranque a correr el enemigo.
    xSemaphoreTake( arrancoElEnemigo, portMAX_DELAY);
    
    while(1){      
        vTaskDelay(pdMS_TO_TICKS(1000UL));
        xSemaphoreTake( semHighScore, portMAX_DELAY);
        highScoreCounter++;
        xSemaphoreGive( semHighScore);
        counterDif++;  
        if(counterDif == 10 ){
            xSemaphoreTake( semDificultadDelay, portMAX_DELAY);
            if(dificultadDelayMS > 400){
                dificultadDelayMS -= 400;
                counterDif = 1;
            }
            xSemaphoreGive( semDificultadDelay );
        }
    }
    
}

void IAEnemiga( void *p_param ){
    vTaskDelay(pdMS_TO_TICKS(1000UL));
    cuadranteMaquina = (rand() % 8) + 1;
    xSemaphoreTake( semCuadrante, portMAX_DELAY);
    while(cuadranteMaquina == cuadrantePlayer){
        cuadranteMaquina = (rand() % 8) + 1;
    }
    xSemaphoreGive( semCuadrante );
    xSemaphoreGive( arrancoElEnemigo );
    while(1){
        int delaydif;
        xSemaphoreTake( semDificultadDelay, portMAX_DELAY);
        delaydif = dificultadDelayMS;
        xSemaphoreGive( semDificultadDelay );
        vTaskDelay(pdMS_TO_TICKS(delaydif));
        // logica del enemigo
        xSemaphoreTake( semCuadrante, portMAX_DELAY);
        if(cuadranteMaquina == cuadrantePlayer){
            // se termino el juego perdiste;
            xSemaphoreGive( semTerminoJuego );
        }else{
            int contadorRecorroNormal = 0;
            int punteroNormal = cuadranteMaquina;
            bool encuentroNormal = false;
            
            int contadorRecorroDerAIzq = 0;
            int punteroDerAIzq = cuadranteMaquina;
            bool encuentroDerAIzq = false;
            while(!encuentroNormal || !encuentroDerAIzq){
                if(punteroNormal != cuadrantePlayer){
                    punteroNormal = (punteroNormal % 8) + 1;
                    contadorRecorroNormal++;
                }else{
                    encuentroNormal = true;
                }
                
                if(punteroDerAIzq != cuadrantePlayer){
                    if(punteroDerAIzq > 1){
                        punteroDerAIzq = punteroDerAIzq - 1;
                    }else{
                        punteroDerAIzq = 8;
                    }
                    contadorRecorroDerAIzq++;
                }else{
                    encuentroDerAIzq = true;
                }
            }
            settingRGB(cuadranteMaquina,Black);
            
            if(contadorRecorroNormal <= contadorRecorroDerAIzq){
                cuadranteMaquina = (cuadranteMaquina % 8) + 1;
            }else{
                if(cuadranteMaquina > 1){
                    cuadranteMaquina = (cuadranteMaquina - 1);
                }else{
                    cuadranteMaquina = 8;
                }
            }
        }
        settingRGB(cuadranteMaquina,Red);
        xSemaphoreGive( semCuadrante );
    }
}

float arcoTangente(float x, float y){
    if(x > 0 && y > 0){
        return atan((float)(y/x)) ;
    }else if(x = 0 && y > 0){
        return (float)(M_PI / 2);
    }else if(x < 0){
        return atan((float)(y/x)) + (float)M_PI;
    }else if(x = 0 && y < 0){
        return (float)(3*M_PI / 2) ;
    }else if(x > 0 && y < 0){
        return atan((float)(y/x)) + (float)2*M_PI;
    }
}

void prenderLed(){
    short cuadranteApagar;
    
    if(anguloR >= (float)M_PI*2){
        anguloR -= (float)2*M_PI;
    }else if(anguloR < 0){
        anguloR += (float)2*M_PI;
    }
    
    // protegerlo con semaforos
    xSemaphoreTake( semCuadrante, portMAX_DELAY);
    if(anguloR >= 0 && anguloR < (float)(M_PI/4)){
        if(cuadrantePlayer != 1){
            cuadranteApagar = cuadrantePlayer;
            cuadrantePlayer = 1;
            settingRGB(cuadranteApagar,Black);
        }
    }else if(anguloR >= (float)(M_PI/4) && anguloR < (float)(M_PI/2)){
        if(cuadrantePlayer != 8){
            cuadranteApagar = cuadrantePlayer;
            cuadrantePlayer = 8;
            settingRGB(cuadranteApagar,Black);
        }
    }else if(anguloR >= (float)(M_PI/2) && anguloR < (float)(3*M_PI/4)){
        if(cuadrantePlayer != 7){
            cuadranteApagar = cuadrantePlayer;
            cuadrantePlayer = 7;
            settingRGB(cuadranteApagar,Black);
        }
    }else if(anguloR >= (float)(3*M_PI/4) && anguloR < (float)M_PI){
        if(cuadrantePlayer != 6){
            cuadranteApagar = cuadrantePlayer;
            cuadrantePlayer = 6;
            settingRGB(cuadranteApagar,Black);
        }
    }else if(anguloR >= (float)M_PI && anguloR < (float)(5*M_PI/4)){
        if(cuadrantePlayer != 5){
            cuadranteApagar = cuadrantePlayer;
            cuadrantePlayer = 5;
            settingRGB(cuadranteApagar,Black);
        }
    }else if(anguloR >= (float)(5*M_PI/4) && anguloR < (float)(3*M_PI/2)){
        if(cuadrantePlayer != 4){
            cuadranteApagar = cuadrantePlayer;
            cuadrantePlayer = 4;
            settingRGB(cuadranteApagar,Black);
        }
    }else if(anguloR >= (float)(3*M_PI/2) && anguloR < (float)(7*M_PI/4)){
        if(cuadrantePlayer != 3){
            cuadranteApagar = cuadrantePlayer;
            cuadrantePlayer = 3;
            settingRGB(cuadranteApagar,Black);
        }
    }else if(anguloR >= (float)(7*M_PI/4)){
        if(cuadrantePlayer != 2){
            cuadranteApagar = cuadrantePlayer;
            cuadrantePlayer = 2;
            settingRGB(cuadranteApagar,Black);
        }
    }
    if(cuadrantePlayer == cuadranteMaquina){
        // perder
        xSemaphoreGive( semTerminoJuego );
    }else{
        settingRGB(cuadrantePlayer,White);
    }
    xSemaphoreGive( semCuadrante );
}

void vUpdatePosition (TimerHandle_t xTimer){
    Accel_t accel;
    // Sistema Polar
    float acTan;
    float acRadial;
    float acTanRoz,acRadRoz;
    float varVelocidadTan,varVelocidadRadial;
    float varPosicRad;
    float varAnguloTan;
    // Sistema Rectangular
    float acXRoz,acYRoz;
    float varVX,varVY;
    float varPX,varPY;
    
    if(ACCEL_GetAccel (&accel)){
        if(radio>=0.015){
            acRadial = accel.Accel_X* -0.1*cos( anguloR ) + accel.Accel_Y * -0.1 *sin( anguloR );
            acTan = accel.Accel_Y*-0.1*cos( anguloR ) - accel.Accel_X * -0.1*sin( anguloR );
            acRadRoz = acRadial - velocidadRad*cos(arcoTangente(velocidadRad,velocidadTan)) * 10;
            acTanRoz = acTan - velocidadTan*sin(arcoTangente(velocidadTan,velocidadRad)) * 10;
            
            varVelocidadTan = acTanRoz * 0.001;
            varVelocidadRadial = acRadRoz * 0.001;
            velocidadTan += varVelocidadTan;
            velocidadRad += varVelocidadRadial;
            varPosicRad = velocidadRad * 0.001 ;
            radio += varPosicRad;
            varAnguloTan = velocidadTan/radio* 0.001 ;
            anguloR += varAnguloTan; 
            if(radio>=0.05 && velocidadRad > 0){
                radio = 0.05;
                velocidadRad = 0;
            }
            x = radio * cos(anguloR);
            y = radio * sin(anguloR);
            velocidadX = velocidadRad*cos(anguloR) - velocidadTan*sin(anguloR);
            velocidadY = velocidadTan*cos(anguloR) + velocidadRad*sin(anguloR);
            
        }else{
            acXRoz = accel.Accel_X - velocidadX*cos(arcoTangente(velocidadX,velocidadY)) * 10;
            acYRoz = accel.Accel_Y - velocidadY*sin(arcoTangente(velocidadY,velocidadX)) * 10;
            
            varVX = acXRoz * -0.1 * 0.001;
            varVY = acYRoz * -0.1 * 0.001;
            velocidadX += varVX;
            velocidadY += varVY;
            varPX = velocidadX * 0.001;
            varPY = velocidadY * 0.001;
            x += varPX;
            y += varPY;
            
            radio = (float)sqrt(x*x+y*y);
            anguloR = arcoTangente(x,y);
            
            velocidadRad = velocidadX*cos(anguloR) + velocidadY*sin(anguloR);
            velocidadTan = -velocidadX*sin(anguloR) + velocidadY*cos(anguloR);
        }
        prenderLed();
    }
    
}

void vApplicationMallocFailedHook( void ){
    /* vApplicationMallocFailedHook() will only be called if
    configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
    function that will get called if a call to pvPortMalloc() fails.
    pvPortMalloc() is called internally by the kernel whenever a task, queue,
    timer or semaphore is created.  It is also called by various parts of the
    demo application.  If heap_1.c or heap_2.c are used, then the size of the
    heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
    FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
    to query the size of free heap space that remains (although it does not
    provide information on how the remaining heap might be fragmented). */
    taskDISABLE_INTERRUPTS( );
    for(;;);
}

/*-----------------------------------------------------------*/

void vApplicationIdleHook( void ){ //lo hago cada vez que la idle task se esté ejecutando
    /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
    to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
    task.  It is essential that code added to this hook function never attempts
    to block in any way (for example, call xQueueReceive() with a block time
    specified, or call vTaskDelay()).  If the application makes use of the
    vTaskDelete() API function (as this demo application does) then it is also
    important that vApplicationIdleHook() is permitted to return to its calling
    function, because it is the responsibility of the idle task to clean up
    memory allocated by the kernel to any task that has since been deleted. */
}

/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName ){ // lo hago cuando me paso del stackoverflow
    (void) pcTaskName; 
    (void) pxTask;

    /* Run time task stack overflow checking is performed if
    configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook	function is 
    called if a task stack overflow is detected.  Note the system/interrupt
    stack is not checked. */
    taskDISABLE_INTERRUPTS( );
    for(;;);
}

/*-----------------------------------------------------------*/

void vApplicationTickHook( void ){ // lo que voy a hacer cada vez que haga un tick fiumba(pasa esta funcion))
    /* This function will be called by each tick interrupt if
    configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h.  User code can be
    added here, but the tick hook is called from an interrupt context, so
    code must not attempt to block, and only the interrupt safe FreeRTOS API
    functions can be used (those that end in FromISR()). */
}

/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/

void vAssertCalled( const char * pcFile, unsigned long ulLine ){
    volatile unsigned long ul=0;

    (void) pcFile;
    (void) ulLine;

    __asm volatile( "di");
    {
        /* Set ul to a non-zero value using the debugger to step out of this
        function. */
        while(ul==0){
            portNOP( );
        }
    }
    __asm volatile( "ei");
}

/**
 End of File
*/

