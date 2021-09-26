#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include "fila.c"

#define BUFLEN 512  //tamanho do buffer de leitura
#define PORT 8888   //porta que será usada no socket

void *receiver(void *data);
void *sender(void *data);
void *packet_handler(void *data);
void *terminal(void *data);
void *roteadores(void *data);
int cria_socket(int porta);
void die(char * s);

pthread_t Thread1;
pthread_t Thread2;
pthread_t Thread3;
pthread_t Thread4;
pthread_t Thread5;

struct sockaddr_in si_me, si_other;
int recv_len;
fila_de_saida fila_saida = {.mutex = PTHREAD_MUTEX_INITIALIZER};

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
    int socket_int = cria_socket(PORT);
    char buffer_local[BUFLEN];

    struct sockaddr_in socket_externo;
    int socket_externo_tamanho = sizeof(si_other);

    while(1)
    {
        printf("Esperando dados...");
        fflush(stdout);
        memset(buffer_local,'\0', BUFLEN);

        /*fica aguardando mensagem chegar no socket s, qnd chegar a mensagem armazena no buffer,
         e a informação do socket de quem enviou a req armazena em si_other.*/
        if ((recv_len = recvfrom(socket_int, buffer_local, BUFLEN, 0, (struct sockaddr *) &socket_externo, &socket_externo_tamanho)) == -1) {
            die("recvfrom()");
        }

        //implementar a adição na fila_entrada. Precisa de mutex por conta da concorrência com a thread packet_handler
    }

    printf("Thread responsavel pelo recebimento\n");
    pthread_exit(NULL);
}

/*
    Responsável por esvaziar esporadicamente a fila_saida
    enviando seus elementos para o próximo roteador.
    A thread fica dormindo enquanto a fila_saida está vazia
*/
void *sender(void *data){
    printf("Thread responsavel pelo envio\n");
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
    printf("Thread responsavel pelo processamento das mensagens\n");
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

int cria_socket(int porta) {
    /*Cria socket vazio*/
    int socket_int = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);//cria socket
    if(socket_int==-1) {
    	die("Erro ao criar socket");
    }

    /*da valores como endereço e porta ao socket*/
    memset((char *) &si_me, 0, sizeof(si_me));//zera campos pra limpar lixo da memória
    si_other.sin_family = AF_INET;//valora como AF_INET, significa que é ipv4
    si_me.sin_port = htons(porta);//define porta, htons transforma long em big endian caso seja little
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);//define ip, htonl transforma long em big endian caso seja little

    if(bind(socket_int , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1) {//conecta socket com endereço e porta
        die("Erro ao conectar socket com os endereços");
    }

    return socket_int;
}

void die(char * s) {
	perror(s);
	exit(1);
}