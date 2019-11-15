/******************************************************************
 * Hubert Data Logger
 * SysTick Setup Example
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
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "pinout.h"

uint32_t ui32Time;

//*****************************************************************************
// Systick Interrupt
//*****************************************************************************
void SysTickHandler(void)
{
    SysTickIntDisable();
    ui32Time++;
    SysTickIntEnable();
}

//*****************************************************************************
// Main Code
//*****************************************************************************

int main(void)
{
    // Set microcontroller to use the 16 MHz external crystal
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    // Initialize and setup the ports
    PinoutSet();

    //*****************************************************************************
    // Systick Setup
    //*****************************************************************************

    // Configure the systick timer for a 100 Hz interrupt
    SysTickPeriodSet(SysCtlClockGet()/100);
    SysTickIntRegister(SysTickHandler);
    SysTickEnable();
    SysTickIntEnable();

    //*****************************************************************************
    // Configure Interrupts
    //*****************************************************************************

    IntMasterEnable();

    //loop forever
    while(1)
    {
    }
}
