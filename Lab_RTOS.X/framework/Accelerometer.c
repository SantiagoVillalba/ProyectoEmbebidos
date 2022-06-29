/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Company Name

  @File Name
    filename.c

  @Summary
    Brief description of the file.

  @Description
    Describe the purpose of this file.
 */
/* ************************************************************************** */

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */
#include "Accelerometer.h"
#include <stdbool.h>
#include <stdio.h>
#include <math.h>


/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */

/* ************************************************************************** */
/** Descriptive Data Item Name

  @Summary
    Brief one-line summary of the data item.
    
  @Description
    Full description, explaining the purpose and usage of data item.
    <p>
    Additional description in consecutive paragraphs separated by HTML 
    paragraph breaks, as necessary.
    <p>
    Type "JavaDoc" in the "How Do I?" IDE toolbar for more information on tags.
    
  @Remarks
    Any additional remarks
 */
volatile static bool accel_action = false;
static bool accel_enable = false;
static float sensitivity = 0.004; //valor inicial por defecto
static LIS3DH_Mode_t modo = LIS3DH_POWER_DOWN; //valor inicial por defecto
static LIS3DH_Fullscale_t escala = LIS3DH_FULLSCALE_2; //valor inicial por defecto


/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */

/* ************************************************************************** */

/** 
  @Function
    int ExampleLocalFunctionName ( int param1, int param2 ) 

  @Summary
    Brief one-line description of the function.

  @Description
    Full description, explaining the purpose and usage of the function.
    <p>
    Additional description in consecutive paragraphs separated by HTML 
    paragraph breaks, as necessary.
    <p>
    Type "JavaDoc" in the "How Do I?" IDE toolbar for more information on tags.

  @Precondition
    List and describe any required preconditions. If there are no preconditions,
    enter "None."

  @Parameters
    @param param1 Describe the first parameter to the function.
    
    @param param2 Describe the second parameter to the function.

  @Returns
    List (if feasible) and describe the return values of the function.
    <ul>
      <li>1   Indicates an error occurred
      <li>0   Indicates an error did not occur
    </ul>

  @Remarks
    Describe any special behavior not described above.
    <p>
    Any additional remarks.

  @Example
    @code
    if(ExampleFunctionName(1, 2) == 0)
    {
        return 3;
    }
 */


/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */

// *****************************************************************************

/** 
  @Function
    int ExampleInterfaceFunctionName ( int param1, int param2 ) 

  @Summary
    Brief one-line description of the function.

  @Remarks
    Refer to the example_file.h interface header for function usage details.
 */

// esta funcion inicializa el acelerometro

