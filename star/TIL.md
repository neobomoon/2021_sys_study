# star TIL
# Monday, June 28, 2021
1. Until now, I did't know about that the simplicity of code is important thing. Today I realized its importance.

2. The most important difference between binary files and text files is that reading stops when reading null in text files, but not when reading null in binary files.

3. The difference between strcpy() and strdup() is that malloc() is used when user call strdup().

4. It is recommended to set the minimum byte for read and write to 512 bytes.

5. If user use malloc, realloc can be uesd for convenience.

6. The struct dirent is structured like this. Also, the format of the file can be known by d_type.
+ struct dirent
<img width="529" alt="스크린샷 2021-06-28 오후 8 17 53" src="https://user-images.githubusercontent.com/70479118/123628328-ecb07200-d84d-11eb-8fd1-a02c4cad84e3.png">

+ d_type
<img width="409" alt="스크린샷 2021-06-28 오후 8 20 05" src="https://user-images.githubusercontent.com/70479118/123628587-3c8f3900-d84e-11eb-8f37-4e8cc72e1f1d.png">
----------------------------------------------



# Tuesday, June 29, 2021
1. Today I studied about fread(), fwrite(). 
+ size_t fread(void * prt, size_t size, size_t count, FILE * fp) ;

    ptr : Pointer of the buffer with a size of the at least (size * count).

    size : Size of one element.

    count : Number of elements to read.

    fp : Pointer of FILE.

    return value : The total number of elements successfully read.

    ex : fread(buffer, 1, sizeof(buffer), fp) ;


+ fwrite(void * ptr, size_t size, size_t count, FILE * fp) ;

    ptr : Pointer of the buffer to be written

    size : Size of one element to be written.

    count : Number of elements to be written.

    fp : Pointer of FILE.

    return value : The total number of elements successfully written.

    ex : fwrite(buffer, 1, sizeof(buffer), fp);

2. I leared about 'goto' instruction. This simplifies the code when handling errors and increases readability.
+ goto
<img width="415" alt="스크린샷 2021-06-29 오후 10 41 35" src="https://user-images.githubusercontent.com/70479118/123807841-2e631a80-d92b-11eb-8efc-fcc3d7c449d5.png">
----------------------------------------------
