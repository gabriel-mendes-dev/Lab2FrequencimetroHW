#include <stdio.h>
#include <stdlib.h>
#include "Ordenacao.h"

/**
 * Ordena um vetor pelo método "select sort"
 * @param vetor Vetor a ser ordenado
 * @param size Tamanho do vetor
 * @return
 */
void selectSort(float vetor[], int size)
{
    int i,j,k;
    float minor, exchange;
    for(i=0;i<size-1;i++){
        exchange=0;
        minor=vetor[i];
        for(j=i+1;j<size;j++){
            if(vetor[j]<minor)
            {
                k=j;
                minor=vetor[j];
                exchange=1;
            }
        }
        if(exchange){
            vetor[k]=vetor[i];
            vetor[i]=minor;
        }
    }
    return;
}


/**
 * Ordena um vetor pelo método "insertion sort"
 * @param vetor Vetor a ser ordenado
 * @param size Tamanho do vetor
 * @return
 */
void insertionSort(float vetor[], int size)
{
    int i,j;
    float pivo;
    for(i=1; i<size ;i++)
    {
        pivo=vetor[i];
        for(j=i-1;j>=0&&vetor[j]>pivo;j--)
            vetor[j+1]=vetor[j];
        vetor[j+1]=pivo;
    }
    return;
}

