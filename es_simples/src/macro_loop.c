#include "macro_loop.h"

void n_loop(uint8_t* vetor_estados, volatile uint32_t* WAVE_INPUT_END)
{
  asm("push {r1,r2,r4}");
  asm("mov r1, %[var]\n" ::[var] "r" (WAVE_INPUT_END)); 
  asm("mov r2, %[var]\n" ::[var] "r" (vetor_estados-1)); 
  
  N_LOOP( asm("LDR.W R4, [R1]"); asm("STRB R4, [R2,#1]!");)//LDR:2 ciclos STR!:3 ciclos
  asm("pop {r1,r2,r4}");
      
}