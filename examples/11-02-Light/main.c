/******************************************************************
 * Hubert Data Logger
 * Light Sensor Example
 * Developed by Sevun Scientific, Inc.
 * http://sevunscientific.com
 * *****************************************************************
 *
 *    _____/\\\\\\\\\\\_______/\\\\\\\\\\\____/\\\\\\\\\\\_
 *     ___/\\\/////////\\\___/\\\/////////\\\_\/////\\\///__
 *      __\//\\\______\///___\//\\\______\///______\/\\\_____
 *       ___\////\\\___________\////\\\_____________\/\\\_____
 *        ______\////\\\___________\////\\\__________\/\\\_____
 *         _________\////\\\___________\////\\\_______\/\\\_____
 *          __/\\\______\//\\\___/\\\______\//\\\______\/\\\_____
 *           _\///\\\\\\\\\\\/___\///\\\\\\\\\\\/____/\\\\\\\\\\\_
 *            ___\///////////_______\///////////_____\///////////__
 *
 * *****************************************************************
 * https://eewiki.net/display/microcontroller/I2C+Communication+with+the+TI+Tiva+TM4C123GXL
 */


#include <stdint.h>
#include <stdbool.h>
#include "utils/uartstdio.h"
#include "inc/hw_gpio.h"
#include "inc/hw_i2c.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/i2c.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include <ls/APDS_9303.h>
#include <ls/APDS_9303_proc.h>

// Define UART speed in kbs
#define UART_SPEED                  115200

// Define APDS-9303 I2C address, determined by PCB layout with pin ADDR = 0 (R18 pulled to GND)
#define LS_SLAVE_ADDR       0x29      //Light Sensor



t_GainSetting                g_tgainSet;
t_IntegrationTimeSetting     g_tintegTimeSet;
t_aRawData                   g_tarrVal;
t_fRawData                   g_tfluxVal;
t_RawData                    g_ui8Val;

#define DEBUG_PRINT 1                             //**Note: Comment out this line to disable debug print statements

uint16_t temperatureLeft = 0;
uint16_t temperatureRight = 0;
float temperatureTemp = 0.0;

volatile bool g_bLightSensorInterruptFlag = 0;          // Flag to indicate Interrupt from Light Sensor


//*****************************************************************************
// I2C Functions
//*****************************************************************************



//*****************************************************************************
// Timer Interrupt
//*****************************************************************************
volatile bool g_bTimer0Flag = 0;        // Timer 0 occurred flag

// The interrupt handler for the first timer interrupt. 1 Hz
void Timer0IntHandler(void)
{
    // Clear the timer interrupt.
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    g_bTimer0Flag = 1;      // Set the flag for Timer 0 interrupt
}


//*****************************************************************************
// Print Float Function
//*****************************************************************************
void printFloat(t_fRawData *tRD)
{
    uint8_t i=0;
    float LightSensorVal;
    int32_t i32IntegerPart;                           //used to split the float into integer and fraction part for easy printing
    int32_t i32FractionPart;

    for(i=0;i<4;i++){

        if(i==0){
            LightSensorVal = tRD->luxVal;
        }
        if(i==1){
            #ifdef  DEBUG_PRINT
                LightSensorVal = tRD->fChan_0;
            #endif
        }
        else if(i==2){
            #ifdef  DEBUG_PRINT
                LightSensorVal = tRD->fChan_1;
            #endif
        }
        else if(i==3){
            #ifdef  DEBUG_PRINT
                LightSensorVal = tRD->fRatio;
            #endif
        }
        else{}

        i32IntegerPart = (int32_t)LightSensorVal;
        i32FractionPart = (int32_t)(LightSensorVal * 1000.0f);
        i32FractionPart = i32FractionPart - (i32IntegerPart * 1000);

        if(i32FractionPart < 0)
        {
            i32FractionPart *= -1;
        }

        if(i==0){
            UARTprintf("\r\nLight Sensor Flux: ");
        }
        if(i==1){
            #ifdef  DEBUG_PRINT
                UARTprintf("\r\nfChan_0: ");
            #endif
        }
        else if(i==2){
            #ifdef  DEBUG_PRINT
                UARTprintf("\r\nfChan_1: ");
            #endif
        }
        else if(i==3){
            #ifdef  DEBUG_PRINT
                UARTprintf("\r\nfRatio: ");
            #endif
        }
        else{}


        //

        UARTprintf("%3d.%02d", i32IntegerPart, i32FractionPart);

    }


}




