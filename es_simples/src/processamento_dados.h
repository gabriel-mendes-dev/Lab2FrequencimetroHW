#ifndef _PROCESSAMENTO_DADOS_
#define _PROCESSAMENTO_DADOS_

#include <stdint.h>

float Media(float * dados, uint32_t n_dados);
float Mediana(float *dados, uint32_t n_dados);
float Moda(float *dados, uint32_t n_dados);
float DesvioPadrao(float *dados, uint32_t n_dados);

#endif