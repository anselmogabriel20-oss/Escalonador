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

PCB *criar_processo(int pid) {
    PCB *p = malloc(sizeof(PCB));
 
    p->pid = pid;
    p->ppid = 0;             // nao tem relacao pai/filho 
    p->prioridade = alta;    // processo novo entra na fila de alta prioridade 
    p->status = PRONTO;
 
    p->tempo_serv = min_cpu + rand() % (max_cpu - min_cpu + 1);
    p->tempo_exec = 0;
 
    p->tem_io = rand() % 2;  // 50% de chance de pedir I/O 
    if (p->tem_io) {
        p->tipo_io = rand() % 3;                  // DISCO, FITA ou IMPRESSORA 
        p->instante_io = rand() % p->tempo_serv;  
        p->io_feito = 0;
    } else {
        p->tipo_io = DISCO;  
        p->instante_io = -1;
        p->io_feito = 1;     // marca como "ja feito"  
    }
    p->temporest_io = 0;
 
    return p;
}

const char *nome_status(Status s) {
    switch (s) {
        case PRONTO:     return "PRONTO";
        case EXECUTANDO: return "EXECUTANDO";
        case BLOQUEADO:  return "BLOQUEADO";
        case TERMINADO:  return "TERMINADO";
    }
    return "";
}

int duracao_io(TipoIO t) {
    switch (t) {
        case DISCO:      return tempo_disco;
        case FITA:       return tempo_fita;
        case IMPRESSORA: return tempo_imp;
    }
    return 0;
}

/* Imprime os PIDs presentes em uma fila, sem remove-los. */
void imprime_fila(const char *nome, Fila *fila) {
    printf("%-12s [", nome);
    No *atual = fila->inicio;
    while (atual != NULL) {
        printf("P%d", atual->processo->pid);
        if (atual->prox != NULL) printf(", ");
        atual = atual->prox;
    }
    printf("]\n");
}

void imprime_estado(int clock, PCB *cpu,
                     Fila *fila_alta, Fila *fila_baixa,
                     Fila *fila_disco, Fila *fila_fita, Fila *fila_imp,
                     PCB *processos[], int n) {
    printf("\nCLOCK %d \n", clock);
 
    if (cpu != NULL) {
        printf("CPU: P%d (%d/%d)\n", cpu->pid, cpu->tempo_exec, cpu->tempo_serv);
    } else {
        printf("CPU: livre\n");
    }
 
    imprime_fila("Fila alta:",   fila_alta);
    imprime_fila("Fila baixa:",  fila_baixa);
    imprime_fila("Fila disco:",  fila_disco);
    imprime_fila("Fila fita:",   fila_fita);
    imprime_fila("Fila impr.:",  fila_imp);
 
    printf("Processos:\n");
    for (int i = 0; i < n; i++) {
        PCB *p = processos[i];
        printf("  P%d - %-10s - exec %d/%d\n",
               p->pid, nome_status(p->status), p->tempo_exec, p->tempo_serv);
    }
}

int main(void) {
    srand((unsigned int) time(NULL));
 
    Fila *fila_alta  = criar_fila();
    Fila *fila_baixa = criar_fila();
    Fila *fila_disco = criar_fila();
    Fila *fila_fita  = criar_fila();
    Fila *fila_imp   = criar_fila();
 
    PCB *processos[max_processos];
    for (int i = 0; i < max_processos; i++) {
        processos[i] = criar_processo(i + 1);
        enfileira(fila_alta, processos[i]);
    }
 
    // So o processo na frente de cada fila de I/O esta sendo atendido 
    Fila *filas_io[3]  = {fila_disco, fila_fita, fila_imp};
    Fila *retorno_io[3] = {fila_baixa, fila_alta, fila_alta}; // disco->baixa e fita/impressora->alta 
 
    PCB *cpu = NULL;
    int quantum_usado = 0;
    int terminados = 0;
    int clock = 0;
 
    while (terminados < max_processos) {
        int evento = 0;
 
        // 1. Avanca o tempo de I/O de quem esta sendo atendido
        for (int i = 0; i < 3; i++) {
            if (!fila_vazia(filas_io[i])) {
                PCB *p = consultar_primeiro(filas_io[i]);
                p->temporest_io--;
                if (p->temporest_io <= 0) {
                    desenfileira(filas_io[i]);
                    p->status = PRONTO;
                    enfileira(retorno_io[i], p);
                    evento = 1;
                }
            }
        }
 
        // 2. Se a CPU estiver livre, escolhe o proximo processo obs: prioriza a fila de alta prioridade
        if (cpu == NULL) {
            if (!fila_vazia(fila_alta)) {
                cpu = desenfileira(fila_alta);
            } else if (!fila_vazia(fila_baixa)) {
                cpu = desenfileira(fila_baixa);
            }
            if (cpu != NULL) {
                cpu->status = EXECUTANDO;
                quantum_usado = 0;
                evento = 1;
            }
        }
 
        // 3. Executa o processo da CPU por 1 unidade de tempo. 
        if (cpu != NULL) {
            cpu->tempo_exec++;
            quantum_usado++;
 
            if (cpu->tempo_exec >= cpu->tempo_serv) {
                //Processo terminou. 
                cpu->status = TERMINADO;
                terminados++;
                cpu = NULL;
                quantum_usado = 0;
                evento = 1;
            } else if (cpu->tem_io && !cpu->io_feito && cpu->tempo_exec >= cpu->instante_io) {
                // Processo pede I/O. 
                cpu->status = BLOQUEADO;
                cpu->io_feito = 1;
                cpu->temporest_io = duracao_io(cpu->tipo_io);
 
                if (cpu->tipo_io == DISCO)      enfileira(fila_disco, cpu);
                else if (cpu->tipo_io == FITA)  enfileira(fila_fita,  cpu);
                else                             enfileira(fila_imp,   cpu);
 
                cpu = NULL;
                quantum_usado = 0;
                evento = 1;
            } else if (quantum_usado >= quantum) {
                // Quantum esgotou: preempcao pra  fila de baixa prioridade
                cpu->status = PRONTO;
                cpu->prioridade = baixa;
                enfileira(fila_baixa, cpu);
                cpu = NULL;
                quantum_usado = 0;
                evento = 1;
            }
        }
 
        if (evento) {
            imprime_estado(clock, cpu, fila_alta, fila_baixa,
                           fila_disco, fila_fita, fila_imp,
                           processos, max_processos);
        }
 
        clock++;
    }
 
    printf("\nTodos os processos terminaram -> CLOCK final: %d.\n", clock);
 
    destruir_fila(fila_alta);
    destruir_fila(fila_baixa);
    destruir_fila(fila_disco);
    destruir_fila(fila_fita);
    destruir_fila(fila_imp);
 
    for (int i = 0; i < max_processos; i++) {
        free(processos[i]);
    }
 
    return 0;
}
 