//*****************************************************************************
// Main Loop
//*****************************************************************************
int main(void){

    uint8_t ui32Data[2];
    uint8_t GPIOReadVal = 1;
    bool callStatus = 0;
    uint8_t ui8callStatus = 0;
    float luxValue = 0.0;

    // Set microcontroller to use the 16 MHz external crystal
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    //*****************************************************************************
    // Pins Setup
    //*****************************************************************************

    // Enable the Port C peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C1);

    // Sets the pin associated with IND1 output
    GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_4);
    GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_5);

    // Sets the pin associated with Light Sensor ALERT input
    GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_2);


    // Sets the pins associated with UART0
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Configure the GPIO Pin Mux for PA6 for I2C1SCL
    GPIOPinConfigure(GPIO_PA6_I2C1SCL);
    GPIOPinTypeI2CSCL(GPIO_PORTA_BASE, GPIO_PIN_6);

    // Configure the GPIO Pin Mux for PA7 for I2C1SDA
    GPIOPinConfigure(GPIO_PA7_I2C1SDA);
    GPIOPinTypeI2C(GPIO_PORTA_BASE, GPIO_PIN_7);

    //*****************************************************************************
    // Timer Setup
    //*****************************************************************************

    // Enable the peripherals used by this example.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

    // Configure the two 32-bit periodic timers.
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet() / 2);      // 2 Hz rate

    // Setup the interrupts for the timer timeouts.
    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    // Enable the timers.
    TimerEnable(TIMER0_BASE, TIMER_A);

    //*****************************************************************************
    // UART Setup
    //*****************************************************************************

    // Initialize the UART0 using uartstdio
    UARTStdioConfig(0, UART_SPEED, SysCtlClockGet());

    //*****************************************************************************
    // I2C Setup
    //*****************************************************************************

    // Set the clock speed for the I2C1 bus
    I2CMasterInitExpClk(I2C1_BASE, SysCtlClockGet(), false);

    // The I2C1 peripheral must be enabled before use.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C1);

    //*****************************************************************************
    // Configure Interrupts
    //*****************************************************************************

    // Enable processor interrupts.
    IntMasterEnable();

    //*****************************************************************************
    // Main Code
    //*****************************************************************************

    // Clear and reset home screen
    UARTprintf("\033[2J\033[;H");
    UARTprintf("\r\nHubert is stirring\r\n");

    //set pointers for ----------

    t_GainSetting *GptrTmp;
    t_RawData *ptrTmpInit;
    t_aRawData *ptrTmpThresh;

    GptrTmp = &g_tgainSet;
    ptrTmpInit = &g_ui8Val;
    ptrTmpThresh = &g_tarrVal;

    //---------------------------


    APDS_9303_PowerEnable(LS_SLAVE_ADDR, POWER_ON);
    APDS_9303_init(LS_SLAVE_ADDR, &g_ui8Val);                                                                              //initialize the light sensor

    UARTprintf("Powering Up Chip %02x\r\n", ptrTmpInit->ui8ResultVal);


    APDS_9303_getID(LS_SLAVE_ADDR, LIGHT_DATA, &g_ui8Val);
    if( callStatus < 1){
        UARTprintf("Error setting Gain Level %d\r\n", callStatus);
    }
    else{
        UARTprintf("OK setting Gain Level %d\r\n", callStatus);
    }

    UARTprintf("Chip ID: %02x\r\n", ptrTmpInit->ui8ResultVal);


    callStatus = APDS_9303_SetGain(LS_SLAVE_ADDR, GAIN_LOW, &g_tgainSet);                                                   //set the gain to low level
    if( callStatus < 1){
        UARTprintf("Error setting Gain Level %d\r\n", callStatus);
    }
    else{
        UARTprintf("OK setting Gain Level %d\r\n", callStatus);
    }

    UARTprintf("\r\ng_tgainSet: %02x", GptrTmp->gainSetting);



    t_IntegrationTimeSetting *IptrTmp;
    IptrTmp = &g_tintegTimeSet;

    callStatus = APDS_9303_SetIntegrationTime(LS_SLAVE_ADDR, INTEGRATION_TIME_13_7_MS, &g_tintegTimeSet);                   //set integration time to shortest
    if(!callStatus){
        UARTprintf("\r\nError setting Integration Time %d\r\n", IptrTmp->integTimeSetting);
    }
    else{
        UARTprintf("\r\nOK setting Integration Time %d\r\n", IptrTmp->integTimeSetting);
    }



    uint8_t ui8DataReg[2];
    ptrTmpThresh->ui8ArrVal[0] = 0x00;
    ptrTmpThresh->ui8ArrVal[1] = 0x00;
    callStatus = APDS_9303_SetLowThreshold(LS_SLAVE_ADDR, &g_tarrVal);                                                      //set the low threshold to 0 to disable
    if(!callStatus){
        UARTprintf("\r\nError setting Low Threshold: %d\r\n", ptrTmpThresh->ui8ArrVal[0]);
    }
    else{
        UARTprintf("\r\nOK setting Low_LOW Threshold %02x\r\n", ptrTmpThresh->ui8ArrVal[0]);
        UARTprintf("OK setting Low_HIGH Threshold %02x\r\n", ptrTmpThresh->ui8ArrVal[1]);
    }




    ptrTmpThresh->ui8ArrVal[0] = 0xF0;
    ptrTmpThresh->ui8ArrVal[1] = 0x02;
    callStatus = APDS_9303_SetHighThreshold(LS_SLAVE_ADDR, &g_tarrVal);                                       //set the high threshold to 432dec or 1B0
    if(!callStatus){
        UARTprintf("\r\nError setting High Threshold: %d\r\n", ptrTmpThresh->ui8ArrVal[0]);
    }
    else{
        UARTprintf("\r\nOK setting High_LOW Threshold %02x\r\n", ptrTmpThresh->ui8ArrVal[0]);
        UARTprintf("OK setting High_HIGH Threshold %02x\r\n", ptrTmpThresh->ui8ArrVal[1]);
    }




    uint8_t cyclesVal;
    cyclesVal = 1;
    ptrTmpInit = &g_ui8Val;
    callStatus = APDS_9303_SetInterruptCycles(LS_SLAVE_ADDR,  cyclesVal, &g_ui8Val);                               //set interrupt cycles to one
    if(!callStatus){
        UARTprintf("\r\nError could not set Interrupt Cycles %02x\r\n", ptrTmpInit->ui8ResultVal);
    }
    else{
        UARTprintf("\r\nOK setting Interrupt Cycles %02x\r\n", ptrTmpInit->ui8ResultVal);
    }


    callStatus = APDS_9303_EnableInterruptMode(LS_SLAVE_ADDR, INTERRUPT_ON, &g_ui8Val);
    if(!callStatus){
        UARTprintf("\r\nError enabling Interrupts %02x\r\n", ptrTmpInit->ui8ResultVal);
    }
    else{
        UARTprintf("\r\nOK setting Interrupts ON %02x\r\n", ptrTmpInit->ui8ResultVal);
    }




    callStatus = APDS_9303_ClearInterruptFlag(LS_SLAVE_ADDR, INTERRUPT_CLEAR);
    if(!callStatus){
        UARTprintf("\r\nError clearing threshold Interrupt\r\n");
    }
    else{
        UARTprintf("\r\nOK clearing threshold Interrupt\r\n");
    }


    uint8_t loopVal = 1;        //enter 0 to disable while loop below to see info above


    while(loopVal)
    {
        // Timer 0
        if ( 1 == g_bTimer0Flag )
        {
            g_bTimer0Flag = 0;  // Clear the flag for Timer 0 interrupt


            UARTprintf("\r\n---------------------------------\r\n");

            t_fRawData *Fptr;
            t_IntegrationTimeSetting *Iptr;

            ui8callStatus = APDS_9303_getLuxData(LS_SLAVE_ADDR, LUX_DATA, &g_tintegTimeSet, &g_tfluxVal, GAIN_LOW);

            Fptr = &g_tfluxVal;
            Iptr = &g_tintegTimeSet;

            #ifdef DEBUG_PRINT
                   UARTprintf("\r\nui8callStatus: %d", ui8callStatus);
                   UARTprintf("\r\nui8Chan_0_0: %02x", Fptr->ui8Chan_0_0);
                   UARTprintf("\r\nui8Chan_0_1: %02x", Fptr->ui8Chan_0_1);
                   UARTprintf("\r\nui8Chan_1_0: %02x", Fptr->ui8Chan_1_0);
                   UARTprintf("\r\nui8Chan_1_1: %02x", Fptr->ui8Chan_1_1);
                   UARTprintf("\r\nf16Chan_0: %02x", Fptr->f16Chan_0);
                   UARTprintf("\r\nf16Chan_1: %02x", Fptr->f16Chan_1);
                   UARTprintf("\r\ng_tintegTimeSet: %d", Iptr->integTimeSetting);
            #endif

            if(!callStatus){
                UARTprintf("\r\nError reading Lux Data\r\n");
            }
            else{
                UARTprintf("\r\nOK Lux Data Read Correctly\r\n");
                printFloat(&g_tfluxVal);
            }


            #ifdef DEBUG_PRINT
                UARTprintf("\r\nGPIO_PIN_2:%02x", GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_2));
            #endif

            //PD2 is Light Sensor Interrupt Pin (Poll for interrupt)
            GPIOReadVal = GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_2);
            if (!GPIOReadVal)
            {
                g_bLightSensorInterruptFlag = 1;
                UARTprintf("\r\nInterrupt Detected on Light Sensor");
            }
            else{
                g_bLightSensorInterruptFlag = 0;
                UARTprintf("\r\nNO Interrupt on Light Sensor");
            }


            //Turn IND2 ON or OFF if either Alert Pin is active (LOW)
            if ( g_bLightSensorInterruptFlag )
            {
                // Writes HIGH to pins
                GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, GPIO_PIN_5);           // IND2 LED ON
                #ifdef DEBUG_PRINT
                    UARTprintf("\r\nTuring ON LED IND2");
                #endif
            }
            else
            {
                // Writes LOW to pins
                GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, 0);  // IND2 LED OFF
                #ifdef DEBUG_PRINT
                    UARTprintf("\r\nTuring OFF LED IND2");
                #endif
            }




            // Check the current value of the pin, then flip it
            if ( GPIO_PIN_4 == GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_4) )
            {
                // Writes HIGH to pins
                GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, 0);           // IND1 LED Off
            }
            else
            {
                // Writes HIGH to pins
                GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, GPIO_PIN_4);  // IND1 LED On
            }


            callStatus = APDS_9303_ClearInterruptFlag(LS_SLAVE_ADDR, INTERRUPT_CLEAR);
            if(!callStatus){
                UARTprintf("\r\nError clearing threshold Interrupt\r\n");
            }
            else{
                UARTprintf("\r\nOK clearing threshold Interrupt\r\n");
            }


           UARTprintf("\r\n---------------------------------\r\n");




        }

    }





}




