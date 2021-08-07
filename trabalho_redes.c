#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void *receiver(void *data);
void *sender(void *data);
void *packet_handler(void *data);
void *terminal(void *data);
void *roteadores(void *data); 

pthread_t Thread1;
pthread_t Thread2;
pthread_t Thread3;
pthread_t Thread4;
pthread_t Thread5;


//pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;



int main(void) {
    //-- Cria as Threads --
    pthread_create(&Thread1, NULL, receiver, NULL); //prioridade NULL - padrão
    pthread_create(&Thread2, NULL, sender, NULL);
    pthread_create(&Thread3, NULL, packet_handler, NULL);
    pthread_create(&Thread4, NULL, terminal, NULL);
    pthread_create(&Thread5, NULL, roteadores, NULL);

    //Faz um Join - Retorno das Threads
    pthread_join(Thread1, NULL);
    printf("Thread id %ld returned\n", Thread1);
    pthread_join(Thread2, NULL);
    printf("Thread id %ld returned\n", Thread2);
    pthread_join(Thread3, NULL);
    printf("Thread id %ld returned\n", Thread3);
    pthread_join(Thread4, NULL);
    printf("Thread id %ld returned\n", Thread4);
    pthread_join(Thread5, NULL);
    printf("Thread id %ld returned\n", Thread5);


    return 1;
    

}


void *receiver(void *data){
     printf("Thread responsavel pelo recebimento\n");
     pthread_exit(NULL);
}

void *sender(void *data){
    printf("Thread responsavel pelo envio\n");
    pthread_exit(NULL);
}

void *packet_handler(void *data){
    printf("Thread responsavel pelo processamento das mensagens\n");
    pthread_exit(NULL);
}

void *terminal(void *data){
    printf("Thread responsavel pelo terminal\n");
    pthread_exit(NULL);
}

void *roteadores(void *data){
    printf("Threads roteadores\n");
    struct ficha_de_aluno
    {
        char tipo_mensagem[50];
        int end_fonte;
        int end_destino;
        char mensagen[100];
    };
    //Fazer fila de entrada
    //Fazer fila de saída
    pthread_exit(NULL);
}







