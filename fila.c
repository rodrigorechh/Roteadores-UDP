#include <stdbool.h>
#include <arpa/inet.h>

/*
    A estrutura da mensagem a ser trocada pelos roteadores. Pode ser uma mensagem de controle ou mensagem de dado
*/
typedef struct {
    bool tipo_msg;//0 = controle, 1 = dado;
    char conteudo_msg[100];

    struct sockaddr_in socket_receiver;//informações do socket fonte
    int socket_fonte_int;
    int receiver_lenght;

    struct sockaddr_in socket_externo;//informações do socket destino
} mensagem;

typedef struct {
    pthread_mutex_t mutex;
    mensagem mensagens[10];
} fila_de_saida;

