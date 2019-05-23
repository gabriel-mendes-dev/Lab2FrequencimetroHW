#ifndef _DEVICE_DRIVER_
#define _DEVICE_DRIVER_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "driverlib/sysctl.h" // driverlib

#include "driverlib/gpio.h"
#include "inc/tm4c1294ncpdt.h" // cmsis-core
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_timer.h"

#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "driverlib/timer.h"


#define uart_putch(ch) UARTCharPut(UART0_BASE, ch)
#define TimerValueSet(u1Base, ulTimer, ulValue)         HWREG((u1Base) + ((ulTimer)==TIMER_A ? TIMER_O_TAV : TIMER_O_TBV)) = (ulValue)


void clockInit(uint32_t PLL_FREQ);
void uartInit();
void GPIOInit();
void timerInit();
void uart_puts(char * s);

#endif