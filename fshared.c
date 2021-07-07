#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <pthread.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>


int port ;
char dir[PATH_MAX] ;

////////set_option
void
set_option (int argc, char **argv, int *port, char *dir) {
	int opt ;
	while ((opt = getopt(argc, argv, "p:d:")) != -1) {
		switch (opt) {
			case 'p':
				*port = atoi(optarg) ;
				break ;
			case 'd':
				strcpy(dir, optarg) ;
		}
	}

	return ;
}

////////send_list
void
send_data (int sock_fd, char *message, int len) {
    int s ;
    while (len > 0 && (s = send(sock_fd, message, len, 0)) > 0) {
        printf("send size: %d\n", s) ; //hello\n
		message += s ;
        len -= s ;
    }
    return ;
}


void
send_list (int sock_fd) {

	DIR *dp = opendir(dir) ;
	if(dp == NULL)
		goto exit_opendir ;

	struct dirent *dep ;
	for(; dep = readdir(dp); ){

		if (strcmp(dep->d_name, ".") == 0)
			continue ;

		if (strcmp(dep->d_name, "..") == 0)
			continue ;

		if (dep->d_type == DT_LNK)
			continue ;

		if (dep->d_type == DT_DIR)
			continue ;

		if (dep->d_type == DT_REG) {
			printf("%s\n", dep->d_name) ;
			int len = strlen(dep->d_name) ;
			send_data(sock_fd, dep->d_name, len) ;
			send_data(sock_fd, "\n", 1) ;

		}

	} //for end

	shutdown(sock_fd, SHUT_WR) ;
	
	return ;

exit_opendir:
	perror("fail to opendir on 'void send_list'") ;
	exit(1) ;
}

// void // 퍄일 이름 받고 -> 확인하고 -> 결과 보내고 -> 열고 -> 보내기
void
recv_data (int sock_fd, char **file) {
	char buff[1024] ;
	int s ;
	int len ;
	while ((s = recv(sock_fd, buff, 1023, 0)) > 0) {
		buff[s] = 0x0 ;
		if (*file == 0x0) {
			*file = strdup(buff) ;
			len = s ;
		}
		else {
			*file = realloc(*file, len + s + 1) ;
			strncpy(*file + len, buff, s) ;
			len += s ;
			(*file) [len] = 0x0 ;
		}
	}
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
get (int sock_fd) {

	// 파일 이름 받기
	char *file = 0x0 ;
	recv_data(sock_fd, &file) ;


	// path 만들기
	char file_path[PATH_MAX] ;
	sprintf(file_path, "%s/%s", dir, file) ; // path_cat 으로 바꾸기 


	// 파일 열기
	FILE *fd = fopen(file_path, "rb") ;
	int check = 0;

	if (fd == NULL) 
		goto no_file ;
	else
		send_data(sock_fd, (char *) &check, sizeof(int)) ;


	// 파일의 내용 보내기
	send_file(sock_fd, fd) ;
	shutdown(sock_fd, SHUT_WR) ;


	fclose(fd) ;
	free(file) ;
	return ;

no_file: 
	printf("there is no %s\n", file_path) ;
	check = 1 ;
	send_data(sock_fd, (char *) &check, sizeof(int)) ;
	shutdown(sock_fd, SHUT_WR) ;
	return ;
}

// void // 파일 이름 받고 -> 데이터 받고 -> 파일 만들기
// put () { 

// }


////////child_proc
void *
child_proc(void *arg)
{
	int sock_fd = * (int *) arg ;

	int type  = 0 ;
	recv(sock_fd, &type, 4, 0) ; // while문 써서 더블체크로 바꾸기!!
	printf("type = %d\n", type) ;
	if (type == 1)
		send_list(sock_fd) ;

	if (type == 2)
		get(sock_fd) ;
	
	if (type == 3)
		// down_file() ;

	close(sock_fd) ;
	return NULL ;
}

int 
main(int argc, char **argv) 
{ 
	pthread_t thread ;
	char buffer[1024] = {0} ; 
	set_option(argc, argv, &port, dir) ;

	int listen_fd = socket(AF_INET /*IPv4*/, SOCK_STREAM /*TCP*/, 0 /*IP*/) ;
	if (listen_fd == 0)  { 
		perror("socket failed : ") ; 
		exit(EXIT_FAILURE) ; 
	}
	
	struct sockaddr_in address ;
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
		if(pthread_create(&thread, NULL, child_proc, &new_socket)) {
            perror("faile to pthread_create") ;
            exit(1) ;
        }
	}
} 
