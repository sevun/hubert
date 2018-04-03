/******************************************************************
 * Hubert Data Logger
 * Blink Hibenerate Repeat Example
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
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/debug.h"
#include "driverlib/hibernate.h"
#include "driverlib/gpio.h"
#include "utils/uartstdio.h"

#define UART_SPEED              115200
#define HIBERNATE_WAKE_DELAY    5

//uint32_t ui32Status;
//uint32_t pui32NVData[64];

int main(void)
{
    // Set microcontroller to use the 16 MHz external crystal
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    //*****************************************************************************
    // GPIO Setup
    //*****************************************************************************

    // Enable the Port C peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    // Sets the pin associated with IND1 to be an output
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2);

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

    // Perform normal power-on initialization
    SysCtlPeripheralEnable(SYSCTL_PERIPH_HIBERNATE);
//    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_HIBERNATE)) {}
    HibernateEnableExpClk(SysCtlClockGet());
    HibernateGPIORetentionEnable();

//    SysCtlDelay(SysCtlClockGet()/3);                        // Delay 1 second

//    HibernateClockConfig( HIBERNATE_OSC_HIGHDRIVE );
    HibernateRTCEnable();

    HibernateRTCSet(0);

    HibernateRTCMatchSet(0,HibernateRTCGet()+HIBERNATE_WAKE_DELAY);

//    ui32Status = HibernateIntStatus(0);
//    HibernateIntClear(ui32Status);

//    HibernateDataSet(pui32NVData, 16);

    HibernateWakeSet(HIBERNATE_WAKE_PIN | HIBERNATE_WAKE_RTC );

//    HibernateCounterMode(HIBERNATE_COUNTER_RTC);

    //*****************************************************************************
    // Main Code
    //*****************************************************************************

    UARTprintf("x");

    // Writes HIGH to pins
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);  // IND1 LED On
    SysCtlDelay(SysCtlClockGet()/3/10);                     // Delay 0.1 second
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);           // IND2 LED Off
    SysCtlDelay(SysCtlClockGet()/3);                        // Delay 1 second

    HibernateRequest();

    while(1)  // Repeats this section over and over
    {
    }
}
