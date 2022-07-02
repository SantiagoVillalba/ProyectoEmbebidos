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

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"
#include "queue.h"

#include "mcc_generated_files/system.h"
#include "mcc_generated_files/pin_manager.h"
#include "Framework/Accelerometer.h"

void UsbController( void *p_param );
void LedCola( void *p_param );
void vUSBService (TimerHandle_t xTimer);

void vUpdatePosition (TimerHandle_t xTimer);

SemaphoreHandle_t semUsb ;
SemaphoreHandle_t semLed ;
char recibir[50];
uint8_t mensaje;
QueueHandle_t queue;

/*
                         Main application
 */
int main(void)
{
    // initialize the device
    SYSTEM_Initialize( );
    while(!ACCEL_init());
    
    // Anterior Lab:
    
    queue = xQueueCreate(20,sizeof(prender_led));
    /* Create the tasks defined within this file. */
    xTaskCreate( UsbController, "task2", configMINIMAL_STACK_SIZE+500, NULL, tskIDLE_PRIORITY+2, NULL );
    xTaskCreate( LedCola, "task3", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+2, NULL );
    
    TimerHandle_t x = xTimerCreate ("Service USB",pdMS_TO_TICKS(5UL),pdTRUE, NULL , vUSBService);
    xTimerStart( x, 0 );
    
 
    semUsb = xSemaphoreCreateCounting(1,0);
    semLed = xSemaphoreCreateCounting(1,1);
    
    // Proyecto
    
    TimerHandle_t x = xTimerCreate ("Update Position",pdMS_TO_TICKS(1UL),pdTRUE, NULL , vUpdatePosition);
    xTimerStart( x, 0 );
    
    
    
    /* Finally start the scheduler. */
    vTaskStartScheduler( );

    /* If all is well, the scheduler will now be running, and the following line
    will never be reached.  If the following line does execute, then there was
    insufficient FreeRTOS heap memory available for the idle and/or timer tasks
    to be created.  See the memory management section on the FreeRTOS web site
    for more details. */
    for(;;);
}


void vUpdatePosition (TimerHandle_t xTimer){
    Accel_t accel;
    if(ACCEL_GetAccel (&accel)){
        
    }
}

void vAgregarQueue (TimerHandle_t xTimer){
    prender_led* mem = (prender_led*)pvTimerGetTimerID(xTimer);
    xQueueSendToBack(queue,mem,0);
    free((void *)mem);
}

void vUSBService (TimerHandle_t xTimer){
    if((USBGetDeviceState() < CONFIGURED_STATE) ||
               (USBIsDeviceSuspended() == true))
            {
                //continue;   //go back to the top of the while loop
            }
            else
            {
                CDCTxService();
                mensaje = getsUSBUSART(recibir, sizeof(recibir));
                if(mensaje!= 0){
                    recibir[mensaje] = '\0';
                    xSemaphoreGive( semUsb );
                }
            }
}

void LedCola( void *p_param ){
    while(1){
        prender_led aMod;
        xQueueReceive(queue,&aMod,portMAX_DELAY);
        //modificarLed( &aMod);    
    }
    
}


void checkUSBReady(){
    while( !USBUSARTIsTxTrfReady()){
        CDCTxService();
        vTaskDelay(pdMS_TO_TICKS(3UL));
    }
}

