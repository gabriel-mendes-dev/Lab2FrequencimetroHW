/****************************************/
/*Frequenc�metro v1.0*/
//Gabriel Mendes
//Geovana Scaramella 
//S12
/****************************************/
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "inc/tm4c1294ncpdt.h" // CMSIS-Core
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_timer.h"

#include "driverlib/sysctl.h" // driverlib
#include "driverlib/gpio.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "driverlib/timer.h"

//constantes de escala
#define PLL_FREQ_24   

#ifndef PLL_FREQ_24
  #define PLL_FREQ_120                       
#endif

#ifdef PLL_FREQ_24
  #define PLL_FREQ      24000000  //Hz
#endif
#ifdef PLL_FREQ_120
  #define PLL_FREQ      120000000//Hz
#endif

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

//uart
#define uart_putch(ch) UARTCharPut(UART0_BASE, ch)

#define N_DADOS 5

#define JANELA_ESCALA_KILOHERTZ 1       //Em milissegundos
#define JANELA_ESCALA_HERTZ 2000        //Em milissegundos

#define TimerValueSet(u1Base, ulTimer, ulValue)         HWREG((u1Base) + ((ulTimer)==TIMER_A ? TIMER_O_TAV : TIMER_O_TBV)) = (ulValue)

uint16_t janela = JANELA_ESCALA_KILOHERTZ;
bool leitura_realizada = false;
uint32_t dados_lidos = 0;
float dadosFrequencia[N_DADOS+1];

//prot�tipos
void            uart_puts(char * s);
void            PortJIntHandler(void);
void            printFrequencia(float frequencia);
void            SysTick_Handler(void);
void            setJanela(uint16_t janela_ms);


void main(void)
{
 
  //inicializa clock
  uint32_t ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                              SYSCTL_OSC_MAIN |
                                              SYSCTL_USE_PLL |
                                              SYSCTL_CFG_VCO_480),
                                              PLL_FREQ); // PLL em 24MHz

  //Variáveis locais do main
  float frequencia = 0.0;
  float frequenciaMedia = 0.0, frequenciaMediana = 0.0, frequenciaModa = 0.0, desvioPadrao = 0.0;
  

  //Inicializa Uart
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0)); // Aguarda final da habilita��o
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)); // Aguarda final da habilita��o
  
  //GPIO da Uart
  GPIOPinConfigure(GPIO_PA0_U0RX);
  GPIOPinConfigure(GPIO_PA1_U0TX);
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
  UARTConfigSetExpClk(UART0_BASE, ui32SysClock, 115200,(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));           
  
  //GPIO de entrada da onda
  SysCtlPeripheralEnable(WAVE_INPUT_PERIPHERAL); // Habilita GPIO B (Entrada da onda = PB2)
  while(!SysCtlPeripheralReady(WAVE_INPUT_PERIPHERAL)); // Aguarda final da habilita��o
    
  //GPIOPinTypeGPIOInput(WAVE_INPUT_PORT, WAVE_INPUT_PIN); // Entrada da onda como entrada
  GPIOPinConfigure(GPIO_PB2_T5CCP0);
  GPIOPinTypeTimer(WAVE_INPUT_PORT, WAVE_INPUT_PIN);
  GPIOPadConfigSet(WAVE_INPUT_PORT, WAVE_INPUT_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);//weak pull-up
  
  //GPIO bot�o
  SysCtlPeripheralEnable(SCALE_CHANGE_PERIPHERAL); // Habilita GPIO J (push-button SW1 = PJ0, push-button SW2 = PJ1)
  while(!SysCtlPeripheralReady(SCALE_CHANGE_PERIPHERAL)); // Aguarda final da habilita��o
    
  GPIOPinTypeGPIOInput(SCALE_CHANGE_BUTTON_PORT, SCALE_CHANGE_BUTTON_PIN); // push-button SW1
  GPIOPadConfigSet(SCALE_CHANGE_BUTTON_PORT, SCALE_CHANGE_BUTTON_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
  GPIOIntTypeSet(SCALE_CHANGE_BUTTON_PORT, SCALE_CHANGE_BUTTON_PIN, GPIO_LOW_LEVEL); // push-button SW1
  GPIOIntEnable(SCALE_CHANGE_BUTTON_PORT, SCALE_CHANGE_INT_PIN);
 
  //Configuração do Timer 5 - linkado com PB2
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER5);
  TimerConfigure(TIMER5_BASE, TIMER_CFG_SPLIT_PAIR|TIMER_CFG_A_CAP_COUNT_UP);
  TimerControlEvent(TIMER5_BASE, TIMER_A, TIMER_EVENT_POS_EDGE);