bool ACCEL_init( void )
{
    uint8_t estado;
    uint8_t value;
    
    estado = LIS3DH_GetWHO_AM_I( &value );
    if( estado == 1 )
    {
        if( value != LIS3DH_DEVICE_ID )
        {
            return false;
        }
        #ifdef DEBUG_ACCEL
        //DEBUG_print("WHO AM I corecto\n");
        #endif
    }
    else
    {
       return false;
    }
    
    estado = LIS3DH_SetSIM(LIS3DH_SIM_4WIRE);
    if( estado == 1 )
    {
        #ifdef DEBUG_ACCEL
        //DEBUG_print("SET_4Wire\n"); //hay que ver donde imprimir esto
        #endif
    }
    else
    {
       return false;
    }
    
    //set ODR (turn ON device)
    estado = LIS3DH_SetODR(LIS3DH_ODR_50Hz);
    if( estado == 1 )
    {
        #ifdef DEBUG_ACCEL
        //DEBUG_print("SET_50Hz\n"); //hay que ver donde imprimir esto
        #endif
    }
    else
    {
        return false;
    }
    //set PowerMode 
    estado = LIS3DH_SetMode( LIS3DH_LOW_POWER );
    if( estado == 1 )
    {
        #ifdef DEBUG_ACCEL
        //DEBUG_print("SET_LP\n"); //hay que ver donde imprimir esto
        #endif
        modo = LIS3DH_LOW_POWER;
        sensitivity_update();
    }
    else
    {
        return false;
    }
    
    //set BDU
    estado = LIS3DH_SetBDU(MEMS_DISABLE);
    if( estado == 1 )
    {
        #ifdef DEBUG_ACCEL
        //DEBUG_print("SET_BDU_0\n"); //hay que ver donde imprimir esto
        #endif
    }
    else
    {
        return false;
    }
    
    //set BLE
    estado = LIS3DH_SetBLE(LIS3DH_BLE_LSB);
    if( estado == 1 )
    {
        #ifdef DEBUG_ACCEL
        //DEBUG_print("SET_BLE_LSB\n"); //hay que ver donde imprimir esto
        #endif
    }
    else
    {
        return false;
    }
    
    //set self test
    estado = LIS3DH_SetSelfTest(LIS3DH_SELF_TEST_DISABLE);
    if( estado == 1 )
    {
        #ifdef DEBUG_ACCEL
        //DEBUG_print("SET_SELF_TEST\n"); //hay que ver donde imprimir esto
        #endif
    }
    else
    {
        return false;
    }
    
    //set Fullscale
    estado = LIS3DH_SetFullScale(LIS3DH_FULLSCALE_8);
    if( estado == 1 )
    {
        #ifdef DEBUG_ACCEL
        //DEBUG_print("SET_SCALE_8g\n"); //hay que ver donde imprimir esto
        #endif
        escala = LIS3DH_FULLSCALE_8;
        sensitivity_update();
    }
    else
    {
        return false;
    }
    //set axis Enable
    estado = LIS3DH_SetAxis(LIS3DH_X_ENABLE | LIS3DH_Y_ENABLE | LIS3DH_Z_ENABLE);
    if( estado == 1 )
    {
        #ifdef DEBUG_ACCEL
        //DEBUG_print("SET_AXIS_XYZ\n"); //hay que ver donde imprimir esto
        #endif
    }
    else
    {
        return false;
    }
    estado = LIS3DH_FIFOModeEnable(LIS3DH_FIFO_BYPASS_MODE);
    if( estado == 1 )
    {
        #ifdef DEBUG_ACCEL
        //DEBUG_print("NO_FIFO\n"); //hay que ver donde imprimir esto
        #endif
    }
    else
    {
        return false;
    }
    return true;
}

#ifdef DEBUG_ACCEL
bool ACCEL_printData( void )
{
    uint8_t estado;
    AxesRaw_t data;
    
    estado = LIS3DH_GetAccAxesRaw(&data);
    if( estado == 1 )
    {
        estado++;
        /*
        sprintf( debugBuf, "X=%d Y=%d Z=%d\n", data.AXIS_X, data.AXIS_Y, data.AXIS_Z);
        DEBUG_print(debugBuf); //hay que ver donde imprimir esto
        */
    }
    else
    {
        return false;
    }
    return true;
}
#endif

//funcion que obtiene los datos de aceleracion del acelerometro
//son los datos crudos

bool ACCEL_GetData(AxesRaw_t *data)
{
    if( LIS3DH_GetAccAxesRaw(data) == 1 )
    {
        return true;
    }
    else
    {
        return false;
    }
}

//funcion que obtiene los datos en unidades de g del acelerometro usa las 3 coordenadas

bool ACCEL_GetAccel (Accel_t *accel)
{
    AxesRaw_t dato;
    float x,y,z;
    //float sensibilidad = 0.192; //este dato hay que hacerlo variable si cambio el modo de funcionamiento LP y 16g ahora
    
    if (ACCEL_GetData(&dato))
    {
        x = (float)(dato.AXIS_X>>8);
        y = (float)(dato.AXIS_Y>>8);
        z = (float)(dato.AXIS_Z>>8);
        accel->Accel_X = x*sensitivity;
        accel->Accel_Y = y*sensitivity;
        accel->Accel_Z = z*sensitivity;
        return true;
    }
    return false;
}

//funcion que retorna el modulo de la aceleracion

bool ACCEL_Mod (float *dato)
{
    Accel_t accel;
    if (ACCEL_GetAccel(&accel))
    {
        *dato = sqrt(accel.Accel_X*accel.Accel_X+accel.Accel_Y*accel.Accel_Y+accel.Accel_Z*accel.Accel_Z);
        return true;
    }
    return false;
}

//funcion que retorna el modulo de la aceleracion sacando el eje z.

