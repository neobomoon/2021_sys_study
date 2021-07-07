#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <pthread.h>

void
send_message(char *message, int len, int conn){
	int s ;
	while(len > 0 && (s = send(conn, message, len, 0)) > 0){
		message += s;
		len -= s;
	}
}
FILE *
open_file(char*file, char* opt){
	FILE *fd = fopen(file, opt) ;
	if(fd == NULL){
		perror("fail to fopen") ;
		exit(1) ;
	}
	return fd ;
}

void
read_file(char * file, int conn){
	FILE *fd = open_file(file, "r") ;

	char buf[512] ;
	while(feof(fd) == 0){
		int len = (int) fread(buf, 1, sizeof(buf), fd) ;
		//읽은건 정확하게 send 하기
		send_message(buf, len, conn) ;
	}
	fclose(fd) ;

	return ;
}

int
check_file(char *file, int conn){
	if(access(file, F_OK) == -1){
		char *message = "there is no file." ;
		send_message(message, (int) strlen(message), conn) ;
		return 0 ;
	}
	return 1;
}


void *
child_proc(void *arg)
{
	int conn = * (int *) arg ;
	char buf[1024] ;
	char * file = 0x0 ;
	int len = 0 ;
	int s ;
	while ( (s = recv(conn, buf, 1023, 0)) > 0 ) {
		buf[s] = 0x0 ;
		if (file == 0x0) {
			file = strdup(buf) ;
			len = s ;
		}
		else {
			file = realloc(file, len + s + 1) ;
			strncpy(file + len, buf, s) ;
			file[len + s] = 0x0 ;
			len += s ;
		}
	}

	printf(">%s\n", file) ;
	if(check_file(file, conn)){
		read_file(file, conn) ;
	}

	shutdown(conn, SHUT_WR) ;

	if (file != 0x0) 
		free(file) ;

	return NULL ;
}

int 
main(int argc, char const *argv[]) 
{ 
	pthread_t thread ;
	int listen_fd ; 
	struct sockaddr_in address ; 

	char buffer[1024] = {0} ; 

	listen_fd = socket(AF_INET /*IPv4*/, SOCK_STREAM /*TCP*/, 0 /*IP*/) ;
	if (listen_fd == 0)  { 
		perror("socket failed : ") ; 
		exit(EXIT_FAILURE) ; 
	}
	
	memset(&address, 0, sizeof(address)); 
	address.sin_family = AF_INET ; 
	address.sin_addr.s_addr = INADDR_ANY /* the localhost*/ ; 
	address.sin_port = htons(8080) ; 
	if (bind(listen_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
		perror("bind failed : ") ; 
		exit(EXIT_FAILURE) ; 
	} 

	while (1) {
		if (listen(listen_fd, 16 /* the size of waiting queue*/) < 0) { 
			perror("listen failed : ") ; 
			exit(EXIT_FAILURE) ; 
		} 
        
        int addrlen = sizeof(address) ; 
		int new_socket = accept(listen_fd, (struct sockaddr *) &address, (socklen_t*)&addrlen) ;
		if (new_socket < 0) {
			perror("accept") ; 
			exit(EXIT_FAILURE) ; 
		} 

		pthread_create(&thread, NULL, child_proc, &new_socket) ;
	}
} 
