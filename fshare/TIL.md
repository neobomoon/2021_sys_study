# Wednesday, June 30, 2021
1. What is socket? Communication between programs on a network. Therefore, nowdays most of the computer networks use socket for communicate each other. And there are some data structure and function which can help socket.

2. Flow of TCP/IP

![image](https://user-images.githubusercontent.com/70479118/123979229-7f424400-d9fb-11eb-94c3-c347a3c80a06.png)

3. int socket(int domain, int type, int protocol) ;
+ description
    It is like open socket.
+ int domain
    A parameter that determines whether to communicate on the Internet or between processes within the same system.. Most of case, 'PF_INET'(IPv4) is used for           domain.
+ int type
    Data transfer type. There are two tpyes which 'SOCK_STREAM'(TCP/IP) and 'SOCK_DGRAM'(UDP/IP).
+ int protocol
    Most of case, 0 is used for protocol.
+ return
    It return socket descriptor, likes namepipe.
    fail : -1.
    success : socket descriptor.
    
4. int bind(int sock_fd, struct sockaddr * myaddr, socklen_t addrlen) ;
+ description
    Assign the IP address and port number to the socket. Therefore, it is to prepare the socket to be used for communication.
+ int sock_fd
    Socket descriptor.
+ struct sockaddr * myaddr
    A struct sockaddr is entered for assignment to the socket.
+ socket_t addrlen
    The sizeof(myaddr) is entered.
+ return
    fail : -1.
    success : 0.
    
5. int listen(int sock_fd, int queue_size) ;
+ description
    The number of requests that can be processed from clients.
+ int sock_fd
    Socket descriptor.
+ int queue_size
    Waiting queue size.
+ return
    fail : -1.
    success : 0.
6. int accept(int sock_fd, struct sockaddr * addr, socklen_t * addrlen) ;
+ description
    It accepts the client's connect() request and creates a dedicated socket to communicate with the client.
+ int s
    Socket descriptor.
+ struct sockaddr * addr
    Pointer of the structure containing the address of the client.
+ socklen_t * addrlen
    The sizeof(addr) is entered.
----------------------------------------------


# Thursday, July 1, 2021
1. Until now, I don't fully understand  about 'pointer" in C language. So I confused when using double pointer in sys_study time. After search the pointer concept in C, I cleared up some confusing concepts and make a diagram which help makes sense by myself.
+ What I confused. When I run this code, Segmentfault is occured.
<img width="348" alt="스크린샷 2021-07-01 오후 10 54 52" src="https://user-images.githubusercontent.com/70479118/124136046-5d11fa00-dabf-11eb-8b49-c43c29806cca.png">
+ Solution code.
<img width="343" alt="스크린샷 2021-07-01 오후 10 57 56" src="https://user-images.githubusercontent.com/70479118/124136481-cabe2600-dabf-11eb-960f-8a6bba4aafc4.png">
+ What I understand.
<img width="636" alt="스크린샷 2021-07-01 오후 11 42 30" src="https://user-images.githubusercontent.com/70479118/124143341-0360fe00-dac6-11eb-8263-9c4738ed3ecf.png">


So, In main(), the data which type is pointer should be used to call test() function by argument with reference which means '&'.
When do malloc() in test(), the argument should be dereference. Therefore, after finished test() function, data variable which declared in main() can not occurs segmentation fault.=
----------------------------------------------



# Friday, July 2, 2021 : Socket programming in client
1. About struct sockaddr_in
+ struct
```
struct sockaddr_in {
    short sin_family ; // 주소체계 : AF_INET (IPv4) 사용
    u_short sin_port ; // 포트번호
    struct in_addr sin_addr ; // IP 주소
    char sin_zero[8] ; // dummy 값 (구조체 크기를 16bytes로 맞추기 위해. 꼭 '0'으로 채우기
}
```
ex)
```
struct sockaddr_in sock_addr ;
```

+ Initialize sock_addr through memset()

ex)
```
memset(&sock_addr, '0', sizeof(sock_addr)) ;
```

+ Set sin_port through htons(). htons is 'Host to Network' and 's' means short(2bytes).

ex)
```
sock_addr.sin_port = htons(8080) ;
```

+ Set sin_addr through inet_pton(). pton is 'Pointer to Number'. Thus, IP arrays is turned to binary. If IP is vaild value, it returns 1. If IP is not vaild value, it returns 0. If IP is not proper address_family, it returns -1.

