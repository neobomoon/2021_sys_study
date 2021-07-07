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
+ 또한 setsockopt()을 통해 TCP_NODELAY를 처음 사용 했을 땐, non_blocking으로 설정된 줄 알았는데 그게 아니라 fcntl()을 통해 설정하는 것을 깨닫게 되었다.
