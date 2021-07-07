#include <netinet/tcp.h>
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <string.h> 
#include <pthread.h>



void *
send_message (void *arg) {

    int conn = * (int *) arg ;
    int len, s ;
    char buff[1024] = {0} ;
    char *data ;

    while (1) {
        printf("check\n") ;
        char c ;
        int i ;
        for (i = 0; (c = getc(stdin)) != '\n'; i++) {
            buff[i] = c ;
        }
        buff[i] = 0x0 ;

        data = buff ;
        len = strlen(buff) ;
        s = 0 ;

        while (len > 0 && (s = send(conn, data, len, 0)) > 0) {
            data += s;
            len -= s;
        }

        data = 0x0 ;
        memset(buff, 0, sizeof(buff)) ;

    }

    return 0 ;
}

void *
recv_message (void *arg) {
    int conn = * (int *) arg ;
    int len, s ;
    char buff[1024] = {0} ;
    char *data = 0x0;

    while (1) {

        s = 0 ;
        len = 0 ;

        if ((s = recv(conn, buff, 1023, 0)) > 0) {
            buff[s] = 0x0 ;
            if (data == 0x0) {
                data = strdup(buff) ;
                len = s ;
            }
            else {
                data = realloc(data, len + s + 1) ;
                strncpy(data + len, buff, s) ;
                len += s ;
                data[len] = 0x0 ;
            }

        }

        if (data != 0x0)
            printf(">%s\n", data) ;

        data = 0x0 ;
        memset(buff, 0, sizeof(buff)) ;
    } // while end

    return 0 ;

}

int 
main(int argc, char const *argv[]) 
{ 

	int sock_fd = socket(AF_INET, SOCK_STREAM, 0) ;
	if (sock_fd <= 0) {
		perror("socket failed : ") ;
		exit(EXIT_FAILURE) ;
	} 

	struct sockaddr_in serv_addr; 
	memset(&serv_addr, '\0', sizeof(serv_addr)); 
	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(8080); 
	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
		perror("inet_pton failed : ") ; 
		exit(EXIT_FAILURE) ;
	} 

    int flag = 1 ;
    if(setsockopt(sock_fd, SOL_TCP, TCP_NODELAY, &flag, sizeof(int))){ 
        perror("fail to setsockopt") ;
        exit(1) ;
    }

	if (connect(sock_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("connect failed : ") ;
		exit(EXIT_FAILURE) ;
	}

    pthread_t send_thread, recv_thread ;
    if(pthread_create(&send_thread, NULL, send_message, &sock_fd)) {
        perror("faile to pthread_create") ;
        exit(1) ;
    }
    if(pthread_create(&recv_thread, NULL, recv_message, &sock_fd)) {
        perror("faile to pthread_create") ;
        exit(1) ;
    }

    pthread_join(send_thread, NULL) ;
    pthread_join(recv_thread, NULL) ;
    
    return 0 ;
} 
