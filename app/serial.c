#include "serial.h"
#include "bsp.h" 

static void Serial_SingleFrameTx( uint8_t *data, uint8_t size )
{
    if(size > 7)
    {
        size = 7;
    }

    for (uint8_t i = size; i > 0; i--)
    {
        *(data + i) = *(data + i -1);
    }

    *(data) = size;
}

static uint8_t Serial_SingleFrameRx( uint8_t *data, uint8_t *size)
{
    uint8_t varRet;
    *size = data[0] & 0x0F;

    if ( ( (data[0] & 0xF0) == 0 ) || ( (*size > 0) || (*size < 8) ) )  /*check if its a valid single frame*/
    {
        varRet = *size;                         /*if it is, return the size*/
        for (uint8_t i = 0; i < *size; i++)     /*and remove the first byte of data*/
        {
            data[i] = data[i + 1];
        }
        
    }else{
        varRet = 0;                             /*if it isn't just return 0*/
    }
    
    return varRet;
}

void Serial_InitTask( void )
{

}

void Serial_PeriodicTask( void )
{
    
}