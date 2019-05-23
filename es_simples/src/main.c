/****************************************/
/*Frequenc�metro v1.0*/
//Gabriel Mendes
//Geovana Scaramella 
//S12
/****************************************/
#include "device_driver.h"

//#define PLL_FREQ_24   

#ifndef PLL_FREQ_24
  #define PLL_FREQ_120                       
#endif

#ifdef PLL_FREQ_24
  #define PLL_FREQ      24000000  //Hz
#endif
#ifdef PLL_FREQ_120
  #define PLL_FREQ      120000000//Hz
#endif


#define N_DADOS 5

#define JANELA_ESCALA_KILOHERTZ 1       //Em milissegundos
#define JANELA_ESCALA_HERTZ 1000        //Em milissegundos


uint16_t janela = JANELA_ESCALA_HERTZ;
uint32_t contador_janela_hertz = 0;
bool leitura_realizada = false;
uint32_t dados_lidos = 0;
float dadosFrequencia[N_DADOS+1];

//protótipos
void            PortJIntHandler(void);  //callback do botão
void            SysTick_Handler(void);  //callback do systick
void            setJanela(uint16_t janela_ms);

void            printFrequencia(float frequencia);

void main(void)
{
 
     clockInit(PLL_FREQ);
     
     uartInit();

     GPIOInit();

     timerInit();

  //Variáveis locais do main
  float frequencia = 0.0;
  float frequenciaMedia = 0.0, frequenciaMediana = 0.0, frequenciaModa = 0.0, desvioPadrao = 0.0;

  //Systick Configuration
  //SysTickDisable();
 // setJanela(janela);
  SysTickPeriodSet((PLL_FREQ/1000));//1ms
  SysTickEnable();
  SysTickIntEnable();


  //Interrupções
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
         uint32_t contagem = TimerValueGet(TIMER5_BASE, TIMER_A);//tirar
          
          dadosFrequencia[dados_lidos++] = (1000.0 * (float)TimerValueGet(TIMER5_BASE, TIMER_A)/(float)janela);
         // dadosFrequencia[dados_lidos++]=(float)TimerValueGet(TIMER5_BASE, TIMER_A);
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
     //frequencia = frequenciaModa;
     frequencia =  dadosFrequencia[1]/2;
   // uint64_t teste = frequencia*1000;
    
    // uart_puts("Frequencia: ");
    printFrequencia(frequencia);
   // uart_puts("\n");
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
  uint64_t f_aux = 0;//formatar string 
  char aux_s[32];
  
  if(janela == JANELA_ESCALA_HERTZ)
  {
    f_aux = ((uint64_t)(frequencia*1000)%1000);//todo
    sprintf(aux_s,"frequencia: %d.%d[Hz]",(int)frequencia,f_aux);
  }
  else //kHz
  {
    f_aux = ((uint64_t)(frequencia*1000000)%1000);//todo
    sprintf(aux_s,"%d.%d[kHz]",(int)(frequencia/1000),f_aux);
  }
  uart_puts(aux_s);
  
}

//Callback interrupção do botão
void PortJIntHandler(void)
{    
  SysTickDisable();
  if(janela == JANELA_ESCALA_HERTZ)
  {
    janela = JANELA_ESCALA_KILOHERTZ;
  }
  else
  {
    contador_janela_hertz = 0;
    janela = JANELA_ESCALA_HERTZ;
  }
  dados_lidos = 0;                      //resetar contagem para estatística
  TimerDisable(TIMER5_BASE, TIMER_A);
  TimerValueSet(TIMER5_BASE,TIMER_A,0);
  TimerEnable(TIMER5_BASE, TIMER_A);
  SysTickEnable();
}

void SysTick_Handler(void){
  if(janela == JANELA_ESCALA_KILOHERTZ)
  {
    TimerDisable(TIMER5_BASE, TIMER_A);
    SysTickDisable();
    leitura_realizada = true;
  }
  else
  {
    if(contador_janela_hertz>=JANELA_ESCALA_HERTZ)
    {
      TimerDisable(TIMER5_BASE, TIMER_A);
      SysTickDisable();
      contador_janela_hertz = 0;
      leitura_realizada = true;  
    }
    else
      contador_janela_hertz++;
      
  }
    
} // SysTick_Handler

//void setJanela(uint16_t janela_ms)
//{
//    janela = janela_ms;
//     // 
//}