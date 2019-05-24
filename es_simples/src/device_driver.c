#include "device_driver.h"
//#include "driverlib/sysctl.h" // driverlib
//
//#include "inc/tm4c1294ncpdt.h" // CMSIS-Core
//#include "inc/hw_memmap.h"
//#include "inc/hw_types.h"
//#include "inc/hw_gpio.h"
//#include "inc/hw_timer.h"
//#include "driverlib/gpio.h"
//#include "driverlib/systick.h"
//#include "driverlib/interrupt.h"
//#include "driverlib/uart.h"
//#include "driverlib/pin_map.h"
//#include "driverlib/timer.h"
//#include "includes.h"


//GPIO entrada da onda
#define WAVE_INPUT_PERIPHERAL           SYSCTL_PERIPH_GPIOB
#define WAVE_INPUT_PORT                 GPIO_PORTB_BASE
#define WAVE_INPUT_PIN                  GPIO_PIN_2
//#define WAVE_INPUT_REG                  HWREG(WAVE_INPUT_PORT + (GPIO_O_DATA + (GPIO_PIN_2 << 2)))
#define WAVE_INPUT_END                  (volatile uint32_t *)(WAVE_INPUT_PORT + (GPIO_O_DATA + (GPIO_PIN_2 << 2)))

//GPIO bot?o de escala
#define SCALE_CHANGE_PERIPHERAL         SYSCTL_PERIPH_GPIOJ
#define SCALE_CHANGE_BUTTON_PORT        GPIO_PORTJ_BASE
#define SCALE_CHANGE_BUTTON_PIN         GPIO_PIN_0
#define SCALE_CHANGE_INT_PIN            GPIO_INT_PIN_0

uint32_t ui32SysClock = 0;
void clockInit(uint32_t PLL_FREQ)
{
    //inicializa clock
   ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                              SYSCTL_OSC_MAIN |
                                              SYSCTL_USE_PLL |
                                              SYSCTL_CFG_VCO_480),
                                              PLL_FREQ); // PLL 
}

void uartInit()
{
  //Inicializa Uart
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0)); // Aguarda final da habilitação
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)); // Aguarda final da habilitação
  
  //GPIO da Uart
  GPIOPinConfigure(GPIO_PA0_U0RX);
  GPIOPinConfigure(GPIO_PA1_U0TX);
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
  UARTConfigSetExpClk(UART0_BASE, ui32SysClock, 115200,(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));     
}

void GPIOInit()
{
   //GPIO de entrada da onda
  SysCtlPeripheralEnable(WAVE_INPUT_PERIPHERAL); // Habilita GPIO B (Entrada da onda = PB2)
  while(!SysCtlPeripheralReady(WAVE_INPUT_PERIPHERAL)); // Aguarda final da habilita??o
    
  GPIOPinConfigure(GPIO_PB2_T5CCP0); //GPIO como contador
  GPIOPinTypeTimer(WAVE_INPUT_PORT, WAVE_INPUT_PIN);
  GPIOPadConfigSet(WAVE_INPUT_PORT, WAVE_INPUT_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);//weak pull-up
  
  //GPIO botão
  SysCtlPeripheralEnable(SCALE_CHANGE_PERIPHERAL); // Habilita GPIO J (push-button SW1 = PJ0, push-button SW2 = PJ1)
  while(!SysCtlPeripheralReady(SCALE_CHANGE_PERIPHERAL)); // Aguarda final da habilita??o
    
  GPIOPinTypeGPIOInput(SCALE_CHANGE_BUTTON_PORT, SCALE_CHANGE_BUTTON_PIN); // push-button SW1
  GPIOPadConfigSet(SCALE_CHANGE_BUTTON_PORT, SCALE_CHANGE_BUTTON_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
  GPIOIntTypeSet(SCALE_CHANGE_BUTTON_PORT, SCALE_CHANGE_BUTTON_PIN, GPIO_LOW_LEVEL); // push-button SW1
  GPIOIntEnable(SCALE_CHANGE_BUTTON_PORT, SCALE_CHANGE_INT_PIN); 
}

void timerInit()
{
    //Configuração do Timer 5 - linkado com PB2
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER5);
  TimerConfigure(TIMER5_BASE, TIMER_CFG_SPLIT_PAIR|TIMER_CFG_A_CAP_COUNT_UP);
  TimerControlEvent(TIMER5_BASE, TIMER_A, TIMER_EVENT_BOTH_EDGES);//TIMER_EVENT_POS_EDGE);
//  TimerMatchSet(TIMER5_BASE,TIMER_A,0xFFFF);
 // TimerPrescaleSet(TIMER5_BASE,TIMER_A,0xFF);
 // TimerPrescaleMatchSet(TIMER5_BASE,TIMER_A,0xFF);
    //TimerUpdateMode(TIMER5_BASE,TIMER_A,TIMER_UP_LOAD_IMMEDIATE);
 // TimerLoadSet(TIMER5_BASE, TIMER_A, 0xFFFF);
 // TimerEnable(TIMER5_BASE, TIMER_A);
 // uint32_t contagem = TimerValueGet(TIMER5_BASE, TIMER_A);
  //TimerMatchSet
}


void systickInit(uint16_t ms)
{  
  //Systick Configuration
  SysTickPeriodSet((ui32SysClock/1000)*ms);//1ms
  SysTickEnable();
  SysTickIntEnable();
}

void interruptInit()
{
  IntEnable(INT_GPIOJ);//habilita interrup??o do portJ - bot?o de escala
  IntMasterEnable();//habilita todas as interrup??es
}

void uart_puts(char * s) 
{
    while (*s != 0) {
        uart_putch(*s);
        s++;
    }
}