bool ACCEL_Mod2D (float *dato)
{
    Accel_t accel;
    if (ACCEL_GetAccel(&accel))
    {
        *dato = sqrt(accel.Accel_X*accel.Accel_X+accel.Accel_Y*accel.Accel_Y);
        return true;
    }
    return false;
}

//funcion que obtiene la temperatura del acelerometro
//da la diferencia con respecto a un valor desconocido (pero cercano a la temp ambiente 25)
// si esta valor esta por encima de 50, seria complicado....

bool ACCEL_GetTemp(int8_t *data)
{
    if( LIS3DH_GetTempRaw(data) == 1 )
    {
        return true;
    }
    else
    {
        return false;
    }
}

// esta funcion chequea el estado de temperatura. retorna:
// -1: error
//  0: temperaura en valores normales
//  1: temperatura muy alta
int8_t Check_Temp()
{
    int8_t data;
    if (ACCEL_GetTemp(&data))
    {
        if (data>TEMPMAX)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return -1;
    }
}

// esta funcion permite el uso de el sensor de temperatura integrado del acelerometro

bool ACCEL_setupTemp()
{
   uint8_t estado;
   estado = LIS3DH_SetBDU(MEMS_ENABLE);
   if( estado == 1 )
   {
       #ifdef DEBUG_ACCEL
       //DEBUG_print("BDU enable\n"); //ver donde imprimir esto
       #endif
   }
   else
   {
       return false;
   }
   estado = LIS3DH_SetADCAux(MEMS_ENABLE);
   if( estado == 1 )
   {
       #ifdef DEBUG_ACCEL
       //DEBUG_print("Habilita ADC\n"); //ver donde imprimir esto
       #endif
   }
   else
   {
       return false;
   }
   estado = LIS3DH_SetTemperature(MEMS_ENABLE);
   if( estado == 1 )
   {
       #ifdef DEBUG_ACCEL
       //DEBUG_print("Habilita medida de temperatura\n"); //ver donde imprimir esto
       #endif
   }
   else
   {
       return false;
   }  
   
}

//esta funcion programa las interrupciones del accelerometro.

