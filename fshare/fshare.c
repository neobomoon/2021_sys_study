#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <string.h> 
#include <limits.h>
#include<sys/stat.h>



///////setoption
void
set_ip_port(char *ip_port, char **ip, int *port){

    int ind;

    for( ind = strlen(ip_port) - 1; ip_port[ind] != ':' && ind > -1; ind-- ) ;

    *port = atoi( &ip_port[ind + 1] );
    ip_port[ind] = '\0' ;

    *ip = strdup(ip_port) ;
}

void
set_option(int argc, char **argv, int *type, char *file, char **ip, int *port){
    if(argc == 3 && (strcmp(argv[2], "list") == 0)) // list인 경우
        *type = 1 ;
    else if(argc == 4){ // get, put인 경우
        strcpy(file, argv[3]) ;
        if(strcmp(argv[2], "get") == 0)
            *type = 2 ;
        else if(strcmp(argv[2], "put") == 0)
            *type = 3 ;
        else
            goto exit_err ;
    }
    else
        goto exit_err ;

    set_ip_port(argv[1], &(*ip), port) ;

    return ;

exit_err:
    perror("wrong option") ;
    exit(1) ;
}

////////send
void
send_data (int sock_fd, char *message, int len) {
    int s ;
    while(len > 0 && (s = send(sock_fd, message, len, 0)) > 0){
        message += s ;
        len -= s ;
    }
    return ;
}

///////////recv
void
recv_s (int sock_fd, char *type, int len) {
	int s ;
	while (len > 0 && (s = recv(sock_fd, type, len, 0)) > 0) {
		type += s ;
		len -= s ;
	}
}

//////////list
void
list (int sock_fd){

    shutdown(sock_fd, SHUT_WR) ;
    
    char *data = 0x0 ;
    char buff[1024] ;
	int s ;
	int len ;
	while ((s = recv(sock_fd, buff, 1023, 0)) > 0) {
		buff[s] = 0x0 ;
		if (data == 0x0) {
			data = strdup(buff) ;
			len = s ;
		}
		else {
			data = realloc(data, len + s + 1) ;
			strncpy(data + len, buff, s) ;
			len += s ;
			(data) [len] = 0x0 ;
		}
	}

    printf("%s\n", data) ;

    free(data) ;

    return ;
}

//////get
void
checking (int sock_fd, char *check) {
    int len = 4 ;
    int s ;
    while (len > 0 && (s = recv(sock_fd, check, len - s, 0)) > 0) { // 보낼 바이트 수 ' - len' 해주기!
        check += s ;
        len -= s;
    }
}

void
get (int sock_fd, char *file) {

    send_data(sock_fd, file, strlen(file)) ; // send 'file name'

    shutdown(sock_fd, SHUT_WR) ;

    // 파일 여부 확인하기
    int check = 0;
    recv_s(sock_fd, (char *) &check, 4) ;
    
    if (check) {
        perror("server has not file or not regular file") ;
        exit(1) ;
    }


    // 파일 생성하기
    FILE *fd = fopen(file, "wb") ;
    if (fd == NULL) {
        perror("fail to fopen") ;
        exit(1) ;
    }
    
    
    // server가 보내는 file내용 받기
    char buff[1024] ;
    int s ;
    while ((s = recv(sock_fd, buff, 1023, 0)) > 0) {
        fwrite(buff, s , 1, fd) ;
    }
    printf("%s, %d\n", buff, s) ;
    
    fclose(fd) ;

    return ;

}

///////put
int
isreg (char *file_path) {
	struct stat st ;
	if (stat(file_path, &st) == -1) {
		perror("fail to stat") ;
		exit(1) ;
	}

	if (S_ISREG(st.st_mode)) 
		return 0 ;
	else
		return 1 ;
}

void
put (int sock_fd, char *file) {
    int len = strlen(file) ;

    //파일 길이 보내기
    send_data(sock_fd, (char *) &len, sizeof(len)) ;

    //파일이름 보내기
    send_data (sock_fd, file, len) ;

    // 파일 열기
	FILE *fd = fopen(file, "rb") ;
	int check = 0;
	if (fd == NULL || isreg(file)) 
		goto no_file ;

	// 파일의 내용 보내기
	char buff[1024] ;
	len = 0 ;

	while (!feof(fd)) {
		len = fread(buff, 1, sizeof(buff), fd) ;
		send_data(sock_fd, buff, len) ;
	}


    shutdown(sock_fd, SHUT_WR) ;

    return ;

no_file:
    perror("fail to fopen or not regular file") ;
    exit(1) ;
}


int 
main(int argc, char **argv) 
{   
    char file[PATH_MAX] = {0};
    int type ;
    int port ;
    char *ip ;
    set_option( argc, argv, &type, file, &ip, &port ) ;


    int sock_fd ;
    struct sockaddr_in serv_addr ;
    sock_fd = socket( AF_INET, SOCK_STREAM, 0 ) ;
    if( sock_fd <= 0 ){
        perror( "fail to socket" ) ;
        exit(1) ;
    }
    memset( &serv_addr, '0', sizeof(serv_addr) ) ;
    serv_addr.sin_family = AF_INET ;
    serv_addr.sin_port = htons( port ) ;
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("fail to inet_pton") ;
        exit(1) ;
    }

    if (connect(sock_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("fail to connect") ;
        exit(1) ;
    }


    // type 보내기
    send_data(sock_fd, (char *) &type, sizeof(type)) ;

    if (type == 1)
        list(sock_fd) ;

    if (type == 2) 
        get(sock_fd, file) ;
    
    if (type == 3) {
        put(sock_fd, file) ;
    }



    free(ip) ;
    return 0 ;

} 
//더 자세히 천천히
