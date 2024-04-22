#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

typedef struct processo {
    int totalTime;
    char taskName[20]; 
    int period;
    int cpuBurst;
    int time;
    int priority;
    int taskExecution;
    int lastTaskExecution;
    char status[2]; // F, H, L, K
    int finishTask;
    int holdTask;
    int killedTask;
    int lostTask;
    int requests;
    struct processo *prox;
} Processo;

void lerArquivo(char *namefile, Processo **lista) {
    FILE *arquivo = fopen(namefile, "r");

    char linha[20];
    int count = 0;
    int numero;
    Processo *novo = NULL;

    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        if (count == 0) { // Primeira linha: tempo total
            numero = atoi(linha);
        } else if (count > 0) { // Daqui pra frente, processos
            novo = malloc(sizeof(Processo));
            char *elemento = strtok(linha, " ");
            strncpy(novo->taskName, elemento, sizeof(novo->taskName)); 
            elemento = strtok(NULL, " ");
            novo->period = atoi(elemento);
            elemento = strtok(NULL, " ");
            novo->cpuBurst = atoi(elemento);
            novo->time = count;
            novo->prox = NULL;

            Processo *ant = NULL;
            Processo *valor = *lista;

            while (valor != NULL && (valor->period < novo->period || (valor->period == novo->period && valor->time <= novo->time))) {
                ant = valor;
                valor = valor->prox;
            }

            if (ant == NULL) {
                novo->prox = *lista;
                *lista = novo;
            } else {
                ant->prox = novo;
                novo->prox = valor;
            }
        }
        count++;
    }
    
    if (*lista != NULL) { // Total time após ler todos os processos
        (*lista)->totalTime = numero;
    }

    fclose(arquivo);
}

void definirPrioridade(Processo *lista) {
    Processo *aux = lista;
    int prioridade = 0;
    while (aux != NULL) {
        aux->priority = prioridade;
        lista = lista->prox; 
        prioridade++;
        aux = aux->prox;
    }
}

void setFinish(Processo *lista) {
    Processo *aux = lista;
    char status[] = "F";
    while (aux != NULL) {
        if (aux->taskExecution == aux->cpuBurst) {
            aux->finishTask++;
            strncpy(aux->status, status, sizeof(aux->status));
            if(aux->lastTaskExecution != aux->taskExecution){
                printf("[%s] for %d units - %s\n", aux->taskName, (aux->taskExecution - aux->lastTaskExecution), aux->status);
                aux->lastTaskExecution = 0;
            }else{
                printf("[%s] for %d units - %s\n", aux->taskName, aux->taskExecution, aux->status);
            }
            aux->taskExecution = 0;
        }
        aux = aux->prox;
    }
} // F - Finish

void setHold(Processo *lista){ // Processo precisou ser interrompido e um processo com a prioridade mais alta precisou ser executado
    Processo *aux = lista;
    char status[] = "H";
    while (aux != NULL){
        // Verifica se a tarefa não está finalizada
        aux->holdTask++;
        strncpy(aux->status, status, sizeof(aux->status));
        aux->lastTaskExecution = aux->taskExecution;
        printf("[%s] for %d units - %s\n", aux->taskName, aux->taskExecution, aux->status);
        aux = aux->prox;
    }
} // H - Hold
void setLost(Processo *lista){ // Processo foi perdido (pode ter chegado ao seu periodo e foi necessario uma nova execução do mesmo processo)
    Processo *aux = lista;
    char status[] = "L";
    while (aux != NULL){
        aux->lostTask++;
        strncpy(aux->status, status, sizeof(aux->status));
        if(aux->taskExecution != aux->lastTaskExecution){
            printf("[%s] for %d units - %s\n", aux->taskName, (aux->taskExecution - aux->lastTaskExecution), aux->status);
        }else{
            printf("[%s] for %d units - %s\n", aux->taskName, aux->taskExecution, aux->status);
        }
        aux->taskExecution = 0;
        aux = aux->prox;
    }

} // L - Lost
void SetKilled(Processo *lista){ // Processo foi morto, pode ter chegado ao total time e não deu tempo de ser finalizado
    Processo *aux = lista;
    char status[] = "K";
    while (aux != NULL){
        aux->killedTask++;
        strncpy(aux->status, status, sizeof(aux->status));
        if(aux->taskExecution > 0){
            printf("[%s] for %d units - %s\n", aux->taskName, aux->taskExecution, aux->status);
        }
        aux->taskExecution = 0;
        aux = aux->prox;
    }
} // K - Killed

