#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
void die(char * s); 

int cria_socket() {
    int socket_int = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);//cria socket
    if(socket_int==-1) {
    	die("Erro ao criar socket");
    } else {
        return socket_int;
    }
}

struct sockaddr_in cria_socket_receiver(int socket_int, int porta) {
    struct sockaddr_in socketaddr;

    /*da valores como endereço e porta ao socket*/
    memset((char *) &socketaddr, 0, sizeof(socketaddr));//zera campos pra limpar lixo da memória
    socketaddr.sin_family = AF_INET;//valora como AF_INET, significa que é ipv4
    socketaddr.sin_port = htons(porta);//define porta, htons transforma long em big endian caso seja little
    socketaddr.sin_addr.s_addr = htonl(INADDR_ANY);//define ip, htonl transforma long em big endian caso seja little. INADDR_ANY significa todos os ips

    if(bind(socket_int , (struct sockaddr*)&socketaddr, sizeof(socketaddr) ) == -1) {//configura socket com endereço e porta pra que consiga escutar na porta
        die("Erro ao conectar socket com os endereços");
    }

    return socketaddr;
}

struct sockaddr_in cria_socket_sender(int socket_int, char* ip, int porta) {
    struct sockaddr_in socketaddr;
    /*da valores como endereço e porta ao socket*/
    memset((char *) &socketaddr, 0, sizeof(socketaddr));//zera campos pra limpar lixo da memória
    socketaddr.sin_family = AF_INET;//valora como AF_INET, significa que é ipv4
    socketaddr.sin_port = htons(porta);//define porta, htons transforma long em big endian caso seja little
    if(inet_aton(ip, &socketaddr.sin_addr) == 0) {//converte ip para o sin_addr da struct. Caso retorne 0 é pq deu erro
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }

    return socketaddr;
}

void die(char * s) {
	perror(s);
	exit(1);
}