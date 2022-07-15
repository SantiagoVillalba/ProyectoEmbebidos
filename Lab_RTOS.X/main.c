#include "TimerYTask/TimersYTasksProyecto.h"

/*
                         Main application
 */
int main(void)
{
    // initialize the device
    SYSTEM_Initialize( );
    
    // Seteo de interrupcion con la funcion antes declarada
    BTN1_SetInterruptHandler(&BotonS2);
    
    // apagamos los leds por si se prendio alguno sin sentido.
    apagarLeds();
    
    // Hacemos un bucle hasta que se inicialize el accelerometro
    while(!ACCEL_init());
    
    // Creamos el timer one-shot con su debido callback y con los 3 segundos que tiene que esperar
    PasoDeSec = xTimerCreate ("Waiting 3 sec",pdMS_TO_TICKS(3000UL),pdFALSE, NULL , vFinishWaiting);

    // Creacion de semaforos:
    // A todos los semaforos binarios por freeRTOS empiezan bloqueados por eso se les hace un give
    semCuadrante = xSemaphoreCreateBinary();
    xSemaphoreGive( semCuadrante );
    
    semDificultadDelay = xSemaphoreCreateBinary();
    xSemaphoreGive( semDificultadDelay );
    
    semHighScore = xSemaphoreCreateBinary();
    xSemaphoreGive( semHighScore );
    
    // Se crean los semaforos contadores para asi poder bloquearnos con el primer take
    arrancoElEnemigo = xSemaphoreCreateCounting(1,0);
            
    semTerminoJuego = xSemaphoreCreateCounting(1,0);
    
    semIniciarJuego = xSemaphoreCreateCounting(1,0);
    
    semArrancarHighScore = xSemaphoreCreateCounting(1,0);
    
    semArrancarIA = xSemaphoreCreateCounting(1,0);
            
    // Creacion de las tasks con sus debidas prioridades y tamaños de stacks
    xTaskCreate( InterfazGeneral, "task0", configMINIMAL_STACK_SIZE+200, NULL, tskIDLE_PRIORITY+10, NULL );
    
    xTaskCreate( HighScore, "task2", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+3, NULL );

    xTaskCreate( IAEnemiga, "task1", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+2, NULL );

    // Se crea el timer que cambia la posicion de la pelotita del player cada 1ms y se inicializa 
    TimerHandle_t TimerPosicion = xTimerCreate ("Update Position",pdMS_TO_TICKS(1UL),pdTRUE, NULL , vUpdatePosition);
    xTimerStart( TimerPosicion, 0 );
    
    
    /* Finally start the scheduler. */
    vTaskStartScheduler( );

    /* If all is well, the scheduler will now be running, and the following line
    will never be reached.  If the following line does execute, then there was
    insufficient FreeRTOS heap memory available for the idle and/or timer tasks
    to be created.  See the memory management section on the FreeRTOS web site
    for more details. */
    for(;;);
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

