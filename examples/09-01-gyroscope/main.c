/******************************************************************
 * Hubert Data Logger
 * Gyroscope Example
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
#include "gyro/fxas21002c.h"

// Define UART speed in kbs
#define UART_SPEED                  115200

// Define FXAS21002C I2C address, determined by PCB layout with pins SA0=0
#define GYRO_SLAVE_ADDR       0x20

//*****************************************************************************
// I2C Functions
//*****************************************************************************

// see fxas21002c.c for gyroscope functions

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

int main(void)
{
    // Set microcontroller to use the 16 MHz external crystal
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    //*****************************************************************************
    // Pins Setup
    //*****************************************************************************

    // Enable the Port C peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);

    // Sets the pin associated with IND1 output
    GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_4);

    // Sets the pins associated with UART0
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Configure the GPIO Pin Mux for PB2 for I2C0SCL
    GPIOPinConfigure(GPIO_PB2_I2C0SCL);
    GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);

    // Configure the GPIO Pin Mux for PB3 for I2C0SDA
    GPIOPinConfigure(GPIO_PB3_I2C0SDA);
    GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);

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

    // Set the clock speed for the I2C0 bus
    I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), false);

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
    UARTprintf("Hubert is stirring");

    uint8_t ui32Data[1];

    // Get WHO_AM_I register, return should be 0xC7
    I2CGyroReceive(GYRO_SLAVE_ADDR, GYRO_WHO_AM_I, ui32Data, sizeof(ui32Data));
    if ( 0xD7 == ui32Data[0] )
    {
        UARTprintf("\r\n... FXAS21002C is alive!!!");
    }
    else
    {
        UARTprintf("\r\n... FXAS21002C is NOT alive.");
    }

    // ***********************Print register values for testing feedback
    I2CGyroReceive(GYRO_SLAVE_ADDR, GYRO_CTRL_REG1, ui32Data, sizeof(ui32Data));
    UARTprintf("\r\n0x%02X 0x%02x",GYRO_CTRL_REG1,ui32Data[0]);

    I2CGyroReceive(GYRO_SLAVE_ADDR, GYRO_CTRL_REG0, ui32Data, sizeof(ui32Data));
    UARTprintf("\r\n0x%02X 0x%02x",GYRO_CTRL_REG0,ui32Data[0]);
    // ***********************Print register values for testing feedback

    // Put the device into standby before changing register values
    GyroStandby(GYRO_SLAVE_ADDR);

    // Choose the range of the accelerometer (2000 dps, 1000 dps, 500 dps, 250 dps)
    GyroRange(GYRO_SLAVE_ADDR, GFSR_250);

    // Optionally, put the device into ready mode (lower power)
    GyroReady(GYRO_SLAVE_ADDR);

    // Activate the data device
    GyroActive(GYRO_SLAVE_ADDR);

    // ***********************Print register values for testing feedback
    I2CGyroReceive(GYRO_SLAVE_ADDR, GYRO_CTRL_REG1, ui32Data, sizeof(ui32Data));
    UARTprintf("\r\n0x%02X 0x%02x",GYRO_CTRL_REG1,ui32Data[0]);

    I2CGyroReceive(GYRO_SLAVE_ADDR, GYRO_CTRL_REG0, ui32Data, sizeof(ui32Data));
    UARTprintf("\r\n0x%02X 0x%02x",GYRO_CTRL_REG0,ui32Data[0]);
    // ***********************Print register values for testing feedback

    while(1)
    {
        // Timer 0
        if ( 1 == g_bTimer0Flag )
        {
            g_bTimer0Flag = 0;  // Clear the flag for Timer 0 interrupt

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
        }
    }
}
