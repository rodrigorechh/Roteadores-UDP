#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include "fila.c"
#include "socket_utils.c"

#define BUFLEN 512  //tamanho do buffer de leitura
#define PORT 8888   //porta que será usada no socket

void *receiver(void *data);
void *sender(void *data);
void *packet_handler(void *data);
void *terminal(void *data);
void *roteadores(void *data); 
struct sockaddr_in cria_socket_receiver(int socket_int, int porta);
struct sockaddr_in cria_socket_sender(int socket_int, char* ip, int porta);
int cria_socket();
void die(char * s);

pthread_t Thread1;
pthread_t Thread2;
pthread_t Thread3;
pthread_t Thread4;
pthread_t Thread5;

pthread_mutex_t mutex_sender = PTHREAD_MUTEX_INITIALIZER;

int main(void) {
    //-- Cria as Threads --
    pthread_create(&Thread1, NULL, receiver, NULL); //prioridade NULL - padrão
    pthread_create(&Thread2, NULL, sender, NULL);
    pthread_create(&Thread3, NULL, packet_handler, NULL);
    pthread_create(&Thread4, NULL, terminal, NULL);

    //Faz um Join - Retorno das Threads
    pthread_join(Thread1, NULL);
    printf("Thread id %ld returned\n", Thread1);
    pthread_join(Thread2, NULL);
    printf("Thread id %ld returned\n", Thread2);
    pthread_join(Thread3, NULL);
    printf("Thread id %ld returned\n", Thread3);
    pthread_join(Thread4, NULL);
    printf("Thread id %ld returned\n", Thread4);

    return 1;
}

/*
    Thread que fica na escuta por novas mensagens em um socket que ele mesmo cria. 
    Quando recebe uma nova mensagem adiciona na fila_entrada onde ela aguardará ser processada pela
    função packet_handler.
*/
void *receiver(void *data) {
    int socket_int = cria_socket();
    struct sockaddr_in socket_receiver = cria_socket_receiver(socket_int, PORT);
    char buffer_local[BUFLEN];

    struct sockaddr_in socket_externo;
    int socket_externo_tamanho = sizeof(socket_receiver);

    while(1)
    {
        printf("Esperando dados...");
        //fflush(stdout);
        memset(buffer_local,'\0', BUFLEN);

        int receiver_length;
        /*fica aguardando mensagem chegar no socket s, qnd chegar a mensagem armazena no buffer,
         e a informação do socket de quem enviou a req armazena em si_externo.*/
        if ((receiver_length = recvfrom(socket_int, buffer_local, BUFLEN, 0, (struct sockaddr *) &socket_externo, &socket_externo_tamanho)) == -1) {
            die("recvfrom()");
        }

        //implementar a adição na fila_entrada. Precisa de mutex por conta da concorrência com a thread packet_handler
        mensagem msg = {.socket_fonte_int = socket_int, .conteudo = buffer_local, .receiver_length = receiver_length, .socket_externo = socket_externo};
        fila_entrada_add(msg);
    }

    pthread_exit(NULL);
}

/*
    Responsável por esvaziar esporadicamente a fila_saida
    enviando seus elementos para o próximo roteador.
    A thread fica dormindo enquanto a fila_saida está vazia
*/
void *sender(void *data){
    while(1) {
        if(fila_saida_tem_elementos()){
            mensagem msg = fila_saida_get();
            fila_saida_remove();
            if (sendto(msg.socket_fonte_int, msg.conteudo, msg.receiver_length, 0, (struct sockaddr*) &msg.socket_externo, sizeof(msg.socket_externo)) == -1) {//manda a mensagem de volta ao cliente a partir do socket s, manda o conteúdo do buf, vai mandar de volta pro socket si_other, resto dos param são 0 ou o tamanho max dos outros parametros
                die("sendto()");
            }
        } else {
            sleep(1);
        }
    }
    pthread_exit(NULL);
}

/*
    Essa thread ocorre após a Receiver, vai pegar as mensagens
    da fila_entrada.
    Mensagens de controle (roteamento) devem ser processadas pelo algoritmo
    Bellman-Ford distribuído. Mensagens de dados, devem ser processadas
    adequadamente. Isto é, caso o roteador atual seja o destino da mensagem,
    trata-se a mensagem localmente; caso contrário, a mesma deve ser
    encaminhada para o roteador vizinho segundo informações da tabela de
    roteamento.
*/
void *packet_handler(void *data){
    while(1) {
        if(fila_entrada_tem_elementos()) {
            mensagem msg = fila_entrada_get();
            fila_entrada_remove();
            fila_saida_add(msg);
        }
    }
    pthread_exit(NULL);
}

/*
    Thread dedicada ao terminal para permitir interações do usuário
*/
void *terminal(void *data){
    printf("Thread responsavel pelo terminal\n");
    pthread_exit(NULL);
}

/*config inicial*/
void configInicial() {
    //ler arquivo roteador.config
    //ler arquivo enlaces.config
}
