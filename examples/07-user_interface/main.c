/******************************************************************
 * Hubert Data Logger
 * User Interface Example
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
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"

#define UART_SPEED 115200

//*****************************************************************************
// Timers Interrupt Function
//*****************************************************************************
volatile bool g_bTimer0Flag = 0;        // Timer 0 occurred flag
volatile bool g_bTimer1Flag = 0;        // Timer 1 occurred flag

// The interrupt handler for the first timer interrupt. 1 Hz
void Timer0IntHandler(void)
{
    // Clear the timer interrupt.
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    g_bTimer0Flag = 1;      // Set the flag for Timer 0 interrupt
}

// The interrupt handler for the second timer interrupt. 10 Hz
void Timer1IntHandler(void)
{
    // Clear the timer interrupt.
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    g_bTimer1Flag = 1;      // Set flag to indicate Timer 1 interrupt
}

//*****************************************************************************
// UART Interrupt Functions
//*****************************************************************************

volatile bool g_bKeyboardInputFlag;

// The interrupt handler for UART0
void UART0IntHandler(void)
{
    uint32_t ui32Status;

    ui32Status = UARTIntStatus(UART0_BASE, true); //get interrupt status

    UARTIntClear(UART0_BASE, ui32Status); //clear the asserted interrupts

    g_bKeyboardInputFlag = 1;
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
    GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_5);

    // Enable the Port A and UART0 peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // Sets the pins associated with UART0
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //*****************************************************************************
    // Timer Setup
    //*****************************************************************************

    // Enable the peripherals used by this example.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);

    // Configure the two 32-bit periodic timers.
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet() * 5);      // 5 s
    TimerLoadSet(TIMER1_BASE, TIMER_A, SysCtlClockGet() / 10);     // 10 Hz

    // Setup the interrupts for the timer timeouts.
    IntEnable(INT_TIMER0A);
    IntEnable(INT_TIMER1A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    // Enable the timers.
    TimerEnable(TIMER0_BASE, TIMER_A);
    TimerEnable(TIMER1_BASE, TIMER_A);

    //*****************************************************************************
    // UART Setup
    //*****************************************************************************
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), UART_SPEED,
        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

    //*****************************************************************************
    // Configure Interrupts
    //*****************************************************************************

    // Enable processor interrupts.
    IntMasterEnable();

    IntEnable(INT_UART0); //enable the UART interrupt
    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT); //only enable RX and TX interrupts

    //*****************************************************************************
    // Main Code
    //*****************************************************************************

    while(1)
    {
        // UART 0
        if ( 1 == g_bKeyboardInputFlag )
        {
            g_bKeyboardInputFlag = 0;  // Clear the flag for keyboard input interrupt

            while( UARTCharsAvail(UART0_BASE) ) //loop while there are chars
            {
                UARTCharPutNonBlocking(UART0_BASE, UARTCharGetNonBlocking(UART0_BASE)); //echo character
            }
        }

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

        // Timer 0
        if ( 1 == g_bTimer1Flag )
        {
            g_bTimer1Flag = 0;  // Clear the flag for Timer 1 interrupt

            // Check the current value of the pin, then flip it
            if ( GPIO_PIN_5 == GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_5) )
            {
                // Writes HIGH to pins
                GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, 0);           // IND2 LED Off
            }
            else
            {
                // Writes HIGH to pins
                GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, GPIO_PIN_5);  // IND2 LED On
            }
        }
    }
}