void UsbController( void *p_param ){
    
    settingRGB(1,Black);
    settingRGB(2,Black);
    settingRGB(3,Black);
    settingRGB(4,Black);
    settingRGB(5,Black);
    settingRGB(6,Black);
    settingRGB(7,Black);
    settingRGB(8,Black);
    
    while(1){
        xSemaphoreTake( semUsb, portMAX_DELAY);
        char nuevo[70];
        Accel_t accel;
        if(ACCEL_GetAccel (&accel)){
            sprintf(nuevo,"\nEntrada de datos:\nx: %f\ny: %f\nz: %f\n",accel.Accel_X,accel.Accel_Y,accel.Accel_Z);
            checkUSBReady();
            putsUSBUSART(nuevo);
        }else{
            checkUSBReady();
            putsUSBUSART("failed");

        }
    }
    
//    char interfazInicial[166];
//    bool seSeteoHora = false;
//    bool seSeteoLed = false;
//    int opcion;
//    bool primeraVez = true;
//
//    strcpy(interfazInicial, "\nEscoja el numero de la opcion\n1- Fijar la fecha"
//            " y hora del reloj \n2- Encender y apagar un led del 1 al 8"
//            "\n3- Consultar estado fecha y hora del ultimo LED modificado\n");
    
//    while (1)
//    {   
//        //xSemaphoreTake( semUsb, portMAX_DELAY);
//        bool formatoCorrecto = true;
//        int formato;
//        int hh, mm, ss;
//        int dd, month, yy;
//        struct tm initialTime;
//        int num;
//        int delay;
//        uint8_t color;
//        struct tm currentTime;
//        time_t tiempo;
//        colors colr;
//        char print[82];
//        struct tm modificado;
//        prender_led ledM;
//        prender_led* memDinamic;
//        app_register_t ultLed;
        
//        if(primeraVez){
//            xSemaphoreTake( semUsb, portMAX_DELAY);
//            checkUSBReady();
//            putsUSBUSART(interfazInicial);
//            primeraVez = false;
//        }
//        xSemaphoreTake( semUsb, portMAX_DELAY);
//        formato = sscanf(recibir,"%u",&opcion);
//        if(formato!= 1 || mensaje > 1){
//            checkUSBReady();
//            putsUSBUSART("\nFormato incorrecto,envie devuelta\n");
//        }else{
//            switch(opcion){
//                case 1: 
//                    do{
//                        checkUSBReady();
//                        putsUSBUSART("\nEl formato para ingresar la hora y "
//                                "fecha es el siguiente: hh;mm;ss;dd;month;yyyy\n");
//                        xSemaphoreTake( semUsb, portMAX_DELAY);
//                        formatoCorrecto = true;
//                        formato = sscanf(recibir,"%d;%d;%d;%d;%d;%4d", &hh,&mm,&ss,&dd,&month,&yy);
//                        if(formato!= 6 || mensaje > 19){
//                            checkUSBReady();
//                            putsUSBUSART("\nFormato incorrecto,envie devuelta\n");
//                            formatoCorrecto = false;
//                        }else{
//                            if(!horaYfechaCorrecta(hh,mm,ss,dd,month,yy)){
//                                checkUSBReady();
//                                putsUSBUSART("\nFecha y Hora incorrectas,envie devuelta\n");
//                                formatoCorrecto = false;
//                            }
//                        }
//                    }while(!formatoCorrecto);
//                    
//                    initialTime.tm_year = yy - 1900;
//                    initialTime.tm_mday = dd;
//                    initialTime.tm_mon = month-1;
//                    initialTime.tm_hour = hh;
//                    initialTime.tm_min = mm;
//                    initialTime.tm_sec = ss;
//                    initialTime.tm_isdst = -1;  
//                    RTCC_TimeSet(&initialTime);
//                    checkUSBReady();
//                    putsUSBUSART("\nOperacion hecha\n");
//                    seSeteoHora = true;
//                    checkUSBReady();
//                    putsUSBUSART(interfazInicial);
//                    break;
//                case 2:
//                    if(!seSeteoHora){
//                        checkUSBReady();
//                        putsUSBUSART("\nAntes de usar esta opcion debe de setear la hora\n");
//                        break;
//                    }
//                    do{
//                        checkUSBReady();
//                        putsUSBUSART("\nEl formato para encender y apagar un "
//                                "led es el siguiente: (numero del led del 1 al 8)"
//                                ";(numero del color);(segundos hasta que se prenda <= 60) \n 1 es Rojo,"
//                                "2 es Verde, 3 es Azul, 4 es Blanco y 5 es Negro \n");
//                        xSemaphoreTake( semUsb, portMAX_DELAY);
//                        formatoCorrecto = true;
//                        formato = sscanf(recibir,"%d;%d;%d",&num,&color,&delay);
//                        if(formato != 3){
//                            checkUSBReady();
//                            putsUSBUSART("\nFormato incorrecto,envie devuelta\n");
//                            formatoCorrecto = false;
//                        }else{
//                            if(!setearColor( color, &colr)){
//                                checkUSBReady();
//                                putsUSBUSART("\nColor incorrecto\n");
//                                formatoCorrecto = false;
//                            }
//                            if(num<1 || num >8){
//                                checkUSBReady();
//                                putsUSBUSART("\nNumero incorrecto\n");
//                                formatoCorrecto = false;
//                            }
//                            if(delay < 0 || delay > 60){
//                                checkUSBReady();
//                                putsUSBUSART("\nSe pasa del minuto, use una cantidad menor\n");
//                                formatoCorrecto = false;
//                            }
//                            if(!RTCC_TimeGet(&currentTime)){
//                                checkUSBReady();
//                                putsUSBUSART("\nNo se pudo conseguir la hora\n");
//                                formatoCorrecto = false;
//                            }
//                        }
//                    }while(!formatoCorrecto);
//                    ledM.color = color;
//                    ledM.led = num;
//                    ledM.colr = colr;
//                    if(delay == 0){
//                        xQueueSendToBack(queue,&ledM,0);
//                    }else{
//                        memDinamic = (prender_led*)malloc(sizeof(prender_led));
//                        memDinamic->color = ledM.color;
//                        memDinamic->led = ledM.led;
//                        memDinamic->colr = ledM.colr;
//                        TimerHandle_t x = xTimerCreate ("DelayLed",pdMS_TO_TICKS(delay*1000),pdFALSE, (void *) memDinamic , vAgregarQueue);
//                        xTimerStart( x, 0 );
//                    }
//                    
//                    checkUSBReady();
//                    putsUSBUSART("\nOperacion hecha\n");
//                    seSeteoLed = true;
//                    checkUSBReady();
//                    putsUSBUSART(interfazInicial);
//                    break;
//                    
//                case 3:
//                    if(!seSeteoLed){
//                        checkUSBReady();
//                        putsUSBUSART("\nAntes de usar esta opcion es necesario cambiar un led\n");
//                        break;
//                    }
//                    
//                    checkUSBReady();
//                    putsUSBUSART("\nEscriba cualquier letra para continuar\n");
//                    xSemaphoreTake( semUsb, portMAX_DELAY);
//                    if(!seSeteoLed){
//                        checkUSBReady();
//                        putsUSBUSART("\nAntes de usar esta opcion es necesario cambiar un led\n");
//                        break;
//                    }
//                    ultLed = ObtenerUltimoModificado();
//                    time_t aux = (time_t)ultLed.time;
//                    modificado = *gmtime (&aux);
//                    sprintf(print,"\nLed elegido:%d, Color elegido:"
//                            " %d, Hora: %u:%u:%u, Fecha: %u-%u-%u \n"
//                            ,ultLed.led,ultLed.color,modificado.tm_hour,modificado.tm_min,
//                            modificado.tm_sec,modificado.tm_mday,modificado.tm_mon + 1,modificado.tm_year + 1900);
//                    checkUSBReady();
//                    putsUSBUSART(print);
//                    checkUSBReady();
//                    putsUSBUSART(interfazInicial);
//                    break;
//                default:
//                    checkUSBReady();
//                    putsUSBUSART("\nLa opcion elegida no esta contemplada\n");
//            }
//        }
//    }
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