//  TimerMatchSet(TIMER5_BASE,TIMER_A,0xFFFF);
  TimerPrescaleSet(TIMER5_BASE,TIMER_A,1);
//  TimerPrescaleMatchSet(TIMER5_BASE,TIMER_A,0xFF);
    //TimerUpdateMode(TIMER5_BASE,TIMER_A,TIMER_UP_LOAD_IMMEDIATE);
  //TimerLoadSet(TIMER5_BASE, TIMER_A, 8);
 // TimerEnable(TIMER5_BASE, TIMER_A);
  uint32_t contagem = TimerValueGet(TIMER5_BASE, TIMER_A);
  //TimerMatchSet

  //Systick Configuration
  //SysTickDisable();
  setJanela(janela);
  SysTickEnable();
  SysTickIntEnable();


  //Interrup��es
  IntEnable(INT_GPIOJ);//habilita interrup��o do portJ - bot�o de escala
  IntMasterEnable();//habilita todas as interrup��es
  
  while(1)
  {     
    dados_lidos = 0;
    TimerValueSet(TIMER5_BASE,TIMER_A,0);
 //   SysTickDisable();
   // leitura_realizada = false;
  //  setJanela(janela);
    TimerEnable(TIMER5_BASE, TIMER_A);
   // SysTickEnable();
    while(dados_lidos < (N_DADOS+1))
    {
      if(leitura_realizada)
      {
          leitura_realizada = false;
          contagem = TimerValueGet(TIMER5_BASE, TIMER_A);//tirar
          
          dadosFrequencia[dados_lidos++] = (1000.0 * (float)TimerValueGet(TIMER5_BASE, TIMER_A)/(float)janela);
          
        //  TimerDisable(TIMER5_BASE, TIMER_A);
          TimerValueSet(TIMER5_BASE,TIMER_A,0);
          TimerEnable(TIMER5_BASE, TIMER_A);
          SysTickEnable();
      }
    }           
    
    
    frequenciaMedia = Media((dadosFrequencia+1), N_DADOS);
    frequenciaMediana = Mediana((dadosFrequencia+1), N_DADOS);
    frequenciaModa = Moda((dadosFrequencia+1), N_DADOS);
    //desvioPadrao = DesvioPadrao((dadosFrequencia+1), N_DADOS); 
   // frequencia = frequenciaMedia;
   // frequencia = frequenciaMediana;
     frequencia = frequenciaModa;
   // uint64_t teste = frequencia*1000;
    printFrequencia(frequencia);
   // printf("\n");
/*    
    printFrequencia(frequenciaMedia);
    printf("\n");
    printFrequencia(frequenciaMediana);
    printf("\n");
    printFrequencia(frequenciaModa);
    printf("\n");
    printFrequencia(desvioPadrao);
    printf("\n");
 */
  } // while
} // main

void printFrequencia(float frequencia)
{
  uint32_t f_aux = 0;//formatar string 
  char aux_s[32];
  
  f_aux = ((uint32_t)(frequencia*1000)%1000);//todo
  sprintf(aux_s,"frequencia: %d.%d",(int)frequencia,f_aux);
  uart_puts(aux_s);
  
}

void uart_puts(char * s) {
    while (*s != 0) {
        uart_putch(*s);
        s++;
    }
}

//Callback interrupção do botão
void PortJIntHandler(void)
{    
  SysTickDisable();
  if(janela == JANELA_ESCALA_HERTZ)
  {
    setJanela(JANELA_ESCALA_KILOHERTZ);
  }
  else
  {
    setJanela(JANELA_ESCALA_HERTZ);
  }
  dados_lidos = 0;                      //resetar contagem para estatística
  TimerDisable(TIMER5_BASE, TIMER_A);
  TimerValueSet(TIMER5_BASE,TIMER_A,0);
  TimerEnable(TIMER5_BASE, TIMER_A);
  SysTickEnable();
}

void SysTick_Handler(void){
    leitura_realizada = true;
    TimerDisable(TIMER5_BASE, TIMER_A);
    SysTickDisable();
} // SysTick_Handler

void setJanela(uint16_t janela_ms)
{
    janela = janela_ms;
    SysTickPeriodSet((PLL_FREQ/1000) * janela); // 
}