ex)
```
inet_pton(AF_INET, "127.0.0.1", &sock_addr.sin_addr)
```

2. How to ues socket programming

+ Frist, Open file descriptor through socket() function. If that function succeeds, it returns 0 or more becuase it is file descriptor. If that function fails, it returns -1.

ex)
```
int sock_fd = socket(AF_INET, SOCK_STREAM, 0) ;
if(sock_fd < 0){
    perror("fail to socket") ;
    exit(1) ;
}
```

+connet() function requests to server for connection using socket descriptor. And it returns 0 on success and -1 on failure.

ex)
```
if(connect(sock_fd, (struct sockaddr * ) &sock_addr, sizeof(sock_addr)) < 0){
    perror("fail to connect") ;
    exit(1) ;
}
```

+ send() function sends data to server. And It returns the number of bytes which function sends. However, we need to prepare for one case that some data is not sent.(Do double check)

ex)
```
char buf[1024] = "test" ;
char * data = buf ;
int s ;
int len = strlen(buf) ;
while(len > 0 && (s = send(sock_fd, data, 1023, 0)) > 0) { // 1023은 NULL을 넣을 공간을 위해
    data += s ;
    len -= s;
}
```

+ shutdown() function can close write side/read side/both on sock_fd. If the write side is closed, then server starts recive the data from client. And it returns 0 on succeess and returns -1 on failure.

ex)
```
if(shutdown(sock_fd, SHUT_WR) != 0){
    perror("fail to shutdonw") ;
    exit(1) ;
}
```

----------------------------------------------



# Tuesday, July 6, 2021 : inotify
1. Today I learned about inotify(). inofity() function provide a monitoring of the file system and give effectiveness.

2. The overall flow of usage is 
+ Create inotify instance through inotify_init(). 
+ Determine which dir/file want to monitor through inotify_add_watch(). And through argument of this function, we can get information about created, deleted, modified, etc.. 

3. Today mission is making dropbox using inotify. However, there are still a lack of understanting about send() & recv() & block & non_block. So I study mainly this concepts after finish study group.

+ blocking 모드는 간단하게 소켓이 블락 되는 것이다. 따라서 블락된 상태에선 어떤 다른 일을 할 수 없게 된다. 하지만 fcntl()함수를 통해 non_blocking으로 설정하게 되면, 소켓 상태를 주기적으로 확인할 수 있는 기회가 생기게 된다. 결국 client와 server간에 좀 더 자유로운 소통이 가능해 진다고 이해했다. 
----------------------------------------------


# Wendsday, July 7, 2021
1. 오늘은 쓰레기 값이 나왔던 'list'와 구하지 못 했던 'get'의 코드를 짰다. 처음에 non_blocking으로 이루어 지는 networking이 잘 이해되지 않아서 많은 시간을 투자했다. 내가 계속 하고 있었던 실수는 server에서 보내는 처음 헤더 4 바이트를 읽고, 다음 동작을 해야 하는데 recv()가 while문 안에 갇혀있어서 server에서 보내는 즉시 바로 읽어대고 있었다. 그래서 그 다음 단계의 recv에선 아무리 socket을 읽어도 아무것도 읽지 못한 것이다. 얼마전에 setsockopt()을 공부하면서 어느정도 이해하고 있다고 생각했지만, 아직 덜 이해한듯 보여 스터디 시간이 끝난 후 보충 공부를 좀 더 하였다.
2. socket network는 때떄로 보내고 싶은 데이터를 다 못 보낼 때가 있다고 했다. 따라서 내가 보내고 싶은 데이터를 확실히 보내고, 받을 수 있는 더블체크가 꼭 필요하다. 더블체크는 while문으로 충분히 구현할 수 있다.
3. 'list'기능이 잘 안 됐던 이유는 너무 복잡하게 생각해서 recv()를 할 때 잘못된 바이트 수 만큼 읽고 있었다. 그 결과 client는 file list를 읽은 뒤 뒤에 쓰레기 값 까지 출력했다.
4. 'get' 기능을 구현하기 위해선, client가 server에서 가져오고 싶은 파일을 요청한다. 그렇게 되면 server는 파일이 존재하는지 체크한 뒤 int값으로 client에게 전송해준다. 그럼 client는 읽은 int값에 따라 에러처리를 한 후, 그 다음 server에서 보내는 파일의 데이터를 받아 파일을 생성하게 구현했다.
