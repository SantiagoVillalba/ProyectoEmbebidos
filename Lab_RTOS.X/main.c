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
// cuenta el highscore de la sesion actual de juego
int volatile highScoreCounter = 0;
// cuenta el highscore salido de la memoria flash
int volatile historicHighScoreCounter = 0;
// bandera para sincronizar las tasks
bool jugando = false;

// Variables globales sobre la posicion del player
float radio = 0;
float anguloR = 0;
float x = 0;
float y = 0;
float velocidadX = 0; 
float velocidadY = 0;
float velocidadTan = 0;
float velocidadRad = 0;

// La seed es una variable usada para cambiar el random
int volatile seed = 0;

// counterDif es un contador que se fija cuanto tiempo paso
// para asi aumentar la dificultad o no
int counterDif = 1;

// Cuanto es el delay que se hace antes de avanzar otra vez al enemigo
int dificultadDelayMS = 3000;

// Semaforos binarios para controlar el flujo del juego
SemaphoreHandle_t semTerminoJuego;
SemaphoreHandle_t semIniciarJuego;
// Semaforo para proteger los cuadrantes
SemaphoreHandle_t semCuadrante ;
// Semaforo para proteger el cambio de dificultad
SemaphoreHandle_t semDificultadDelay;
// Semaforo para señalizar cuando tiene que arrancar el highscore a contar
SemaphoreHandle_t arrancoElEnemigo;
// Semaforo para proteger el highScore para hacer cambios en el historico
SemaphoreHandle_t semHighScore;
// Semaforos binarios para señalizar arranque de ambas tareas
SemaphoreHandle_t semArrancarIA;
SemaphoreHandle_t semArrancarHighScore;

// Funciones declaradas para ser usadas en la instanciacion de tareas y timers
void InterfazGeneral( void *p_param );
void vUpdatePosition (TimerHandle_t xTimer);
void IAEnemiga( void *p_param );
void HighScore( void *p_param );
void prenderLed();
void ChequearAnguloR();
void vFinishWaiting (TimerHandle_t xTimer);

// Funcion para atender la interrupcion del BotonS2
void BotonS2();
// declaracion de funcion que es llamada cuando termina el timer para resetear el historicHighScoreCounter
void vFinishWaiting (TimerHandle_t xTimer);
// Bandera para llevar tracking de el estado del boton
bool apretoS2 = false;
// Handle del timer para resetear el historicHighScoreCounter para poder parar o resetear este timer
TimerHandle_t PasoDeSec;
// Bandera para controlar el inicio del timer o no
bool inicioTimer = false;

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

