/*
 * *****************************************************************
 * Hubert - A data aquisition platform
 * Developed by Counterwound Labs, Inc.
 * http://counterwound.com
 * *****************************************************************
 */

#include <stdint.h>
#include <stdbool.h>
#include "driverlib/sysctl.h"
#include "pinmux.h"

int main(void)
{
    // Set the clocking to run directly from the crystal.
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    // Initialize and setup the ports
    PortFunctionInit();

	return 0;
}
