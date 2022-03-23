// server side
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
#include<sys/stat.h>



int port ;
char dir[PATH_MAX] ;

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

void
recv_s (int sock_fd, char *type, int len) {
	int s ;
	while (len > 0 && (s = recv(sock_fd, type, len, 0)) > 0) {
		type += s ;
		len -= s ;
	}
}

void
send_data (int sock_fd, char *message, int len) {
    int s ;
    while (len > 0 && (s = send(sock_fd, message, len, 0)) > 0) {
        printf("send size: %d\n", s) ;
		message += s ;
        len -= s ;
    }
    return ;
}


////////list
void
list (int sock_fd) {

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


///////get
void
recv_data (int sock_fd, char **data) {
	char buff[1024] ;
	int s ;
	int len ;
	while ((s = recv(sock_fd, buff, 1023, 0)) > 0) {
		buff[s] = 0x0 ;
		if (*data == 0x0) {
			*data = strdup(buff) ;
			len = s ;
		}
		else {
			*data = realloc(*data, len + s + 1) ;
			strncpy(*data + len, buff, s) ;
			len += s ;
			(*data) [len] = 0x0 ;
		}
	}
}

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
	if (fd == NULL || isreg(file_path)) 
		goto no_file ;
	else
		send_data(sock_fd, (char *) &check, sizeof(int)) ;


	// 파일의 내용 보내기
	char buff[1024] ;
	int len ;

	while (!feof(fd)) {
		len = fread(buff, 1, sizeof(buff), fd) ;
		send_data(sock_fd, buff, len) ;
	}

	shutdown(sock_fd, SHUT_WR) ;


	fclose(fd) ;
	free(file) ;
	return ;

no_file: 
	printf("there is no or not regular file\n") ;
	check = 1 ;
	send_data(sock_fd, (char *) &check, sizeof(int)) ;
	shutdown(sock_fd, SHUT_WR) ;
	return ;
}


//////put
void // 파일 이름 받고 -> 데이터 받고 -> 파일 만들기
put (int sock_fd) { 
	// 파일 길이 받고
	int file_len ;
	recv_s(sock_fd, (char *) &file_len, sizeof(file_len)) ;
	
	// 파일 이름 받기
	char *file = (char *) malloc(sizeof(char) * file_len) ;
	recv_s(sock_fd, file, file_len) ;

	// path 만들기
	char file_path[PATH_MAX] ;
	sprintf(file_path, "%s/%s", dir, file) ; // path_cat 으로 바꾸기 

	printf("file len : %d, file name : %s, file path : %s\n", file_len, file, file_path) ;

	// 파일 생성하기
    FILE *fd = fopen(file_path, "wb") ;
    if (fd == NULL) {
        perror("fail to fopen") ;
        exit(1) ;
    }
    
    
    // client가 보내는 file내용 받기
    char buff[1024] ;
    int s ;
    while ((s = recv(sock_fd, buff, 1024, 0)) > 0) {
        fwrite(buff, s , 1, fd) ;
    }
    printf("%s\n", buff) ;
    

    fclose(fd) ;
	free(file) ;

	return ;
}


////////child_proc
void *
child_proc(void *arg)
{
	int sock_fd = * (int *) arg ;

	int type ;
	recv_s(sock_fd, (char *) &type, 4) ;

	printf("type = %d\n", type) ;
	if (type == 1)
		list(sock_fd) ;

	if (type == 2)
		get(sock_fd) ;
	
	if (type == 3)
		put(sock_fd) ;

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
