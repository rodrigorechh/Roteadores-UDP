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

#define PORT 8888   //porta que será usada no socket
#define QTD_MAXIMA_ROTEADOR 20   //define quantidade máxima de roteadores que a rede suporta
#define DEBUG 1   //define debug ativo ou inativo


void *receiver(void *data);
void *sender(void *data);
void *packet_handler(void *data);
void *terminal(void *data);
void *roteadores(void *data); 
struct sockaddr_in cria_socket_receiver(int socket_int, int porta);
struct sockaddr_in cria_socket_sender(int socket_int, char* ip, int porta);
int cria_socket();
void die(char * s);
void carregarConfiguracaoRoteadores();
RoteadorConfig obterConfiguracaoRoteadorPorId(char * id);

pthread_t Thread1;
pthread_t Thread2;
pthread_t Thread3;
pthread_t Thread4;
pthread_t Thread5;

pthread_mutex_t mutex_sender = PTHREAD_MUTEX_INITIALIZER;

RoteadorConfig arrayConfiguracaoRoteadores[QTD_MAXIMA_ROTEADOR];

int main(int argc, char *argv[]) {
    carregarConfiguracaoRoteadores();

    char idRoteador[15];
	strncpy(idRoteador, argv[1], 15);

    if(DEBUG)
        printf("\n\nId do roteador é %s\n", idRoteador);

    obterConfiguracaoRoteadorPorId(idRoteador);
    return 1;

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

void carregarConfiguracaoRoteadores(){
    FILE * configuracaoRoteadores;
    
    char buffer[BUFFER_LENGTH];
    int totalLido = 0;
    int quantidadeRoteadores = 0;

    configuracaoRoteadores = fopen("roteador.config", "r");

    if (configuracaoRoteadores == NULL) {
        printf("Erro ao abrir o arquivo de configuração de roteador 'roteador.config'");
        exit(EXIT_FAILURE);
    }

    while(fgets(buffer, BUFFER_LENGTH, configuracaoRoteadores) != NULL) 
    {
        if(quantidadeRoteadores == QTD_MAXIMA_ROTEADOR){
            printf("A rede suporta no máximo %d roteadores", QTD_MAXIMA_ROTEADOR);
            exit(EXIT_FAILURE);
        }
        totalLido = strlen(buffer);

        buffer[totalLido - 1] = buffer[totalLido - 1] == '\n' ? '\0' : buffer[totalLido - 1];

        char * roteadorId;
        char * roteadorPorta;
        char * roteadorIP;
        
        roteadorId = strtok(buffer, "   ");
        roteadorPorta = strtok(NULL, "   ");
        roteadorIP = strtok(NULL, "   ");

        if(DEBUG)
            printf("Roteador %s: %s:%s\n", roteadorId, roteadorIP, roteadorPorta);

        arrayConfiguracaoRoteadores[quantidadeRoteadores].id = roteadorId;
        arrayConfiguracaoRoteadores[quantidadeRoteadores].porta = atoi(roteadorPorta);
        arrayConfiguracaoRoteadores[quantidadeRoteadores].ip = roteadorIP;

        quantidadeRoteadores++;
    } 
    
    if(DEBUG)
        printf("\n\n%d roteadores configurados\n", quantidadeRoteadores);

    fclose(configuracaoRoteadores);
}

RoteadorConfig obterConfiguracaoRoteadorPorId(char * id){
    for (int i = 0; i < QTD_MAXIMA_ROTEADOR; i++) {
        printf("\narrayConfiguracaoRoteadores[%d].id = %s", i, arrayConfiguracaoRoteadores[i].id);
		if (strcmp(id, arrayConfiguracaoRoteadores[i].id) == 0){
            return arrayConfiguracaoRoteadores[i];
        }
    }

    die(sprintf("A função 'obterConfiguracaoRoteadorPorId' não encontrou roteador com o id %s", id));
}

/*
    Thread que fica na escuta por novas mensagens em um socket que ele mesmo cria. 
    Quando recebe uma nova mensagem adiciona na fila_entrada onde ela aguardará ser processada pela
    função packet_handler.
*/
void *receiver(void *data) {
    int socket_int = cria_socket();
    struct sockaddr_in socket_receiver = cria_socket_receiver(socket_int, PORT);
    char buffer_local[BUFFER_LENGTH];

    struct sockaddr_in socket_externo;
    int socket_externo_tamanho = sizeof(socket_receiver);

    while(1)
    {
        printf("Esperando dados...");
        //fflush(stdout);
        memset(buffer_local,'\0', BUFFER_LENGTH);

        int receiver_length;
        /*fica aguardando mensagem chegar no socket s, qnd chegar a mensagem armazena no buffer,
         e a informação do socket de quem enviou a req armazena em si_externo.*/
        if ((receiver_length = recvfrom(socket_int, buffer_local, BUFFER_LENGTH, 0, (struct sockaddr *) &socket_externo, &socket_externo_tamanho)) == -1) {
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
