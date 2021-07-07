#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <pthread.h>
#include <netinet/tcp.h>
#define MAX_CLIENT 16

typedef struct node {
    struct node *next ;
    int sock_fd ;
} Node ;

Node *head ;
int client = 0 ; // 만약 16이 넘어가면 어떻게 해야할까?
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER ;


void
insert_Node (int conn) {

    Node * node = malloc(sizeof(Node)) ;
    node->sock_fd = conn ;
    node->next = head ;
    head = node ;

    return ;
}

void
delete_Node (int conn) {

    Node *temp = head ;

    // 첫번째 노드면 바로 삭제
    if (temp->sock_fd == conn) {
        head = temp->next ;
        free(temp) ;

        return ;
    }

    // 노드 찾기
    while (temp->next->sock_fd != conn) {
        temp = temp->next ;

        // 특정 소켓 fd가 없으면 끝
        if (temp->next == NULL) {
            printf("there is no open socket\n") ;
            return ;
        }
    }

    // 노드 삭제
    Node *del_node = temp->next ;
    temp->next = del_node->next ;

    free(del_node) ;
    close(conn) ;

    return ;
}


///////////////////// worker
void
send_message (int conn, char *data, int len) {
    int s = 0 ;
    while (len > 0 && (s = send(conn, data, len, MSG_NOSIGNAL)) > 0) { // len 확인하기
        data += s ;
        len -= s ;
    }

    //printf("len: %d, s: %d\n", len, s) ;
    if (s == -1){
        //printf("return value %d\n", s) ;
        delete_Node(conn) ;
        client-- ;
        //printf("client value %d\n", client) ;

    }

}

void *
worker (void *arg){

    int conn = * (int *) arg ;
	char buff[1024] ;
	char * data = 0x0, * orig = 0x0 ;
	int len, s ;

    // race condition이 생길 수 있다 -> lock 걸기!
    pthread_mutex_lock(&mutex) ;
    // printf("insert_Node %d\n", conn) ;
    insert_Node(conn) ;
    client++ ;
    pthread_mutex_unlock(&mutex) ;
    // printf("insert_Node end %d\n", conn) ;


    while (1) {
        // while을 사용하면?
        // printf("check %d\n", conn) ;
        if ((s = recv(conn, buff, 1023, 0)) > 0 ) { //else 조건에 

            buff[s] = 0x0 ;

            if (data == 0x0) {
                data = strdup(buff) ;
                len = s ;
            }
            else { // 필요없다
                data = realloc(data, len + s + 1) ;
                strncpy(data + len, buff, s) ;
                data[len + s] = 0x0 ;
                len += s ;
            }

        }

        if (data != 0x0) {
            printf(">%s\n", data) ;

            // client1에게 보낼 때 client2로부터 온 메세지도 보내게 되면 섞여 나간다 -> lock필요!
            pthread_mutex_lock(&mutex) ;
            Node *temp = head ;
            printf("broadcast %d\n", client) ;
            for (int i = 0; i < client; i++){
                send_message (temp->sock_fd, data, len) ;
                temp = temp->next ;
            }
            pthread_mutex_unlock(&mutex) ;

        
        }

        data = 0x0 ;
        memset(buff, 0, sizeof(buff)) ;
    
    } // while end

    return 0  ;
}

int 
main(int argc, char const *argv[]) 
{ 
	int new_socket ; 
	pthread_t thread ;	
    struct sockaddr_in address; 
    int addrlen = sizeof(address) ;
    head = NULL ;


	int listen_fd = socket(AF_INET /*IPv4*/, SOCK_STREAM /*TCP*/, 0 /*IP*/) ;
	if (listen_fd == 0) { 
		perror("socket failed : "); 
		exit(EXIT_FAILURE); 
	}

	memset(&address, '0', sizeof(address)); 
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY /* the localhost*/ ; 
	address.sin_port = htons(8080); 

	if (bind(listen_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
		perror("bind failed : "); 
		exit(EXIT_FAILURE); 
	} 

	while (1) {

		if (listen(listen_fd, 16 /* the size of waiting queue*/) < 0) { 
			perror("listen failed : "); 
			exit(EXIT_FAILURE); 
		} 

		new_socket = accept(listen_fd, (struct sockaddr *) &address, (socklen_t*) &addrlen) ;
		if (new_socket < 0) {
			perror("accept"); 
			exit(EXIT_FAILURE); 
		}

        int flag = 1 ;
        if(setsockopt(new_socket, SOL_TCP, TCP_NODELAY, &flag, sizeof(int))){
            perror("fail to setsockopt") ;
            exit(1) ;
        }

		if(pthread_create(&thread, NULL, worker, &new_socket)) {
            perror("faile to pthread_create") ;
            exit(1) ;
        }

	}
} 
