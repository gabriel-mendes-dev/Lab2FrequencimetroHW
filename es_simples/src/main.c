/****************************************/
/*Frequenc�metro v1.0*/
//Gabriel Mendes
//Geovana Scaramella 
//S12
/****************************************/
#include "device_driver.h"
#include "processamento_dados.h"

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


#define N_DADOS 10

#define JANELA_ESCALA_KILOHERTZ 1       //Em milissegundos
#define JANELA_ESCALA_HERTZ 1000        //Em milissegundos


uint16_t janela = JANELA_ESCALA_KILOHERTZ;
uint32_t contador_janela_hertz = 0;
bool leitura_realizada = false;
uint32_t dados_lidos = 0;
float dadosFrequencia[N_DADOS+1];
uint32_t contagem = 0;

//protótipos
void PortJIntHandler(void);  //callback do botão
void SysTick_Handler(void);  //callback do systick

void printFrequencia(float frequencia);
void printDesvio(float desvio);

void main(void)
{
   //Variáveis locais do main
  float frequencia = 0.0;
  float frequenciaMedia = 0.0, frequenciaMediana = 0.0, frequenciaModa = 0.0, desvioPadrao = 0.0;
  
   clockInit(PLL_FREQ);
   
   uartInit();

   GPIOInit();

   timerInit();
   TimerEnable(TIMER5_BASE, TIMER_A);

   systickInit(1);

   interruptInit();
  
  while(1)
  {     
    dados_lidos = 0;
    TimerValueSet(TIMER5_BASE,TIMER_A,0);
    

    while(dados_lidos < (N_DADOS+1))
    {
      if(leitura_realizada)
      {
        leitura_realizada = false;
        
         if(janela==JANELA_ESCALA_KILOHERTZ)
         {  SysTickDisable();             
            dadosFrequencia[dados_lidos++] = (1000.0 * (float)contagem/((float)janela));        
            
            
            TimerValueSet(TIMER5_BASE,TIMER_A,0);
            SysTickEnable();
         }
         else
         {
           if(contador_janela_hertz>=JANELA_ESCALA_HERTZ)
           {
             dadosFrequencia[dados_lidos++] = (1000.0 * (float)contagem/((float)janela));
             contador_janela_hertz = 0;
             TimerValueSet(TIMER5_BASE,TIMER_A,0);
           }
           contador_janela_hertz++;
         }        
      }
    }           
    
    
    //Descarta a primeira medida
    frequenciaMedia = Media((dadosFrequencia+1), N_DADOS);
    frequenciaMediana = Mediana((dadosFrequencia+1), N_DADOS);
    frequenciaModa = Moda((dadosFrequencia+1), N_DADOS);
    desvioPadrao = DesvioPadrao((dadosFrequencia+1), N_DADOS); 
    
    //frequencia = frequenciaMedia;
    frequencia = frequenciaMediana;
     //frequencia = frequenciaModa;
    if(janela == JANELA_ESCALA_KILOHERTZ)
    {      
      printFrequencia(frequencia/2.0);//borda de subida e descida   
       SysCtlDelay(PLL_FREQ);
    }
    else
      printFrequencia(frequencia/2.0);//borda de subida e descida   
   // printDesvio(desvioPadrao);

  } // while
} // main

void printFrequencia(float frequencia)
{
  uint32_t f_aux = 0;//formatar string 
  char aux_s[32];
  
  if(janela == JANELA_ESCALA_HERTZ)
  {
    f_aux =(uint32_t) ((uint64_t)(frequencia*1000)%1000);//todo
    sprintf(aux_s,"\nFrequencia: %u,%u[Hz]",(uint32_t)frequencia,f_aux);
  }
  else //kHz
  {
    f_aux = ((uint64_t)(frequencia*1000000)%1000);//todo    
    sprintf(aux_s,"\nFrequencia: %u,%u[kHz]",(uint32_t)(frequencia/1000),f_aux);
  }
  uart_puts(aux_s);
}

void printDesvio(float desvio)
{
  uint32_t f_aux = 0;//formatar string 
  char aux_s[32];
 
    f_aux =(uint32_t) ((uint64_t)(desvio*1000)%1000);//todo
    sprintf(aux_s,"Desvio: %u,%u[Hz]",(uint32_t)desvio,f_aux);
  uart_puts(aux_s);
}
//Callback interrupção do botão
void PortJIntHandler(void)
{    
  SysTickDisable();
  //troca janela
  
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
//  TimerDisable(TIMER5_BASE, TIMER_A);   //reseta timer
  TimerValueSet(TIMER5_BASE,TIMER_A,0);
  //TimerEnable(TIMER5_BASE, TIMER_A);
  
  SysTickEnable();
}

//Callback do systick
void SysTick_Handler(void)
{  
//  SysTickDisable();   
  contagem = TimerValueGet(TIMER5_BASE, TIMER_A); 
  leitura_realizada = true;    
}