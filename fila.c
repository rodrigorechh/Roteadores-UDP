#include <pthread.h>
#include <stdbool.h>
#include <arpa/inet.h>
#define QTD_MENSAGENS 3

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
    mensagem mensagens[QTD_MENSAGENS];
} fila_de_saida;

pthread_mutex_t mutex_fila_saida = PTHREAD_MUTEX_INITIALIZER;
int tamanho_atual_fila_saida = 0;

/*Add elemento no final da fila*/
void fila_saida_add(mensagem mensagem_nova, fila_de_saida fila_saida) {
    pthread_mutex_lock(&mutex_fila_saida);
    tamanho_atual_fila_saida++;
    fila_saida.mensagens[tamanho_atual_fila_saida] = mensagem_nova;
    pthread_mutex_unlock(&mutex_fila_saida);
}

/*Remove elemento do inicio da fila*/
void fila_saida_remove(fila_de_saida fila_saida) {
    pthread_mutex_lock(&mutex_fila_saida);
    for(int i = 0; i < tamanho_atual_fila_saida; i++) {
        fila_saida.mensagens[i] = fila_saida.mensagens[i+1];
    }
    tamanho_atual_fila_saida--;
    pthread_mutex_unlock(&mutex_fila_saida);
}

mensagem fila_saida_get(fila_de_saida fila_saida) {
    pthread_mutex_lock(&mutex_fila_saida);
    return fila_saida.mensagens[0];
    pthread_mutex_unlock(&mutex_fila_saida);
}

bool existe_elemento_fila_saida() {
    return (tamanho_atual_fila_saida == 0) ? false : true;
}