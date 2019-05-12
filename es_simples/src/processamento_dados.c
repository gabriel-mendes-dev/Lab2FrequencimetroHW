#include "processamento_dados.h"
#include "Ordenacao.h"
#include <math.h>
#include <stdint.h>

float Media(float * dados, uint32_t n_dados)
{
    uint32_t i;
    float soma = 0.0;
    for(i = 0; i < n_dados; i++)
    {
        soma += dados[i];
    }
    uint64_t t = soma/n_dados;
    return (float)(soma/((float)n_dados));
}

float Mediana(float * dados, uint32_t n_dados)
{
    insertionSort(dados, n_dados);
  //  uint64_t help = (float)((n_dados%2 == 0)? ((dados[n_dados/2 -1] + dados[n_dados/2])/2) : dados[n_dados/2]);
    return (float)((n_dados%2 == 0)? ((dados[n_dados/2 -1] + dados[n_dados/2])/2) : dados[n_dados/2]);
}

float Moda(float * dados, uint32_t n_dados)
{
    uint32_t i, contagemAtual = 1, contagemMax = 1;
    float moda = 0;
    insertionSort(dados, n_dados);
    for(i = 0; i < n_dados - 1; i++)
    {
        if(dados[i] != dados[i+1])
        {
            if(contagemAtual > contagemMax)
            {
                contagemMax = contagemAtual;
                moda = dados[i];
            }
            contagemAtual = 0;
        }
        contagemAtual ++;
    }
    if(contagemAtual > contagemMax)
    {
        moda = dados[i];
    }
    return moda;
}

float DesvioPadrao(float * dados, uint32_t n_dados)
{
    float media = Media(dados, n_dados);
    float SDQ = 0;
    uint32_t i;
    for (i = 0; i < n_dados; i++)
    {
        SDQ += pow(dados[i] - media, 2); 
    }
    return sqrt(SDQ)/((float)n_dados - 1.0);
}