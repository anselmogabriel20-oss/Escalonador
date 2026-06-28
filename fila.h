#ifndef FILA_H
#define FILA_H


typedef enum {PRONTO, EXECUTANDO, BLOQUEADO, TERMINADO} Status;

typedef enum {DISCO = 0, FITA = 1, IMPRESSORA = 2} TipoIO;

/* PCB guarda o contexto de cada processo. */
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


Fila *criar_fila(void);

int fila_vazia(Fila *fila);


void enfileira(Fila *fila, PCB *processo);


PCB *desenfileira(Fila *fila);


PCB *consultar_primeiro(Fila *fila);


void destruir_fila(Fila *fila);

#endif