// Funcion simple para prender el buzzer el doble de tiempo que el x entrado por parametro
void PrenderBuzzer(int x){
    for(int i = 0; i <= x;i++){
        BUZZER_SetHigh();
        vTaskDelay(pdMS_TO_TICKS(1));
        BUZZER_SetLow();
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

// Funcion que va digito por digito de el puntaje en base 4 y va prendiendo los leds para mostrar el highScore
void mostrarHighScore(int puntajeBase4){
    /*
     * 0 es blanco
     * 1 es rojo
     * 2 es azul
     * 3 es verde
     */
    int digito = 8;
    while(digito >= 5)
    {
        // Se hace un delay para que se prendan correctamente los leds
        vTaskDelay(pdMS_TO_TICKS(1));
        settingRGB( digito , colorBase4(puntajeBase4 % 10));
        digito--;
        puntajeBase4 /= 10;
    }
}

// Callback de la interrupcion para el botonS2
void BotonS2(){
    // se inicializa un contador static para asi no perder el valor cuando se salga
    // y para poder saber en que flanco estamos
    int static contadorS2 = 0;
    contadorS2++;
    // Se le manda al srand una seed para asi que cambie el valor de random para
    // la proxima vez que tengamos que crear la posicion del player y el enemigo
    srand(++seed);

    // se da vuelta el valor de la bandera
    apretoS2 = !apretoS2;
    
    // Si se inicio el timer y el flanco es de cuando soltamos el boton
    // paramos el timer y cambiamos la bandera
    if(inicioTimer && !apretoS2){
        xTimerStopFromISR(PasoDeSec,NULL);
        inicioTimer = false;
    }
    
    // si no se inicio timer y el flanco es de cuando soltamos el boton, dependiendo
    // de si se esta jugando o no liberamos los semaforos correspondientes
    if(!inicioTimer && !apretoS2 && contadorS2 == 2){
        if(jugando){
            xSemaphoreGiveFromISR( semTerminoJuego ,NULL);
            xSemaphoreGiveFromISR( semIniciarJuego ,NULL);
        }else{
            xSemaphoreGiveFromISR( semIniciarJuego ,NULL);
        }
    }
    
    // en caso de haberse apretado el boton y que no se haya inciado timer
    // reiniciamos el timer que resetea el highscore
    if(!inicioTimer && apretoS2  && contadorS2 == 1){
        inicioTimer = true;
        xTimerResetFromISR(PasoDeSec,NULL);
    }
    
    // Reseteamos el contadorS2 para poder seguir trackeando el estado del boton
    if(contadorS2 >= 2){
        contadorS2 = 0;
    }
}

// Callback del timer de 3 segundos que se controla con el boton
// Funcion que estando protegido por el semaforo, resetea el valor del
// historicHighScoreCounter y guarda en memoria no volatil el cambio
void vFinishWaiting (TimerHandle_t xTimer){
    xSemaphoreTake( semHighScore, portMAX_DELAY);
    historicHighScoreCounter = 0;
    GuardarEnFlash(0);
    xSemaphoreGive( semHighScore );
    inicioTimer = false;
}

// Task principal de nuestro sistema que controla el flujo de todo lo demas 
// a traves de banderas o semaforos.
void InterfazGeneral( void *p_param ){
    int puntajeBase4 = 0;
    while(1){
        // me bloqueo esperando a que me desbloquee el boton
        xSemaphoreTake( semIniciarJuego, portMAX_DELAY);
        // ARRANCA JUEGO
        
        // apenas me desbloqueo me aseguro que el valor historico que tengo este bien
        // cargandolo desde la memoria no volatil
        historicHighScoreCounter = LeerEnFlash();
        // prendo el buzzer para inicializar el incio del juego
        PrenderBuzzer(500);
        // apago los leds en caso de que hayan quedado prendidas del juego previo
        apagarLeds();
        // reseteo todas las variables que podrian haber quedado sin cambiar del anterior juego
        highScoreCounter = 0;
        cuadrantePlayer = 0;
        cuadranteMaquina = 0;
        x = 0;
        y = 0;
        radio = 0;
        anguloR = 0;
        velocidadX = 0;
        velocidadY = 0;
        velocidadTan = 0;
        velocidadRad = 0;
        // cambiar posicion bolita random
        radio = (float)(rand() % 50 );
        while(radio < 5){
            radio = (float)(rand() % 50 );
        }
        radio /= 1000;
        anguloR = rand() * ((float)M_PI*2) / RAND_MAX  ;
        
        // Uso chequear angulo para ver que este dentro de los rangos deseados el angulo
        ChequearAnguloR();

        // Convertir los datos polares en cartesianos.
        x = radio * cos(anguloR);
        y = radio * sin(anguloR);
        
        // Prendo la led de donde se encuentra ahora el player
        prenderLed();
        
        //arrancar tasks y habilitar timer
        
        // habilito la bandera para el timer de updatePosition
        jugando = true;
        // libero ambas tasks para que arranque a andar
        // pero como donde estoy parado es de mayor prioridad no pasan a ejecutar todavia
        xSemaphoreGive(semArrancarIA);
        xSemaphoreGive(semArrancarHighScore);
        
        // ME BLOQUEO ESPERANDO A QUE TERMINE
        xSemaphoreTake( semTerminoJuego, portMAX_DELAY);
        // TERMINO JUEGO
        
        //cambio jugando para que se bloquee el timer de el player
        jugando = false;
        // borro el cuadrante maquina para la prox partida
        cuadranteMaquina = 0;
        // apago los leds que quedaron prendidos de el juego recien terminado
        apagarLeds();
        
        // Protejo el highscore
        xSemaphoreTake( semHighScore, portMAX_DELAY);
        // muestro el highscore en base a 4 con leds
        puntajeBase4 = cambiarBase4(highScoreCounter);
        mostrarHighScore(puntajeBase4);
        // Si el highscore actual es mayor que el historico actualizo el historico
        // y hago sonar 5 veces el buzzer de manera corta y repetida
        if(highScoreCounter > historicHighScoreCounter){
            
            historicHighScoreCounter = highScoreCounter;
            GuardarEnFlash(historicHighScoreCounter);
            
            PrenderBuzzer(50);
            vTaskDelay(pdMS_TO_TICKS(100));
            PrenderBuzzer(50);
            vTaskDelay(pdMS_TO_TICKS(100));
            PrenderBuzzer(50);
            vTaskDelay(pdMS_TO_TICKS(100));
            PrenderBuzzer(50);
            vTaskDelay(pdMS_TO_TICKS(100));
            PrenderBuzzer(50);
            
        }else{
            // En caso de que no se haya superado el highscore historico
            // simplemente hacemos sonar el buzzer dos veces para mostrar que se termino el juego
            PrenderBuzzer(200);
            vTaskDelay(pdMS_TO_TICKS(100));
            PrenderBuzzer(200);
        }
        // libero la proteccion del highscore 
        xSemaphoreGive( semHighScore );
    }
}

// Task que va contando el score actual del jugador cada 1 segundo
// y cada 10 segundos cambia la dificultad del juego
void HighScore( void *p_param ){
    while(1){
        // te bloqueas esperando a que la tarea de interfaz general te permita desbloquearte
        xSemaphoreTake(semArrancarHighScore, portMAX_DELAY);
        // proteges el cambio de dificultad
        xSemaphoreTake( semDificultadDelay, portMAX_DELAY);
        // seteamos el movimiento inicial cada 3 segundos 
        dificultadDelayMS = 3000;
        // desbloqueas el semaforo de la dificultad
        xSemaphoreGive( semDificultadDelay );
        // Se espera a que arranque a correr el enemigo.
        xSemaphoreTake( arrancoElEnemigo, portMAX_DELAY);
        // mientras la bandera de jugando sea true
        while(jugando){
            // nos bloqueamos cada 1 sec para poder llevar la cuenta de los segundos
            vTaskDelay(pdMS_TO_TICKS(1000UL));
            // se protege el cambio en el HighScore
            xSemaphoreTake( semHighScore, portMAX_DELAY);
            highScoreCounter++;
            xSemaphoreGive( semHighScore);
            // aumentamos el contador de dificultad
            counterDif++;  
            // si se llego a los 10 segundos ya se puede aumentar la dificultad
            if(counterDif == 10 ){
                // protegemos el cambio de dificultad
                xSemaphoreTake( semDificultadDelay, portMAX_DELAY);
                // si se puede aumentar la dificultad, se resta al delay.
                if(dificultadDelayMS > 400){
                    dificultadDelayMS -= 400;
                    counterDif = 1;
                }
                xSemaphoreGive( semDificultadDelay );
            }
        }
        // se resetea el contador para cambiar la dificultad ya que no se esta mas jugando
        counterDif = 1;
    }
}

// Task que controla los movimientos de la task enemiga
void IAEnemiga( void *p_param ){
    while(1){
        // nos bloqueamos esperando a que la task principal nos libere el semaforo 
        xSemaphoreTake(semArrancarIA, portMAX_DELAY);
        // chequeamos la bandera de jugando
        if(jugando){
            //esperamos un segundo para aparecer como dice la letra
            vTaskDelay(pdMS_TO_TICKS(1000UL));
            // protegemos los cuadrantes
            xSemaphoreTake( semCuadrante, portMAX_DELAY);
            //randomizar el lugar donde aparece el enemigo:
            cuadranteMaquina = (rand() % 8) + 1;
            while(cuadranteMaquina == cuadrantePlayer){
                cuadranteMaquina = (rand() % 8) + 1;
            }
            // prendemos donde estamos
            settingRGB(cuadranteMaquina,Red);
            xSemaphoreGive( semCuadrante );
            // liberamos la task del highscore para que arranque a contar
            xSemaphoreGive( arrancoElEnemigo );
            // chequeamos la bandera de jugando otra vez porque en el delay de un segundo de arriba
            // se podria haber cambiado otra vez esta bandera.
            while(jugando){
                //protegemos la dificultad y nos bloqueamos
                xSemaphoreTake( semDificultadDelay, portMAX_DELAY);
                vTaskDelay(pdMS_TO_TICKS(dificultadDelayMS));
                xSemaphoreGive( semDificultadDelay );
                // logica del enemigo
                // otra vez chequeamos el estado de jugando porque en el delay se podria haber cambiado
                if(!jugando){
                    break;
                }
                // protegemos los cuadrantes
                xSemaphoreTake( semCuadrante, portMAX_DELAY);
                if(cuadranteMaquina == cuadrantePlayer){
                    // se termino el juego perdiste;
                    if(jugando){
                        xSemaphoreGive( semTerminoJuego );
                    }
                }else{
                    /* 
                    En el caso de que no estes en el mismo cuadrante que el player
                    se busca cual es el mejor camino para llegar al player y se va por ese.
                    Para encontrar el mejor camino recorremos de izquierda a derecha o de manera normal
                    como indican las variables y tambien recorremos de derecha a izquierda, el que encuentre al player en menor
                    cantidad de movimientos sera el que indique como se va a mover la maquina
                    */
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
                    // apagamos el cuadrante anterior para poder cambiar
                    settingRGB(cuadranteMaquina,Black);
                    
                    // chequeamos cual de los dos caminos es mejor
                    if(contadorRecorroNormal <= contadorRecorroDerAIzq){
                        cuadranteMaquina = (cuadranteMaquina % 8) + 1;
                    }else{
                        if(cuadranteMaquina > 1){
                            cuadranteMaquina = (cuadranteMaquina - 1);
                        }else{
                            cuadranteMaquina = 8;
                        }
                    }
                    // prendemos el cuadrante actual de rojo
                    settingRGB(cuadranteMaquina,Red);
                }
                // desbloqueamos el semaforo para el cuadrante
                xSemaphoreGive( semCuadrante );
            }
        }
    }
}

// Funcion arcoTangente ampliada para que ande para todos los valores
float arcoTangente(float y, float x){
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

// Chequeo del angulo del player para ver si esta dentro del rango entre 0 y 2*PI
void ChequearAnguloR(){
    if(anguloR >= (float)M_PI*2){
        anguloR -= (float)2*M_PI;
    }else if(anguloR < 0){
        anguloR += (float)2*M_PI;
    }
}

void prenderLed(){
    short cuadranteApagar;
    
    // chequeas el angulo
    ChequearAnguloR();
    
    // orotegemos el cambio de cuadrantes
    xSemaphoreTake( semCuadrante, portMAX_DELAY);
    // nos fijamos segun el angulo en que cuadrante estamos
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
    // Despues de setear el cuadrante actual nos fijamos si perdimos o no, y liberamos
    // el semaforo para que se active la task principal o prendemos nuestro led respectivamente.
    if(cuadrantePlayer == cuadranteMaquina){
        // perder
        if(jugando){
            xSemaphoreGive( semTerminoJuego );
        }
    }else{
        settingRGB(cuadrantePlayer,White);
    }
    xSemaphoreGive( semCuadrante );
}

// Callback del timer que se activa cada 1ms que actualiza la posicion de la pelotita del jugador
void vUpdatePosition (TimerHandle_t xTimer){
    // se chequea la bandera ya que este timer sigue andando aunque este todo parado
    if(jugando){
        Accel_t accel;
        // Sistema Polar
        float acTan;
        float acRadial;
        //float acTanRoz,acRadRoz;
        float varVelocidadTan,varVelocidadRadial;
        float varPosicRad;
        float varAnguloTan;
        // Sistema Rectangular
        //float acXRoz,acYRoz;
        float varVX,varVY;
        float varPX,varPY;
        // si se pudo conseguir una aceleracion hacemos los calculos debidos
        if(ACCEL_GetAccel (&accel)){
            // mientras estemos por fuera del rango definido hacemos los calculos con polares , sino pasamos al sistema rectangular
            if(radio>=0.0005){
                // calculamos las aceleraciones radiales y tangenciales
                acRadial = accel.Accel_X* -0.1*cos( anguloR ) + accel.Accel_Y * -0.1 *sin( anguloR );
                acTan = accel.Accel_Y*-0.1*cos( anguloR ) - accel.Accel_X * -0.1*sin( anguloR );

                //acRadRoz = acRadial - sqrt(powf(velocidadTan,2)+powf(velocidadRad,2))*cos(arcoTangente(velocidadTan,velocidadRad)) * 10;
                //acTanRoz = acTan - sqrt(powf(velocidadTan,2)+powf(velocidadRad,2))*sin(arcoTangente(velocidadTan,velocidadRad)) * 10;
                
                // hacemos los calculos para calcular las otras variables
                varVelocidadTan = acTan * 0.001;
                varVelocidadRadial = acRadial * 0.001;
                velocidadTan += varVelocidadTan;
                velocidadRad += varVelocidadRadial;
                varPosicRad = velocidadRad * 0.001 ;
                radio += varPosicRad;
                varAnguloTan = velocidadTan/radio* 0.001 ;
                anguloR += varAnguloTan; 
                
                // Si nos pasamos del limite y estamos yendo afuera de el, ponemos a la pelota en el borde y reseteamos la velocidad hacia afuera
                if(radio>=0.05 && velocidadRad > 0){
                    radio = 0.05;
                    velocidadRad = 0;
                }
                
                // se chequea el angulo para ver si es correcto antes de pasar al sistema rectangular
                ChequearAnguloR();
                
                // pasamos los datos calculados al sistema rectangular
                x = radio * cos(anguloR);
                y = radio * sin(anguloR);
                velocidadX = velocidadRad*cos(anguloR) - velocidadTan*sin(anguloR);
                velocidadY = velocidadTan*cos(anguloR) + velocidadRad*sin(anguloR);

            }else{
                // Calculos del sistema rectangular cuando estamos en el centro
                
                //acXRoz = accel.Accel_X - sqrt(powf(velocidadX,2)+powf(velocidadY,2))*cos(arcoTangente(velocidadY,velocidadX)) * 10;
                //acYRoz = accel.Accel_Y - sqrt(powf(velocidadX,2)+powf(velocidadY,2))*sin(arcoTangente(velocidadY,velocidadX)) * 10;

                varVX = accel.Accel_X * -0.1 * 0.001;
                varVY = accel.Accel_Y * -0.1 * 0.001;
                velocidadX += varVX;
                velocidadY += varVY;
                varPX = velocidadX * 0.001;
                varPY = velocidadY * 0.001;
                x += varPX;
                y += varPY;
                
                // conversion a polares
                radio = (float)sqrt((x*x)+(y*y));
                anguloR = arcoTangente(y,x);
                
                // se chequea el angulo despues de la conversion de rectangular a polar
                ChequearAnguloR();

                velocidadRad = velocidadX*cos(anguloR) + velocidadY*sin(anguloR);
                velocidadTan = -velocidadX*sin(anguloR) + velocidadY*cos(anguloR);
            }
            // Se prende el led donde estamos.
            prenderLed();
        }
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

