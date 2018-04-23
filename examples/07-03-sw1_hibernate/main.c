/******************************************************************
 * Hubert Data Logger
 * SW1 Hibernate Example
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
#include "utils/ustdlib.h"
#include "utils/uartstdio.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/hibernate.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"

#define UART_SPEED              115200
#define HIBERNATE_WAKE_DELAY    5

//*****************************************************************************
// Hibernate Interrupt
//*****************************************************************************

void HibernateHandler(void)
{
    uint32_t ui32Status;

    // Get the interrupt status and clear any pending interrupts.
    ui32Status = HibernateIntStatus(true);
    HibernateIntClear(ui32Status);

    // Process the RTC match 0 interrupt.
    if(ui32Status & HIBERNATE_INT_RTC_MATCH_0)
    {
        UARTprintf("\r\nWake due to RTC Match 0.");
    }

    // Process the RTC match 0 interrupt.
    if(ui32Status & HIBERNATE_INT_PIN_WAKE)
    {
        UARTprintf("\r\nWake due to wake pin.");
    }
}

//*****************************************************************************
// Timers Interrupt
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

    // Enable the peripherals
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    // Sets the pin associated with SW1 to be an input
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);

    // Sets the pin associated with IND1 and IND2 outputs
    GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_4);
    GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_5);

    // Sets the pins associated with UART0
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

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
    // Hibernate Setup
    //*****************************************************************************

    if( !HibernateIsActive() )
    {
        UARTprintf("\r\nHibernate is not active.  Trying to set active ...");

        // Perform normal power-on initialization
        SysCtlPeripheralEnable(SYSCTL_PERIPH_HIBERNATE);
        while(!SysCtlPeripheralReady(SYSCTL_PERIPH_HIBERNATE)) {}

        HibernateEnableExpClk(SysCtlClockGet());
        HibernateGPIORetentionEnable();
        SysCtlDelay(SysCtlClockGet()/3/50); // Delay 2 ms
        HibernateClockConfig(HIBERNATE_OSC_HIGHDRIVE);
        HibernateRTCTrimSet (0x7FFF);   // This line is necessary due to bug in Hibernate
        HibernateRTCEnable();
        HibernateWakeSet(HIBERNATE_WAKE_RTC | HIBERNATE_WAKE_PIN );

        HibernateIntEnable(HIBERNATE_INT_RTC_MATCH_0 | HIBERNATE_INT_PIN_WAKE);
        HibernateIntClear(HIBERNATE_INT_RTC_MATCH_0 | HIBERNATE_INT_PIN_WAKE);
        HibernateIntRegister(HibernateHandler);
    }

    //*****************************************************************************
    // Configure Interrupts
    //*****************************************************************************

    IntEnable(INT_HIBERNATE_TM4C123);

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

        if ( GPIO_PIN_4 != GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4) )
        {
            GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, GPIO_PIN_5);  // IND2 LED On
            SysCtlDelay(SysCtlClockGet()/3/10);                     // Delay 0.1 second
            GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, 0);           // IND2 LED Off
            SysCtlDelay(SysCtlClockGet()/3/10);                     // Delay 0.1 second
            GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, GPIO_PIN_5);  // IND2 LED On
            SysCtlDelay(SysCtlClockGet()/3/10);                     // Delay 0.1 second
            GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, 0);           // IND2 LED Off

            uint32_t ui32RTCTime;
            ui32RTCTime = HibernateRTCGet();

            SysCtlDelay(SysCtlClockGet()/3/50); // Delay 2 ms
            UARTprintf("\r\n%d seconds",ui32RTCTime);
            SysCtlDelay(SysCtlClockGet()/3/50); // Delay 2 ms

            HibernateRTCMatchSet(0,ui32RTCTime+HIBERNATE_WAKE_DELAY);
            HibernateRequest();
        }
    }
}