bool ACCEL_setupInt( LIS3DH_CLICK click, uint8_t clkth, uint8_t clklmt, uint8_t clklat, uint8_t clklim )
{
   uint8_t estado;
   
   estado = LIS3DH_SetIntMode(LIS3DH_INT_MODE_OR);
   if( estado == 1 )
   {
       #ifdef DEBUG_ACCEL
       //DEBUG_print("Mode OR\n"); //ver donde imprimir esto
       #endif
   }
   else
   {
       return false;
   }
   
   estado = LIS3DH_SetClickTHS(clkth); // 127 es maxima escala como esta 8G son 62mg por numero, puse g/4 por defecto
   if( estado == 1 )
   {
       #ifdef DEBUG_ACCEL
       //DEBUG_print("THS para click set\n"); //ver donde imprimir esto
       #endif
   }
   else
   {
       return false;
   }
   //timpo dura el click como maximo
   estado = LIS3DH_SetClickLIMIT(clklmt); // 127 es maxima escala como esta 10Hz puse chico (200ms)
   if( estado == 1 )
   {
       #ifdef DEBUG_ACCEL
       //DEBUG_print("limit para click set\n"); //ver donde imprimir esto
       #endif
   }
   else
   {
       return false;
   }
   //cada cuanto es un click para el doble click
   estado = LIS3DH_SetClickLATENCY(clklat); // 127 es maxima escala como esta 10Hz puse chico (200ms)
   if( estado == 1 )
   {
       #ifdef DEBUG_ACCEL
       //DEBUG_print("latency para click set\n"); //ver donde imprimir esto
       #endif
   }
   else
   {
       return false;
   }
   
   estado = LIS3DH_SetClickWINDOW(clklim); // 127 es maxima escala como esta 10Hz puse chico (400ms)
   if( estado == 1 )
   {
       #ifdef DEBUG_ACCEL
       //DEBUG_print("window para click set\n"); //ver donde imprimir esto
       #endif
   }
   else
   {
       return false;
   }
   //seteo interrupcion en el 1
   estado = LIS3DH_SetInt1Pin(LIS3DH_CLICK_ON_PIN_INT1_ENABLE | LIS3DH_I1_INT1_ON_PIN_INT1_DISABLE | LIS3DH_I1_INT2_ON_PIN_INT1_DISABLE | LIS3DH_I1_DRDY1_ON_INT1_DISABLE | LIS3DH_I1_DRDY2_ON_INT1_DISABLE | LIS3DH_WTM_ON_INT1_DISABLE | LIS3DH_INT1_OVERRUN_DISABLE );
   if( estado == 1 )
   {
       #ifdef DEBUG_ACCEL
       //DEBUG_print("SETUP DEL INT1 CUANDO HAY CLICK\n"); //ver donde imprimir esto
       #endif
   }
   else
   {
       return false;
   } 
   // configuro para que quede latcheada la interrupcion, no se si es lo mejor
   estado = LIS3DH_Int1LatchEnable(MEMS_ENABLE);
   if( estado == 1 )
   {
       #ifdef DEBUG_ACCEL
       //DEBUG_print("Latch enable\n"); //ver donde imprimir esto
       #endif
   }
   else
   {
       return false;
   } 
   //tipo de click
   if (click == LIS3DH_DOUBLE){ //configuro para click doble
       estado = LIS3DH_SetClickCFG( LIS3DH_ZD_ENABLE | LIS3DH_ZS_DISABLE | LIS3DH_YD_ENABLE | LIS3DH_YS_DISABLE | LIS3DH_XD_ENABLE | LIS3DH_XS_DISABLE);
   }
   else
   { //configuro para click simple
       estado = LIS3DH_SetClickCFG( LIS3DH_ZD_DISABLE | LIS3DH_ZS_ENABLE | LIS3DH_YD_DISABLE | LIS3DH_YS_ENABLE | LIS3DH_XD_DISABLE | LIS3DH_XS_ENABLE);
   }
   if( estado == 1 )
   {     
        #ifdef DEBUG_ACCEL
        //sprintf( debugBuf, "click configurado %i\n", click);
        //DEBUG_print(debugBuf); //ver donde imprimir esto
        #endif
   }
   else
   {
       return false;
   }
   
   //tipo de click
   if (click == LIS3DH_DOUBLE){ //configuro para click doble
       estado = LIS3DH_SetClickSrc( LIS3DH_DCLICK );
   }
   else
   { //configuro para click simple
       estado = LIS3DH_SetClickSrc( LIS3DH_SCLICK );
   }
   if( estado == 1 )
   {     
        #ifdef DEBUG_ACCEL
        //sprintf( debugBuf, "click configurado %i\n", click);
        //DEBUG_print(debugBuf); //ver donde imprimir esto
        #endif
   }
   else
   {
       return false;
   }
   return true;
}

LIS3DH_CLICK ACCEL_intHandler( void )
{
    accel_action = true;
    return true;
}

void ACCEL_setAction( bool p_flag )
{
    accel_action = p_flag;
}

bool ACCEL_getAction( void )
{
    return accel_action;
}

void ACCEL_setEnable( bool p_en )
{
    accel_enable = p_en;
}

bool ACCEL_getEnable( void )
{
    return accel_enable;
}

void sensitivity_update()
{
    if (modo == LIS3DH_POWER_DOWN)
    {
        sensitivity = 0.0;
    }
    else
    {
        switch (escala)
        {
            case (LIS3DH_FULLSCALE_2):
                if (modo == LIS3DH_LOW_POWER)
                {
                    sensitivity = 0.016;
                }
                else
                {
                    sensitivity = 0.001;
                }
                break;
                    
            case (LIS3DH_FULLSCALE_4):
                if (modo == LIS3DH_LOW_POWER)
                {
                    sensitivity = 0.032;
                }
                else
                {
                    sensitivity = 0.002;
                }
                break;
                            
            case (LIS3DH_FULLSCALE_8):
                if (modo == LIS3DH_LOW_POWER)
                {
                    sensitivity = 0.064;
                }
                else
                {
                    sensitivity = 0.004;
                }
                break;
                            
            case (LIS3DH_FULLSCALE_16):
                if (modo == LIS3DH_LOW_POWER)
                {
                    sensitivity = 0.192;
                }
                else
                {
                    sensitivity = 0.012;
                }
                break;
        } 
    }
    
}
/* *****************************************************************************
 End of File
 */
