# Tuesday, July 6, 2021 : inotify
1. Today I learned about inotify(). inofity() function provide a monitoring of the file system and give effectiveness.

2. The overall flow of usage is 
+ Create inotify instance through inotify_init(). 
+ Determine which dir/file want to monitor through inotify_add_watch(). And through argument of this function, we can get information about created, deleted, modified, etc.. 

3. Today mission is making dropbox using inotify. However, there are still a lack of understanting about send() & recv() & block & non_block. So I study mainly this concepts after finish study group.

+ blocking 모드는 간단하게 소켓이 블락 되는 것이다. 따라서 블락된 상태에선 어떤 다른 일을 할 수 없게 된다. 하지만 fcntl()함수를 통해 non_blocking으로 설정하게 되면, 소켓 상태를 주기적으로 확인할 수 있는 기회가 생기게 된다. 결국 client와 server간에 좀 더 자유로운 소통이 가능해 진다고 이해했다. 
+ 또한 setsockopt()을 통해 TCP_NODELAY를 처음 사용 했을 땐, non_blocking으로 설정된 줄 알았는데 그게 아니라 fcntl()을 통해 설정하는 것을 깨닫게 되었다.
