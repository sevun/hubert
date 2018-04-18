/******************************************************************
 * Hubert Data Logger
 * Timer Interrupt Hibernate Example
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
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/debug.h"
#include "driverlib/hibernate.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "utils/uartstdio.h"

#define UART_SPEED              115200
#define HIBERNATE_WAKE_DELAY    5

//*****************************************************************************
// Hibernate Interrupt Function
//*****************************************************************************
void HibernateHandler(void)
{
    g_bHibernateFlag = 1;
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
    if(ui32Status & HIBERNATE_INT_LOW_BAT)
    {
        UARTprintf("\r\nWake due to low battery.");
    }
    // Process the RTC match 0 interrupt.
    if(ui32Status & HIBERNATE_INT_PIN_WAKE)
    {
        UARTprintf("\r\nWake due to wake pin.");
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

    // Sets the pin associated with IND1 to be an output
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
    // Hibernate Setup
    //*****************************************************************************

    if( !HibernateIsActive() )
    {
        UARTprintf("\r\nTOP Hyberate is not active.  Setting to active ...");
    }

    // Perform normal power-on initialization
    SysCtlPeripheralEnable(SYSCTL_PERIPH_HIBERNATE);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_HIBERNATE)) {}

    HibernateEnableExpClk(SysCtlClockGet());
    HibernateGPIORetentionEnable();
    SysCtlDelay(SysCtlClockGet()/3/50); // Delay 2 ms
    HibernateClockConfig(HIBERNATE_OSC_HIGHDRIVE);
    HibernateRTCTrimSet (0x7FFF);   // This line is necessary due to bug in Hibernate
    HibernateRTCEnable();
    HibernateWakeSet(HIBERNATE_WAKE_RTC | HIBERNATE_WAKE_LOW_BAT | HIBERNATE_WAKE_PIN );

    IntEnable(INT_HIBERNATE_TM4C123);
    HibernateIntEnable(HIBERNATE_INT_RTC_MATCH_0 | HIBERNATE_INT_LOW_BAT | HIBERNATE_INT_PIN_WAKE);
    HibernateIntClear(HIBERNATE_INT_RTC_MATCH_0 | HIBERNATE_INT_LOW_BAT | HIBERNATE_INT_PIN_WAKE);
    HibernateIntRegister(HibernateHandler);

    //*****************************************************************************
    // Main Code
    //*****************************************************************************

    UARTprintf("\r\n%d seconds",HibernateRTCGet());

    // Writes HIGH to pins
    GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, GPIO_PIN_4);  // IND1 LED On
    SysCtlDelay(SysCtlClockGet()/3/10);                     // Delay 0.1 second
    GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, 0);           // IND2 LED Off

    HibernateRTCMatchSet(0,HibernateRTCGet()+HIBERNATE_WAKE_DELAY);
    HibernateRequest();

    while(1) {}
}
