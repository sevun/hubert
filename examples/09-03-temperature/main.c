/******************************************************************
 * Hubert Data Logger
 * Temperature Right and Left Example
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
#include "th/TMP102AIDRLT_proc.h"
#include "th/TMP102AIDRLT.h"

// Define UART speed in kbs
#define UART_SPEED                  115200

// Define TMP102AIDRLT I2C address, determined by PCB layout with pin ADD0=1
#define TH_L_SLAVE_ADDR       0x49      //Left Sensor
#define TH_R_SLAVE_ADDR       0x48      //Right Sensor

#define Celcius 0
#define Fahrenheit 1

//#define DEBUG_PRINT 1                             //**Note: Comment out this line to disable debug print statements

//tRawData g_tAccelData;
//tRawData g_tMagData;

uint16_t temperatureLeft = 0;
uint16_t temperatureRight = 0;
float temperatureTemp = 0.0;

volatile bool g_bTempLeftNegativeFlag = 0;        // Flag to indicate negative temperature on left temperature sensor
volatile bool g_bTempRightNegativeFlag = 0;       // Flag to indicate negative temperature on Right temperature sensor

volatile bool g_bTempRightAlertFlag = 0;          // Flag to indicate Temperature Alert Pin Right
volatile bool g_bTempLeftAlertFlag = 0;           // Flag to indicate Temperature Alert Pin Left

volatile bool temperatureSetNegativeFlag = 0;      // Flag to indicate a negative temperature for setting High and Low Limits 1 = Negative, 0 = Positive

float tempResolution = 0.0625;

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
void printFloat(float temperatureVal, bool unitsFlag, bool sensorSelect)
{
    char unitsStr;
    int32_t i32IntegerPart;                           //used to split the float into integer and fraction part for easy printing
    int32_t i32FractionPart;

    if(unitsFlag == Fahrenheit){                      //Fahrenheit selected so convert
        temperatureVal = (temperatureVal * 1.8) + 32;
    }

    i32IntegerPart = (int32_t)temperatureVal;
    i32FractionPart = (int32_t)(temperatureVal * 1000.0f);
    i32FractionPart = i32FractionPart - (i32IntegerPart * 1000);

    if(i32FractionPart < 0)
    {
        i32FractionPart *= -1;
    }


    if(!sensorSelect){
        UARTprintf("\r\ntemperature_Right = ");
    }
    else{
        UARTprintf("\r\ntemperature_Left =  ");
    }
    UARTprintf("%3d.%02d", i32IntegerPart, i32FractionPart);


    if(!unitsFlag){
        unitsStr = 'C';
        UARTprintf(" %c", unitsStr);
    }
    else{
        unitsStr = 'F';
        UARTprintf(" %c", unitsStr);
    }


}




//*****************************************************************************
// Main Loop
//*****************************************************************************
int main(void){

    bool sensorSelect = 0;          //default 0 = Right Sensor, 1 = Left Sensor
    uint8_t ui32Data[2];
    uint8_t GPIOReadVal = 1;
    uint16_t temperatureSet = 0;


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

    // Sets the pin associated with ALERT_R input
    GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_1);

    // Sets the pin associated with ALERT_L input
    GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_3);

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
    UARTprintf("Hubert is stirring\r\n");

    //NOTE: Using default 12Bit format, For extended 13Bit format, additional configuration settings are required along with modified formulas below, refer to data sheet
    //set temperature R and L High and Low limits for example, respective Alert Pins are triggered when these values are reached or exceeded

    //------------------------RIGHT TEMPERATURE CONFIGURATION------------------------//

    //set the configuration register to enable Alert pin
    I2CTHReceive(TH_R_SLAVE_ADDR, PR_CONFIG_REG, ui32Data, sizeof(ui32Data));         //read the configuration register to get default settings

    #ifdef DEBUG_PRINT
        UARTprintf("\r\nPR_CONFIG_REG[0]:    = 0x%02x",ui32Data[0]);                  //configuration register BYTE 1
        UARTprintf("\r\nPR_CONFIG_REG[1]:    = 0x%02x",ui32Data[1]);                  //configuration register BYTE 2
    #endif

    //ui32Data[0] &= ~CONFIG_TM_BIT;                                                    //Set TM Mode to Comparator
    ui32Data[0] |= CONFIG_TM_BIT;                                                     //Set TM Mode to Interrupt

    #ifdef DEBUG_PRINT
        UARTprintf("\r\nPR_CONFIG_REG[0]:    = 0x%02x",ui32Data[0]);
    #endif
    I2CTHSend(TH_R_SLAVE_ADDR, PR_CONFIG_REG, ui32Data, sizeof(ui32Data));            //send the configuration bytes back to the chip


    //------------------------LEFT TEMPERATURE CONFIGURATION------------------------//

    //set the configuration register to enable Alert pin
   I2CTHReceive(TH_L_SLAVE_ADDR, PR_CONFIG_REG, ui32Data, sizeof(ui32Data));         //read the configuration register to get default settings

   #ifdef DEBUG_PRINT
       UARTprintf("\r\nPR_CONFIG_REG[0]:    = 0x%02x",ui32Data[0]);                      //configuration register BYTE 1
       UARTprintf("\r\nPR_CONFIG_REG[1]:    = 0x%02x",ui32Data[1]);                      //configuration register BYTE 2
   #endif

   //ui32Data[0] &= ~CONFIG_TM_BIT;                                                //Set TM Mode to Comparator
   ui32Data[0] |= CONFIG_TM_BIT;                                                 //Set TM Mode to Interrupt

   #ifdef DEBUG_PRINT
       UARTprintf("\r\nPR_CONFIG_REG[0]:    = 0x%02x",ui32Data[0]);
   #endif
   I2CTHSend(TH_L_SLAVE_ADDR, PR_CONFIG_REG, ui32Data, sizeof(ui32Data));            //send the configuration bytes back to the chip



    //--------------------------------------------------------------------------------------------------------------------------------------------------------------//
    //-------------------------------------------------------------------RIGHT TEMPERATURE SENSOR-------------------------------------------------------------------//
    //--------------------------------------------------------------------------------------------------------------------------------------------------------------//

    //--Temperature High Limit--//
    temperatureSetNegativeFlag = 0;                                       //to set a negative temperature for High and Low limit, set this flag to 1, else 0
    temperatureSet = (27) / tempResolution;                               //enter the required temperature in Degrees C set value in brackets (non decimal)
                                                                          //Touching your finger on the sensor will increase the reading to approx 28 C

    if(temperatureSetNegativeFlag){
        temperatureSet = (~(temperatureSet)) + 1;                        //generate two's complement and add 1
        temperatureSet &=(0B0000111111111111);
    }

    UARTprintf("\r\ntemperatureSet:    = 0x%02x",temperatureSet);
    ui32Data[0] = temperatureSet >> 4;                                    //move Hi Byte to array 0
    ui32Data[1] = (temperatureSet << 8) & 0xF0;                           //move Lo Byte to array 1 and left justify as lowest four bits are 0 in 12Bit temperature format
    #ifdef DEBUG_PRINT
        UARTprintf("\r\nui32Data[0]:    = 0x%02x",ui32Data[0]);
        UARTprintf("\r\nui32Data[1]:    = 0x%02x",ui32Data[1]);
    #endif

    I2CTHSend(TH_R_SLAVE_ADDR, PR_TEMP_HIGH, ui32Data, sizeof(ui32Data));

    ui32Data[0] = 0;
    ui32Data[1] = 0;
    I2CTHReceive(TH_R_SLAVE_ADDR, PR_TEMP_HIGH, ui32Data, sizeof(ui32Data));

    #ifdef DEBUG_PRINT
        UARTprintf("\r\nTH_R_SLAVE_ADDR[0]:    = 0x%02x",ui32Data[0]);
        UARTprintf("\r\nTH_R_SLAVE_ADDR[1]:    = 0x%02x",ui32Data[1]);
    #endif



    //--Temperature Low Limit--//
    temperatureSet = 0;                                                   //clear the temporary register
    temperatureSet = (24) / tempResolution;                               //enter the required temperature set value in brackets (non decimal) to clear the Alert bit
                                                                          //The Alarm Pin will not go off until the temperature has reached this value if High Limit has been exceeded

    if(temperatureSetNegativeFlag){
        temperatureSet = (~(temperatureSet)) + 1;                         //generate two's complement and add 1
        temperatureSet &=(0B0000111111111111);
    }

    UARTprintf("\r\ntemperatureSet:    = 0x%02x",temperatureSet);
    ui32Data[0] = temperatureSet >> 4;
    ui32Data[1] = (temperatureSet << 8) & 0xF0;
    #ifdef DEBUG_PRINT
        UARTprintf("\r\nui32Data[0]:    = 0x%02x",ui32Data[0]);
        UARTprintf("\r\nui32Data[1]:    = 0x%02x",ui32Data[1]);
    #endif

    I2CTHSend(TH_R_SLAVE_ADDR, PR_TEMP_LOW, ui32Data, sizeof(ui32Data));

    ui32Data[0] = 0;
    ui32Data[1] = 0;
    I2CTHReceive(TH_R_SLAVE_ADDR, PR_TEMP_LOW, ui32Data, sizeof(ui32Data));
    #ifdef DEBUG_PRINT
        UARTprintf("\r\nTH_R_SLAVE_ADDR[0]:    = 0x%02x",ui32Data[0]);
        UARTprintf("\r\nTH_R_SLAVE_ADDR[1]:    = 0x%02x",ui32Data[1]);
    #endif

    //--------------------------------------------------------------------------------------------------------------------------------------------------------------//
    //--------------------------------------------------------------------LEFT TEMPERATURE SENSOR-------------------------------------------------------------------//
    //--------------------------------------------------------------------------------------------------------------------------------------------------------------//
    //--Temperature High Limit--//
    temperatureSetNegativeFlag = 0;                                       //to set a negative temperature for High and Low limit, set this flag to 1, else 0
    temperatureSet = (27) / tempResolution;                               //enter the required temperature in Degrees C set value in brackets (non decimal)
                                                                          //Touching your finger on the sensor will increase the reading to approx 28 C

    if(temperatureSetNegativeFlag){
        temperatureSet = (~(temperatureSet)) + 1;                        //generate two's complement and add 1
        temperatureSet &=(0B0000111111111111);
    }

    UARTprintf("\r\ntemperatureSet:    = 0x%02x",temperatureSet);
    ui32Data[0] = temperatureSet >> 4;                                    //move Hi Byte to array 0
    ui32Data[1] = (temperatureSet << 8) & 0xF0;                           //move Lo Byte to array 1 and left justify as lowest four bits are 0 in 12Bit temperature format
    #ifdef DEBUG_PRINT
        UARTprintf("\r\nui32Data[0]:    = 0x%02x",ui32Data[0]);
        UARTprintf("\r\nui32Data[1]:    = 0x%02x",ui32Data[1]);
    #endif

    I2CTHSend(TH_L_SLAVE_ADDR, PR_TEMP_HIGH, ui32Data, sizeof(ui32Data));

    ui32Data[0] = 0;
    ui32Data[1] = 0;
    I2CTHReceive(TH_L_SLAVE_ADDR, PR_TEMP_HIGH, ui32Data, sizeof(ui32Data));

    #ifdef DEBUG_PRINT
        UARTprintf("\r\nTH_R_SLAVE_ADDR[0]:    = 0x%02x",ui32Data[0]);
        UARTprintf("\r\nTH_R_SLAVE_ADDR[1]:    = 0x%02x",ui32Data[1]);
    #endif



    //--Temperature Low Limit--//
    temperatureSet = 0;                                                   //clear the temporary register
    temperatureSet = (24) / tempResolution;                               //enter the required temperature set value in brackets (non decimal) to clear the Alert bit
                                                                          //The Alarm Pin will not go off until the temperature has reached this value if High Limit has been exceeded

    if(temperatureSetNegativeFlag){
        temperatureSet = (~(temperatureSet)) + 1;                         //generate two's complement and add 1
        temperatureSet &=(0B0000111111111111);
    }

    UARTprintf("\r\ntemperatureSet:    = 0x%02x",temperatureSet);
    ui32Data[0] = temperatureSet >> 4;
    ui32Data[1] = (temperatureSet << 8) & 0xF0;
    #ifdef DEBUG_PRINT
        UARTprintf("\r\nui32Data[0]:    = 0x%02x",ui32Data[0]);
        UARTprintf("\r\nui32Data[1]:    = 0x%02x",ui32Data[1]);
    #endif

    I2CTHSend(TH_L_SLAVE_ADDR, PR_TEMP_LOW, ui32Data, sizeof(ui32Data));

    ui32Data[0] = 0;
    ui32Data[1] = 0;
    I2CTHReceive(TH_L_SLAVE_ADDR, PR_TEMP_LOW, ui32Data, sizeof(ui32Data));
    #ifdef DEBUG_PRINT
        UARTprintf("\r\nTH_R_SLAVE_ADDR[0]:    = 0x%02x",ui32Data[0]);
        UARTprintf("\r\nTH_R_SLAVE_ADDR[1]:    = 0x%02x",ui32Data[1]);
    #endif



    while(1)
    {
        // Timer 0
        if ( 1 == g_bTimer0Flag )
        {
            g_bTimer0Flag = 0;  // Clear the flag for Timer 0 interrupt

            #ifdef DEBUG_PRINT
                UARTprintf("\r\nGPIO_PIN_1:%02x", GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_1));
                UARTprintf("\r\nGPIO_PIN_3:%02x", GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_3));
            #endif

            //PD1 is Right Temperature Alert Pin
            GPIOReadVal = GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_1);
            if (!GPIOReadVal)
            {
                g_bTempRightAlertFlag = 1;           // Set Temperature Right Alert Flag
            }
            else{
                g_bTempRightAlertFlag = 0;           // Clear Temperature Right Alert Flag
            }

            //PD3 is Left Temperature Alert Pin
            GPIOReadVal = GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_3);
            if (!GPIOReadVal)
            {
                g_bTempLeftAlertFlag = 1;           // Set Temperature Left Alert Flag
            }
            else{
                g_bTempLeftAlertFlag = 0;           // Clear Temperature Left Alert Flag
            }

            //Turn IND2 ON or OFF if either Alert Pin is active (LOW)
            if ( (g_bTempRightAlertFlag) || (g_bTempLeftAlertFlag) )
            {
                // Writes HIGH to pins
                GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, GPIO_PIN_5);           // IND2 LED Off
                #ifdef DEBUG_PRINT
                    UARTprintf("\r\nTuring ON LED IND2");
                #endif
            }
            else
            {
                // Writes LOW to pins
                GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, 0);  // IND2 LED On
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

            ui32Data[0] = 0;
            ui32Data[1] = 0;
           // ***********************Print Right temperature register values for testing feedback
           I2CTHReceive(TH_R_SLAVE_ADDR, PR_TEMP_REG, ui32Data, sizeof(ui32Data));

           #ifdef DEBUG_PRINT
               UARTprintf("\r\nTH_R_SLAVE_ADDR[0]:    = 0x%02x",ui32Data[0]);
               UARTprintf("\r\nTH_R_SLAVE_ADDR[1]:    = 0x%02x",ui32Data[1]);
           #endif


           if(ui32Data[0] & 0x80){
               g_bTempRightNegativeFlag = 1;       //negative temperature
           }
           else{
               g_bTempRightNegativeFlag = 0;       //positive temperature
           }

           temperatureRight = (int16_t)((ui32Data[0] << 4) | (ui32Data[1] >> 4));

           if(!g_bTempRightNegativeFlag){
               temperatureTemp = temperatureRight * tempResolution;
               sensorSelect = 0;
               printFloat(temperatureTemp, Celcius, sensorSelect);                          //Print temperature in Celcius Format
               //printFloat(temperatureTemp, Fahrenheit, sensorSelect);                     //Print temperature in Fahrenheit Format
           }
           else{
               temperatureRight = (~(temperatureRight)) + 1;
               temperatureRight &=(0B0000111111111111);
               temperatureTemp = temperatureRight * tempResolution;
               temperatureTemp *= (-1);
               sensorSelect = 0;
               printFloat(temperatureTemp, Celcius, sensorSelect);                          //Print temperature in Celcius Format
               //printFloat(temperatureTemp, Fahrenheit, sensorSelect);                     //Print temperature in Fahrenheit Format
           }

           // ***********************Print Left temperature register values for testing feedback
           I2CTHReceive(TH_L_SLAVE_ADDR, PR_TEMP_REG, ui32Data, sizeof(ui32Data));

           if(ui32Data[0] & 0x80){
               g_bTempLeftNegativeFlag = 1;       //negative temperature
           }
           else{
               g_bTempLeftNegativeFlag = 0;       //positive temperature
           }

           temperatureLeft = (int16_t)((ui32Data[0] << 4) | (ui32Data[1] >> 4));

           if(!g_bTempLeftNegativeFlag){
               temperatureTemp = temperatureLeft * tempResolution;
               sensorSelect = 1;
               printFloat(temperatureTemp, Celcius, sensorSelect);                          //Print temperature in Celcius Format
               //printFloat(temperatureTemp, Fahrenheit, sensorSelect);                     //Print temperature in Fahrenheit Format

           }
           else{
               temperatureLeft = (~(temperatureLeft)) + 1;
               temperatureLeft &=(0B0000111111111111);
               temperatureTemp = temperatureLeft * tempResolution;
               temperatureTemp *= (-1);
               sensorSelect = 1;
               printFloat(temperatureTemp, Celcius, sensorSelect);                          //Print temperature in Celcius Format
               //printFloat(temperatureTemp, Fahrenheit, sensorSelect);                     //Print temperature in Fahrenheit Format
           }

           UARTprintf("\r\n---------------------------------\r\n");


        }
    }



}



