#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include "fila.c"
#include "socket_utils.c"

#define BUFFER_LENGTH 512  //tamanho do buffer de leitura
#define PORT 8888   //porta que será usada no socket
#define IP "127.0.0.1"

void *receiver(void *data);
void *sender(void *data);
void *packet_handler(void *data);
void *terminal(void *data);
void *roteadores(void *data); 
struct sockaddr_in cria_socket_receiver(int socket_int, char* ip, int porta);
int cria_socket();
void die(char * s);
void add_socket_destino_fila_saida(char* conteudo, char* ip, int porta);

pthread_t Thread1;
pthread_t Thread2;
pthread_t Thread3;
pthread_t Thread4;
pthread_t Thread5;

int socket_int;
struct sockaddr_in sockaddr;

int main(void) {

    socket_int = cria_socket();
    sockaddr = cria_socket_receiver(socket_int, IP , PORT);

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
    struct sockaddr_in socket_externo;
    int socket_externo_tamanho = sizeof(socket_externo);
    char buffer_local[BUFFER_LENGTH];

    while(1) {
        printf("Esperando dados...");
        //fflush(stdout);
        memset(buffer_local,'\0', BUFFER_LENGTH);

        int receiver_length;
        /*fica aguardando mensagem chegar no socket s, qnd chegar a mensagem armazena no buffer,
         e a informação do socket de quem enviou a req armazena em si_externo.*/
        if ((recvfrom(socket_int, buffer_local, BUFFER_LENGTH, 0, (struct sockaddr *) &socket_externo, &socket_externo_tamanho)) == -1) {
            die("recvfrom()");
        } else {
            printf("Recebido requisição...");
            fflush(stdout);
        }

        //implementar a adição na fila_entrada. Precisa de mutex por conta da concorrência com a thread packet_handler
        mensagem msg = {.conteudo = buffer_local, .socket_externo = socket_externo};
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
            if (sendto(socket_int, msg.conteudo, strlen(msg.conteudo), 0, (struct sockaddr*) &msg.socket_externo, sizeof(msg.socket_externo)) == -1) {//manda a mensagem de volta ao cliente a partir do socket s, manda o conteúdo do buf, vai mandar de volta pro socket si_other, resto dos param são 0 ou o tamanho max dos outros parametros
                die("sendto()");
            } else {
                printf("Enviada requisição...");
                fflush(stdout);
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
    char ip[9], conteudo[BUFFER_LENGTH];
    int porta;
    printf("Escreva o ip do roteador: \n");
    scanf("%s", ip);

    printf("Escreva a porta do roteador: \n");
    scanf("%i", &porta);

    printf("Escreva o conteúdo da mensagem: \n");
    scanf("%s", conteudo);

    add_socket_destino_fila_saida("msg", IP, PORT);

    pthread_exit(NULL);
}

/*config inicial*/
void configInicial() {
    //ler arquivo roteador.config
    //ler arquivo enlaces.config
}

void add_socket_destino_fila_saida(char* conteudo, char* ip, int porta) {
    struct sockaddr_in sockaddr_other = cria_socket_sender(cria_socket(), ip, 8889);
    mensagem msg = {.conteudo = conteudo, .socket_externo = sockaddr_other};
    fila_saida_add(msg);
}