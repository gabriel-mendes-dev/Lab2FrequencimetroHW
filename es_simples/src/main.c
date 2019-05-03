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

#include "driverlib/sysctl.h" // driverlib
#include "driverlib/gpio.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"

#include "macro_loop.h"
#include "processamento_dados.h"

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
#define SCALE_CHANGE_REG                HWREG(SCALE_CHANGE_BUTTON_PORT + (0x00000000+(SCALE_CHANGE_BUTTON_PIN << 2)))

//uart
#define uart_putch(ch) UARTCharPut(UART0_BASE, ch)

#define N_DADOS 50

bool escala = 0;

//prot�tipos
void            uart_puts(char * s);
void            PortJIntHandler(void);
uint32_t        getBordas();
void            printFrequencia(float frequencia);

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
  float dados[N_DADOS];
  float frequenciaMedia = 0.0, frequenciaMediana = 0.0, frequenciaModa = 0.0, desvioPadrao = 0.0;
  uint8_t *vetor_estados = (uint8_t *) malloc(N*sizeof(uint8_t));//Alocação dinâmica utilizada somente 
                                                                 //com o intuito de alocar na heap

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
    
  GPIOPinTypeGPIOInput(WAVE_INPUT_PORT, WAVE_INPUT_PIN); // Entrada da onda como entrada
  GPIOPadConfigSet(WAVE_INPUT_PORT, WAVE_INPUT_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);//weak pull-up
  
  //GPIO bot�o
  SysCtlPeripheralEnable(SCALE_CHANGE_PERIPHERAL); // Habilita GPIO J (push-button SW1 = PJ0, push-button SW2 = PJ1)
  while(!SysCtlPeripheralReady(SCALE_CHANGE_PERIPHERAL)); // Aguarda final da habilita��o
    
  GPIOPinTypeGPIOInput(SCALE_CHANGE_BUTTON_PORT, SCALE_CHANGE_BUTTON_PIN); // push-button SW1
  GPIOPadConfigSet(SCALE_CHANGE_BUTTON_PORT, SCALE_CHANGE_BUTTON_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
  GPIOIntTypeSet(SCALE_CHANGE_BUTTON_PORT, SCALE_CHANGE_BUTTON_PIN, GPIO_LOW_LEVEL); // push-button SW1
  GPIOIntEnable(SCALE_CHANGE_BUTTON_PORT, SCALE_CHANGE_INT_PIN);
 
  //Interrup��es
  IntEnable(INT_GPIOJ);//habilita interrup��o do portJ - bot�o de escala
  IntMasterEnable();//habilita todas as interrup��es
    
  if(vetor_estados == NULL)
  {
      char aux_s[32];
      sprintf(aux_s,"Erro malloc\n");
      uart_puts(aux_s);
      while(1);      
  }
  
  while(1)
  {            
    if(escala == 0)    
    {
      for(uint16_t i = 0; i < N_DADOS; i++)
      {
        n_loop(vetor_estados,WAVE_INPUT_END);     
        
        //C�lculo da frequ�ncia     
        dados[i] = (float)((float)getBordas(vetor_estados))*PLL_FREQ/(float)(5 * N);
      }
    }
    else
    {
      for(uint16_t i = 0; i < N_DADOS; i++)
      {
        asm("push {r1,r2,r4}");
        asm("mov r1, %[var]\n" ::[var] "r" (WAVE_INPUT_END)); 
        asm("mov r2, %[var]\n" ::[var] "r" (vetor_estados-1)); 
      
        //loop dura 2 segundos para PLL = 24Mhz
        //(2 segundos/16000 amostras) * 24MHz = 3000 ciclos
        for(uint32_t i = 0; i < N; i++)//usa R6 ~5ciclos
        {
          asm("LDR.W R4, [R1]"); //2 ciclos
          asm("STRB R4, [R2,#1]!");//3 ciclos
          SysCtlDelay(996);//usa R0 ~ (3000 - 10)/3
        }
        
        asm("pop {r1,r2,r4}");
        
        //C�lculo da frequ�ncia 
        dados[i] = (float)(getBordas(vetor_estados)/2.0)*PLL_FREQ/(float)(3000 * N);
      }
    }

    frequenciaMedia = Media(dados, N_DADOS);
    frequenciaMediana = Mediana(dados, N_DADOS);
    frequenciaModa = Moda(dados, N_DADOS);
    desvioPadrao = DesvioPadrao(dados, N_DADOS); 
    frequencia = frequenciaMedia;
    
    printFrequencia(frequencia);
 
  } // while
} // main

uint32_t getBordas(uint8_t *vetor_estados)
{
      uint32_t contagem_bordas = 0;
      uint8_t  estado_anterior = *(vetor_estados);
      for(uint32_t i = 0; i < N; i++)
      {
        if(estado_anterior != *(vetor_estados+i))//andar 1 byte
        {
          contagem_bordas++;        
          estado_anterior = *(vetor_estados+i);
        }
      }
      
      return contagem_bordas;
}

void printFrequencia(float frequencia)
{
  uint16_t f_aux = 0;//formatar string 
  char aux_s[32];
  
  f_aux = ((uint16_t)(frequencia*1000)%1000);//todo
  sprintf(aux_s,"frequencia: %d.%d",(int)frequencia,f_aux);
  uart_puts(aux_s);
  
}

void uart_puts(char * s) {
    while (*s != 0) {
        uart_putch(*s);
        s++;
    }
}

//Callbeack interrup??o do bot?o
void PortJIntHandler(void)
{
  escala = !escala;
}