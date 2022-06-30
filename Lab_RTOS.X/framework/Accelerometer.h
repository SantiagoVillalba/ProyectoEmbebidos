/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Company Name

  @File Name
    filename.h

  @Summary
    Brief description of the file.

  @Description
    Describe the purpose of this file.
 */
/* ************************************************************************** */

#ifndef _ACCELEROMETER_H    /* Guard against multiple inclusion */
#define _ACCELEROMETER_H


/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */
#include <stdbool.h>
#include <stdint.h>
#include "../../platform/LIS3DH.h"


/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Constants                                                         */
/* ************************************************************************** */
/* ************************************************************************** */

/* ************************************************************************** */
/** Descriptive Constant Name

  @Summary
    Brief one-line summary of the constant.

  @Description
    Full description, explaining the purpose and usage of the constant.
    <p>
    Additional description in consecutive paragraphs separated by HTML 
    paragraph breaks, as necessary.
    <p>
    Type "JavaDoc" in the "How Do I?" IDE toolbar for more information on tags.

  @Remarks
    Any additional remarks
 */
//#define EXAMPLE_CONSTANT 0

#define TEMPMAX 50

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************

/** Descriptive Data Type Name

  @Summary
    Brief one-line summary of the data type.

  @Description
    Full description, explaining the purpose and usage of the data type.
    <p>
    Additional description in consecutive paragraphs separated by HTML 
    paragraph breaks, as necessary.
    <p>
    Type "JavaDoc" in the "How Do I?" IDE toolbar for more information on tags.

  @Remarks
    Any additional remarks
    <p>
    Describe enumeration elements and structure and union members above each 
    element or member.
 */

typedef enum
{
    LIS3DH_SINGLE,
    LIS3DH_DOUBLE
}LIS3DH_CLICK;


typedef struct 
{
    float Accel_X;
    float Accel_Y;
    float Accel_Z;
} Accel_t;

// *****************************************************************************
// *****************************************************************************
// Section: Interface Functions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/**
  @Function
    int ExampleFunctionName ( int param1, int param2 ) 

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
bool ACCEL_init( void );
#ifdef DEBUG_ACCEL
bool ACCEL_printData( void );
#endif
bool ACCEL_setupInt( LIS3DH_CLICK click, uint8_t clkth, uint8_t clklmt, uint8_t clklat, uint8_t clklim );

LIS3DH_CLICK ACCEL_intHandler( void );
void ACCEL_setAction( bool p_flag );
bool ACCEL_getAction( void );

void ACCEL_setEnable( bool p_en );
bool ACCEL_getEnable( void );

bool ACCEL_GetAccel (Accel_t *accel);
bool ACCEL_GetData(AxesRaw_t *data);

bool ACCEL_GetTemp(int8_t *data);
int8_t Check_Temp();
bool ACCEL_setupTemp();

void sensitivity_update();

bool ACCEL_Mod2D (float *dato);
bool ACCEL_Mod (float *dato);
#endif /* _ACCELEROMETER_H */

/* *****************************************************************************
 End of File
 */
