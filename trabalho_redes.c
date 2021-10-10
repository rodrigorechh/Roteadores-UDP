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
#define QTD_MAXIMA_ROTEADOR 20   //define quantidade máxima de roteadores que a rede suporta
#define QTD_MAXIMA_ENLACES 100   //define quantidade máxima de enlaces que a rede suporta

void *receiver(void *data);
void *sender(void *data);
void *packet_handler(void *data);
void *terminal(void *data);
void *roteadores(void *data); 
struct sockaddr_in cria_socket_receiver(int socket_int, char* ip, int porta);
int cria_socket();
void die(char * s);
void add_socket_destino_fila_saida(char* conteudo, char* ip, int porta);
void carregar_config_roteadores();
void carregar_enlaces_roteadores();
roteador_config obter_config_roteador_por_id(int id);

pthread_t Thread1;
pthread_t Thread2;
pthread_t Thread3;
pthread_t Thread4;
pthread_t Thread5;

int socket_int = cria_socket();
struct sockaddr_in sockaddr;

roteador_config array_config_roteadores[QTD_MAXIMA_ROTEADOR];
enlace_config array_config_enlaces[QTD_MAXIMA_ENLACES];

int main(int argc, char *argv[]) {
    carregar_config_roteadores();
    carregar_enlaces_roteadores();

    char tmpIdRoteador[15];
	strncpy(tmpIdRoteador, argv[1], 15);
    int idRoteador = atoi(tmpIdRoteador);

    if(DEBUG)
        printf("\n\nId do roteador do processo atual é %d\n", idRoteador);

    roteador_config config_roteador_atual = obter_config_roteador_por_id(idRoteador);

    if(DEBUG)
        printf("\nConfiguração do roteador do processo atual é %s:%d\n", config_roteador_atual.ip, config_roteador_atual.porta);

    //-- Define os argumentos das threads
    struct thread_arg_struct *args = (struct thread_arg_struct *)malloc(sizeof(struct thread_arg_struct));
    args->roteador_config = &config_roteador_atual;


    //-- Inicia socket que será usado para receber(thread receiver) e enviar(thread sender) requisições
    struct thread_arg_struct *args = (struct thread_arg_struct *)data;
	struct sockaddr_in socket_receiver = cria_socket_receiver(socket_int, args->roteador_config->ip, args->roteador_config->porta);
    
    //-- Cria as Threads --
    pthread_create(&Thread1, NULL, receiver, (void *)args); //prioridade NULL - padrão
    pthread_create(&Thread2, NULL, sender, (void *)args);
    pthread_create(&Thread3, NULL, packet_handler, (void *)args);
    pthread_create(&Thread4, NULL, terminal, (void *)args);

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

void carregar_enlaces_roteadores(){
    FILE * configuracao_enlaces;
    
    char buffer[BUFFER_LENGTH];
    int total_lido = 0;
    int quantidade_enlaces = 0;
    char separador[] = "      ";

    configuracao_enlaces = fopen("enlaces.config", "r");

    if (configuracao_enlaces == NULL) {
        printf("\nErro ao abrir o arquivo de configuração de enlaces 'enlaces.config'");
        exit(EXIT_FAILURE);
    }

    if(DEBUG)
        printf("\nIniciando carregamento da configuração de enlaces");

    while(fgets(buffer, BUFFER_LENGTH, configuracao_enlaces) != NULL) 
    {
        total_lido = strlen(buffer);

        buffer[total_lido - 1] = buffer[total_lido - 1] == '\n' ? '\0' : buffer[total_lido - 1];
       
        char roteador_origem[10];
        char roteador_destino[10];
        char roteador_custo[20];

        char * ptr = strtok(buffer, separador);
        strcpy(roteador_origem, ptr);

        ptr = strtok(NULL, separador);
        strcpy(roteador_destino, ptr);

        ptr = strtok(NULL, separador);
        strcpy(roteador_custo, ptr);

        if(DEBUG)
            printf("\nEnlace de %s até %s -> %s", roteador_origem, roteador_destino, roteador_custo);
        
        array_config_enlaces[quantidade_enlaces].origem = atoi(roteador_origem);
        array_config_enlaces[quantidade_enlaces].destino = atoi(roteador_destino);
        array_config_enlaces[quantidade_enlaces].custo = atoi(roteador_custo);

        quantidade_enlaces++;
    } 
    
    fclose(configuracao_enlaces);

    if(DEBUG)
        printf("\nCarregamento da configuração de %d enlaces finalizada!!", quantidade_enlaces);
}

void carregar_config_roteadores(){
    FILE * configuracao_roteadores;
    
    char buffer[BUFFER_LENGTH];
    int total_lido = 0;
    int quantidade_roteadores = 0;
    char separador[] = "   ";

    configuracao_roteadores = fopen("roteador.config", "r");

    if (configuracao_roteadores == NULL) {
        printf("\nErro ao abrir o arquivo de configuração de roteador 'roteador.config'");
        exit(EXIT_FAILURE);
    }

    if(DEBUG)
        printf("\nIniciando carregamento da configuração de roteadores");

    while(fgets(buffer, BUFFER_LENGTH, configuracao_roteadores) != NULL) 
    {
        if(quantidade_roteadores == QTD_MAXIMA_ROTEADOR){
            printf("\nA rede suporta no máximo %d roteadores", QTD_MAXIMA_ROTEADOR);
            exit(EXIT_FAILURE);
        }

        total_lido = strlen(buffer);

        buffer[total_lido - 1] = buffer[total_lido - 1] == '\n' ? '\0' : buffer[total_lido - 1];
       
        char roteador_id[4];
        char roteador_porta[10];
        char roteador_IP[16];

        char * ptr = strtok(buffer, separador);
        strcpy(roteador_id, ptr);

        ptr = strtok(NULL, separador);
        strcpy(roteador_porta, ptr);

        ptr = strtok(NULL, separador);
        strcpy(roteador_IP, ptr);

        if(DEBUG)
            printf("\nRoteador %s: %s:%s", roteador_id, roteador_IP, roteador_porta);
        
        array_config_roteadores[quantidade_roteadores].id = atoi(roteador_id);
        array_config_roteadores[quantidade_roteadores].porta = atoi(roteador_porta);
        array_config_roteadores[quantidade_roteadores].ip = roteador_IP;

        quantidade_roteadores++;
    } 
    
    fclose(configuracao_roteadores);

    if(DEBUG)
        printf("\nCarregamento da configuração de %d roteadores finalizada!!", quantidade_roteadores);
}

roteador_config obter_config_roteador_por_id(int id){
    for (int i = 0; i < QTD_MAXIMA_ROTEADOR; i++) {
		if (id == array_config_roteadores[i].id){
            return array_config_roteadores[i];
        }
    }

    char msg_erro[100];
    sprintf(msg_erro, "A função 'obter_config_roteador_por_id' não encontrou roteador com o id %d", id);
    die(msg_erro);
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

    while(1)
    {
        if(DEBUG)
            printf("\nThread receiver está esperando dados...");

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