Processo *escolherTask(Processo *lista, int lastPriority) {
    Processo *tarefaAtual = lista;
    Processo *melhorTask = NULL;
    int menorPrioridade = INT_MAX; 

    while (tarefaAtual != NULL) {

        if (( tarefaAtual->priority > lastPriority)) {
            
            if (tarefaAtual->priority < menorPrioridade) {
                melhorTask = tarefaAtual;
                menorPrioridade = tarefaAtual->priority;

            }
        }
        tarefaAtual = tarefaAtual->prox;
    }

    if (melhorTask == NULL) { 
        tarefaAtual = lista;
        menorPrioridade = INT_MAX; 

        while (tarefaAtual != NULL) {
            if (tarefaAtual->taskExecution == 0 && tarefaAtual->priority < menorPrioridade) {
                melhorTask = tarefaAtual;
                menorPrioridade = tarefaAtual->priority;

            }
            tarefaAtual = tarefaAtual->prox;
        }
    }

    if (melhorTask != NULL) {
        melhorTask->requests--; 
    }

    return melhorTask;
}

void teste(Processo *lista) {
    printf("\n");
    Processo *aux = lista;
    
    printf("LOST DEADLINES\n");
    while (aux != NULL) {
        printf("[%s] %d\n", aux->taskName, aux->lostTask);
        aux = aux->prox;
    }

    aux = lista; 

    printf("\nCOMPLETE EXECUTION\n");
    while (aux != NULL) {
        printf("[%s] %d\n", aux->taskName, aux->finishTask);
        aux = aux->prox;
    }

    aux = lista; 

    printf("\nKILLED\n");
    while (aux != NULL) {
        printf("[%s] %d\n", aux->taskName, aux->killedTask);
        aux = aux->prox;
    }
}

void rate_monotonic(char *namefile) {
    printf("EXECUTION BY RATE\n");

    Processo *lista = NULL;
    lerArquivo(namefile, &lista);
    definirPrioridade(lista);

    Processo *currentTask = NULL;
    int lastPriority = -1;
    int count = 1;
    int idle = 0;

    while (count <= lista->totalTime) {
        Processo *aux = lista;
        Processo *temp = lista; 

        

        if (currentTask != NULL) { 
            if (currentTask->taskExecution == currentTask->cpuBurst) { // caso finish
                setFinish(currentTask); 
                currentTask->requests++;
                lastPriority = currentTask->priority; 
                if(currentTask->prox == NULL && !((count % temp->period) == 0)){
                    while(!((count % temp->period) == 0)){
                        idle++;
                        count++;
                    }    
                    idle++;
                    count++;
                    printf("idle for %d units\n", idle);
                }
                currentTask = NULL;   
                while (aux != NULL){
                    strncpy(aux->status, "F", sizeof(aux->status));
                    aux = aux->prox; 
                }   
            }else if((count % temp->period) == 0 && temp != currentTask && currentTask->priority > temp->priority){ // caso hold
                count++;
                currentTask->taskExecution++;
                setHold(currentTask);
                if(strcmp(currentTask->status, "H") == 0){
                    lastPriority = currentTask->priority;
                    currentTask = NULL;
                }
            }else if( (count % currentTask->period) == 0 && (lista->totalTime - count > currentTask->cpuBurst)) { // caso lost
                count++;
                currentTask->taskExecution++;
                setLost(currentTask);
            }else if(count == lista->totalTime){  // caso killed
                currentTask->taskExecution++;
                 SetKilled(currentTask);
                 currentTask = NULL;
                 break;
            }else{ // rodar unidade de tempo
                currentTask->taskExecution++;
                count++;
            }
        }
        
        if (currentTask == NULL) {    
            currentTask = escolherTask(lista, lastPriority);
            
            

        } 
    }

    teste(lista); // função só para imprimir valores e testar
}


void earliest_deadline_first(char *namefile) { // Implementação EDF
    printf("\nEXECUTION BY EDF");

    Processo *lista = NULL;
    lerArquivo(namefile, &lista);

    teste(lista);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {return 1;}
    
    char *namefile = argv[1]; // Nome arquivo de entrada
    char *algorithm = strrchr(argv[0], '/') + 1; // Pegar nome algoritmo

    freopen("rate.out", "w", stdout); 
    if (strcmp(algorithm, "rate") == 0) { // Rodar rate
        rate_monotonic(namefile);
    }
    return 0;
}