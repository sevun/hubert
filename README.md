# Hubert
Data acquisition platform for remote data collection

## Environment
A video on how to setup the environment.

[![Link](https://img.youtube.com/vi/Lsf7iXAZT8M/1.jpg)](https://www.youtube.com/watch?v=Lsf7iXAZT8M)

## Examples
The examples generally follow the tutorial provided by Texas Instruments called
'[Getting Started with the TIVA™ C Series TM4C123G LaunchPad](http://processors.wiki.ti.com/index.php/Getting_Started_with_the_TIVA™_C_Series_TM4C123G_LaunchPad)'.
They are tailored to match the Hubert hardware.
1. **GPIO Write** - This just turns on the IND1 and IND2 lights.  These LEDs are there for general use.
2. **Blinky** - Sets the IND1 and IND2 lights to blink in an alternating pattern using a delay.
3. **GPIO Read** - Uses the SW1 to turn on the IND2.
4. **UART Echo** - Reads character on UART0 Rx and echoes it to UART0 Tx
5. **Timers**
    5.1 **One Timer** - A repeating timer which blinks a light on IND1
    5.2 **Two Timer** - A repeating timer which blinks a light on IND1 slow and IND2 fast
6. **UART User Interface** - Setup a display to see data on the UART and a menu system.
7. **Hibernate** - Use the hibernate module to sleep between blinking IND1.