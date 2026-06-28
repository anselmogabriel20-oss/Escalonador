#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "fila.h"

#define max_processos 5
#define quantum 4
#define tempo_disco 4
#define tempo_fita 12
#define tempo_imp 20  
#define min_cpu 5
#define max_cpu 20
#define alta 0
#define baixa 1


typedef struct {
    int pid;
    int prioridade;
    int ppid;
    Status status;

    int tempo_serv;
    int tempo_exec;

    int tem_io;
    int instante_io;
    TipoIO tipo_io;
    int io_feito;
    int temporest_io;    
} PCB;

typedef struct No {
    PCB *processo;
    struct No *prox;
} No;

typedef struct {
    No *inicio;
    No *fim;
} Fila;


