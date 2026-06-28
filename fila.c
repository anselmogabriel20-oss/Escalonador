#include <stdlib.h>
#include "fila.h"

Fila *criar_fila(void) {
    Fila *fila = malloc(sizeof(Fila));
    fila->inicio = NULL;
    fila->fim = NULL;
    return fila;
}

int fila_vazia(Fila *fila) {
    return fila->inicio == NULL;
}

void enfileira(Fila *fila, PCB *processo) {
    No *novo = malloc(sizeof(No));
    novo->processo = processo;
    novo->prox = NULL;

    if (fila_vazia(fila)) {
        fila->inicio = novo;
    } else {
        fila->fim->prox = novo;
    }
    fila->fim = novo;
}

PCB *desenfileira(Fila *fila) {
    if (fila_vazia(fila)) return NULL;

    No *no = fila->inicio;
    PCB *processo = no->processo;

    fila->inicio = no->prox;
    if (fila->inicio == NULL) {
        fila->fim = NULL;
    }
    free(no);
    return processo;
}

PCB *consultar_primeiro(Fila *fila) {
    if (fila_vazia(fila)) return NULL;
    return fila->inicio->processo;
}

void destruir_fila(Fila *fila) {
    No *atual = fila->inicio;
    while (atual != NULL) {
        No *prox = atual->prox;
        free(atual);
        atual = prox;
    }
    free(fila);
}