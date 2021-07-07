#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <string.h> 
#include <limits.h>

typedef struct header{
    int type;
    int file_len;
    char file_name[PATH_MAX];
}Header;

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

void
send_file (int sock_fd, FILE *fd) {
	char buff[1024] ;
	int len ;

	while (!feof(fd)) {
		len = fread(buff, 1, sizeof(buff), fd) ;
		send_data(sock_fd, buff, len) ;
	}

}

void
send_file_data(int sock_fd, char *file_name){
    //파일 열고
    FILE *fd = fopen(file_name, "r") ;
    if(fd == NULL)
        goto exit_fopen ;

    //읽은 만큼 보내주기
    char buff[512] ;
    char *data = NULL ;
    size_t size, len ; 
    int s ;
    while(feof(fd) == 0){
        size = fread(buff, 1, sizeof(buff), fd) ;
        len = strlen(buff) ;
        data = buff ;
        while(len > 0 && (s = send(sock_fd, buff, len, 0)) > 0){
            data += s ;
            len -= s;
        }
    }

    fclose(fd) ;
    return ;

exit_fopen:
    perror("fail to fopen") ;
    exit(1) ;
}

////////recv_info
void
recv_list (int sock_fd) {// 여러가지 틀릴 가능성이 너무 많다.
    char buff[1024] ;
    char *data = 0x0 ;
    int s ;
    int len ;
    //버퍼사이즈보다 클 경우 대비하기
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
            data[len] = 0x0 ;
        }

    } // while end

    printf("%s\n", data) ;

    free(data) ;

    return ;
}

void
checking (int sock_fd, char *check) {
    int len = 4 ;
    int s ;
    while (len > 0 && (s = recv(sock_fd, check, sizeof(int), 0))) { // 보낼 바이트 수 ' - len' 해주기!
        check += s ;
        len -= s;
    }
}
void
recv_get (int sock_fd, char *file) {

    // 파일 여부 확인하기
    int check = 0;
    checking(sock_fd, (char *) &check) ;
    // recv(sock_fd, &check, sizeof(int), 0) ;

    if (check) {
        perror("server has not file") ;
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


int 
main(int argc, char **argv) 
{   
    Header header ;
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

    if (type == 1) {
        shutdown(sock_fd, SHUT_WR) ;
        recv_list(sock_fd) ; // recive 'list'
    }

    if(type == 2) {
        send_data(sock_fd, file, sizeof(file)) ; // send 'file name'
        shutdown(sock_fd, SHUT_WR) ;
        recv_get(sock_fd, file) ; // recive 'file data' & 'create'
    }

    if(type == 3) {
        // send_data(sock_fd, file, sizeof(file)) ; // send 'file name'
        // send_file_data(sock_fd, file) ; // send 'file data'
        // shutdown(sock_fd, SHUT_WR) ;
        // recv_put() ; // 성공여부
    }



    free(ip) ;
    return 0 ;

} 
//더 자세히 천천히