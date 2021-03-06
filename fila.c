#include <pthread.h>
#include <stdbool.h>
#include <arpa/inet.h>
#define QTD_MENSAGENS 3
#define BUFFER_LENGTH 512
#define DEBUG 1   //define debug ativo ou inativo
/*
    A estrutura da mensagem a ser trocada pelos roteadores. Pode ser uma mensagem de controle ou mensagem de dado
*/
typedef struct {
    bool tipo;//0 = controle, 1 = dado;
    char* conteudo;
    struct sockaddr_in socket_externo;//informações do socket destino
} mensagem;

typedef struct {
	int id;
	uint16_t porta;
	char *ip;
} roteador_config;

typedef struct {
	int origem;
	int destino;
	int custo;
} enlace_config;

typedef struct {
    mensagem mensagens[QTD_MENSAGENS];
} fila_mensagens;

struct thread_arg_struct {
    roteador_config *roteador_config;
};

fila_mensagens fila_entrada;
pthread_mutex_t mutex_fila_entrada = PTHREAD_MUTEX_INITIALIZER;
int tamanho_atual_fila_entrada = 0;

fila_mensagens fila_saida;
pthread_mutex_t mutex_fila_saida = PTHREAD_MUTEX_INITIALIZER;
int tamanho_atual_fila_saida = 0;

/*Add elemento no final da fila*/
void fila_entrada_add(mensagem mensagem_nova) {
    //TODO, tratar se a fila tiver cheia ficar esperando.
    pthread_mutex_lock(&mutex_fila_entrada);
    fila_entrada.mensagens[tamanho_atual_fila_entrada] = mensagem_nova;
    tamanho_atual_fila_entrada++;
    pthread_mutex_unlock(&mutex_fila_entrada);
}

/*Remove elemento do inicio da fila*/
void fila_entrada_remove() {
    pthread_mutex_lock(&mutex_fila_entrada);
    for(int i = 0; i < tamanho_atual_fila_entrada; i++) {
        fila_entrada.mensagens[i] = fila_entrada.mensagens[i+1];
    }
    tamanho_atual_fila_entrada--;
    pthread_mutex_unlock(&mutex_fila_entrada);
}

mensagem fila_entrada_get() {
    pthread_mutex_lock(&mutex_fila_entrada);
    mensagem mensagem = fila_entrada.mensagens[0];
    pthread_mutex_unlock(&mutex_fila_entrada);
    return mensagem;
}

bool fila_entrada_tem_elementos() {
    pthread_mutex_lock(&mutex_fila_entrada);
    bool temElementos = (tamanho_atual_fila_entrada > 0) ? true : false;
    pthread_mutex_unlock(&mutex_fila_entrada);
    return temElementos;
}

/*Add elemento no final da fila*/
void fila_saida_add(mensagem mensagem_nova) {
    //TODO, tratar se a fila tiver cheia ficar esperando.
    pthread_mutex_lock(&mutex_fila_saida);
    fila_saida.mensagens[tamanho_atual_fila_saida] = mensagem_nova;
    tamanho_atual_fila_saida++;
    pthread_mutex_unlock(&mutex_fila_saida);
}

/*Remove elemento do inicio da fila*/
void fila_saida_remove() {
    pthread_mutex_lock(&mutex_fila_saida);
    for(int i = 0; i < tamanho_atual_fila_saida; i++) {
        fila_saida.mensagens[i] = fila_saida.mensagens[i+1];
    }
    tamanho_atual_fila_saida--;
    pthread_mutex_unlock(&mutex_fila_saida);
}

mensagem fila_saida_get() {
    pthread_mutex_lock(&mutex_fila_saida);
    mensagem mensagem = fila_saida.mensagens[0];
    pthread_mutex_unlock(&mutex_fila_saida);
    return mensagem;
}

bool fila_saida_tem_elementos() {
    pthread_mutex_lock(&mutex_fila_saida);
    bool temElementos = (tamanho_atual_fila_saida > 0) ? true : false;
    pthread_mutex_unlock(&mutex_fila_saida);
    return temElementos;
}
