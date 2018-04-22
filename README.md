
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
4. **Timers**
	1. **One Timer** - A repeating timer which blinks a light on IND1
	2. **Two Timer** - A repeating timer which blinks a light on IND1 slow and IND2 fast
5. **UART**
	1. **UART Echo with Polling** - Reads character using polling on UART0 Rx and echoes it to UART0 Tx
	2. **UART Echo with Interrupts** - Reads character using interrupt on UART0 Rx and echoes it to UART0 Tx
6. **User Interface** - Uses a timer to update the display and UART receive to change screens
7. **Hibernate**
	1. **Blink, Hibernate, Repeat** - Use the hibernate module to sleep between blinking IND1
	2. **Timer Interrupt Hibernate** - Wake from hibernate on timer schedule
	3. **SW1 Hibernate** - Hibernate when SW1 is pressed for and wake after a set period of time
	4. **Auto Hibernate with Wake** - Automatically goes into timed interrupt hibernate when powered, then Wake button wakes, and SW1 button puts back in hibernate mode
0. **SD Card** - TBD
0. **I2C**
	1. **Accelerometer and Magnetometer** - TBD
	2. **Gyroscope** - TBD
	3. **Temperature** - TBD
	4. **Humidity** - TBD