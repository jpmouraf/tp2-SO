#ifndef _PSTAT_H_
#define _PSTAT_H_

#include "param.h"

struct pstat {
  int inuse[NPROC];   // se o slot da tabela de processos está em uso (1 ou 0) 
  int tickets[NPROC]; // o número de tickets que este processo tem 
  int pid[NPROC];     // o PID de cada processo 
  int ticks[NPROC];   // o número de ticks que cada processo acumulou 
};

#endif