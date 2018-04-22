/******************************************************************
 * Hubert Data Logger
 * Accelerometer and Magnetometer Example
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

#define UART_SPEED                  115200

// FXOS8700CQ I2C address, determined by PCB layout with pins SA0=1, SA1=0
#define FXOS8700CQ_SLAVE_ADDR       0x1D

// FXOS8700CQ internal register addresses
#define FXOS8700CQ_STATUS           0x00
#define FXOS8700CQ_WHOAMI           0x0D
#define FXOS8700CQ_XYZ_DATA_CFG     0x0E
#define FXOS8700CQ_CTRL_REG1        0x2A
#define FXOS8700CQ_M_CTRL_REG1      0x5B
#define FXOS8700CQ_M_CTRL_REG2      0x5C
#define FXOS8700CQ_WHOAMI_VAL       0xC7

uint64_t g_ui64Heartbeat;

//*****************************************************************************
// Timers Interrupt Function
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
//! Indicates whether or not the I2C bus has timed out.
//!
//! \param ui32Base is the base address of the I2C module.
//!
//! This function returns an indication of whether or not the I2C bus has time
//!  out.  The I2C Master Timeout Value must be set.
//!
//! \return Returns \b true if the I2C bus has timed out; otherwise, returns
//! \b false.
//*****************************************************************************
bool I2CMasterTimeout(uint32_t ui32Base)
{
    // Return the bus timeout status
    if(HWREG(ui32Base + I2C_O_MCS) & I2C_MCS_CLKTO)
    {
        return(true);
    }
    else
    {
       return(false);
    }
}

int main(void)
{
    // Set microcontroller to use the 16 MHz external crystal
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    //*****************************************************************************
    // Pins Setup
    //*****************************************************************************

    // Enable the Port C peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);

    // Sets the pin associated with iND1 and IND2 to be output
    GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_4);

    //*****************************************************************************
    // UART Setup
    //*****************************************************************************

    // Enable the Port A and UART0 peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // Sets the pins associated with UART0
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Initialize the UART0 using uartstdio
    UARTStdioConfig(0, UART_SPEED, SysCtlClockGet());

    //*****************************************************************************
    // I2C Setup
    //*****************************************************************************

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    // Configure the GPIO Pin Mux for PB2 for I2C0SCL
    GPIOPinConfigure(GPIO_PB2_I2C0SCL);
    GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);

    // Configure the GPIO Pin Mux for PB3 for I2C0SDA
    GPIOPinConfigure(GPIO_PB3_I2C0SDA);
    GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);

    // Set the clock speed for the I2C0 bus
    I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), false);
    I2CMasterTimeoutSet(I2C0_BASE,  0xFF);

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
    // Configure Interrupts
    //*****************************************************************************

    // Enable processor interrupts.
    IntMasterEnable();

    //*****************************************************************************
    // Main Code
    //*****************************************************************************

    while(1)
    {
        // Timer 0
        if ( 1 == g_bTimer0Flag )
        {
            g_bTimer0Flag = 0;  // Clear the flag for Timer 0 interrupt

            g_ui64Heartbeat++;

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

            //specify that we are writing (a register address) to the
            //slave device
            I2CMasterSlaveAddrSet(I2C0_BASE, FXOS8700CQ_SLAVE_ADDR, false);

            //specify register to be read
            I2CMasterDataPut(I2C0_BASE, FXOS8700CQ_WHOAMI);
            I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);
            while(I2CMasterBusy(I2C0_BASE));

            //specify that we are going to read from slave device
            I2CMasterSlaveAddrSet(I2C0_BASE, FXOS8700CQ_SLAVE_ADDR, true);
            I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);
            while(I2CMasterBusy(I2C0_BASE));

            //return data pulled from the specified register
            uint8_t ui8Data =  I2CMasterDataGet(I2C0_BASE);

            // Clear and reset home screen
            UARTprintf("\033[2J\033[;H");
            UARTprintf("It's alive!!!");

            UARTprintf("\r\n0x%08X",(uint32_t) g_ui64Heartbeat);
            UARTprintf("\r\n0x%02X",ui8Data);
        }
    }
}
