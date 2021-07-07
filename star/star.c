#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>

char src_path[PATH_MAX];
char dst_file[PATH_MAX];


typedef struct header{
    char type;
    unsigned int path_len;
    unsigned int data_size;
    char path_name[PATH_MAX];
}Header;


void
*path_cat(char *path, char *dir){
    char *new_path = (char *) malloc(sizeof(char) * (strlen(path) + strlen(dir) + 1));
    strcpy(new_path, path);
    strcat(new_path, "/");
    strcat(new_path, dir);
    return new_path;
}

void
header_set(char type, int path_len, int data_size, char *path_name, Header *header){
    header->type = type ;
    header->path_len = path_len ;
    header->data_size = data_size ;
    strcpy(header->path_name, path_name) ;
    return ;
}

void
header_write(char type, int path_len, int data_size, char *path_name, FILE *new_fp, Header *header){
    header_set(type, path_len, data_size, path_name, header) ; 

    if(fwrite(&header->type, 1, sizeof(header->type), new_fp) != sizeof(header->type)) 
        goto exit_fwrite ;
    if(fwrite(&header->path_len, 1, sizeof(header->path_len), new_fp) != sizeof(header->path_len))
        goto exit_fwrite ;
    if(fwrite(&header->data_size, 1, sizeof(int), new_fp) != sizeof(int))
        goto exit_fwrite ;

    if(fwrite(&header->path_name, 1, path_len, new_fp) != path_len)
        goto exit_fwrite ;
    printf("%s %d\n", header->path_name, path_len);

    // size_t len;
    // len = fwrite(&header->path_len, 4, 1, new_fp) ;
    // char buff[512];
    // fread(buff, 4, 1, new_fp) ;
    // printf("check %ld, %s, %ld\n", len, header->path_name, sizeof(header->path_len)) ;

    //free(header->path_name) ;
    return ;

exit_fwrite:
    perror("fail to fwrite on 'void header_write'");
    exit(1);
}

void
data_write(char type, char *path, FILE *new_fp){
    if(type = 'd')
        return ;

    FILE *fp = fopen(path, "rb") ; 
    if(fp == NULL)
        goto exit_fopen ;
    
    char buff[512] ;
    size_t size ;
    while(feof(fp) == 0){
        size = fread(buff, 1, sizeof(buff), fp) ; 
        if(fwrite(buff, 1, size, new_fp) != size)
            goto exit_fwrite ;
    }

    fclose(fp) ;
    return ;

exit_fopen:
    perror("fail to fopen on 'data_write'") ;
    exit(1) ;

exit_fwrite:
    perror("fail to fwrite on 'data_write'") ;
    exit(1) ;
}

void
archive_write(char *path, FILE *new_fp, Header *header, char type){
    struct stat s_file ;
    if(stat(path, &s_file) == -1)
        goto exit_stat ;
    
    header_write(type, (unsigned int) strlen(path), type == 'f' ? (unsigned int) s_file.st_size : 0, path, new_fp, header) ;
    data_write(type, path, new_fp) ; 

    return ;

exit_stat:
    perror("fail to stat on 'void archive_write'") ;
    exit(1) ;
}

void
archive(char *path, FILE *new_fp, Header *header){
    struct stat s_file ;
    if(stat(path, &s_file) == -1)
        goto exit_stat ; 
    
    if(S_ISREG(s_file.st_mode) == 1)
        archive_write(path, new_fp, header, 'f') ; 

    if(S_ISDIR(s_file.st_mode) == 1){
        printf("check\n");
        DIR *dp = opendir(path) ;
        if(dp == NULL)
            goto exit_opendir ;
        
        char *new_path ;
        struct dirent *dep ;
        for(; dep = readdir(dp); ){
            if(strcmp(dep->d_name, ".") == 0)
                continue ;
            if(strcmp(dep->d_name, "..") == 0)
                continue ;
            if(dep->d_type == DT_LNK)
                continue ;
            if(dep->d_type == DT_REG){
                new_path = path_cat(path, dep->d_name) ;
                archive_write(new_path, new_fp, header, 'f') ; // 데이터 적기
            }
            if(dep->d_type == DT_DIR){
                new_path = path_cat(path, dep->d_name) ;
                archive_write(new_path, new_fp, header, 'd') ;
                archive(new_path, new_fp, header) ;
            }
        } // for end
        closedir(dp) ;
        free(new_path) ;
    }

    return ;

exit_opendir:
    perror("fail to opendir on 'void archive'") ;
    exit(1) ;
exit_stat:
    perror("fail to stat on 'void archive'") ;
    exit(1) ;
}

///////////////////////list
void
list(char *file, FILE *new_fp){
    int path_len ;
    char path_name[PATH_MAX];

    fseek(new_fp, 1, SEEK_CUR) ;
    fread(&path_len, 1, sizeof(int), new_fp) ;
    fseek(new_fp, 4, SEEK_CUR) ;
    fread(path_name, 1, path_len, new_fp) ;
    path_name[path_len] = '\0' ;

    printf("%s\n", path_name);
    
    return ;
}


////////////////////////main
void
get_option(int argc, char **argv, char *opt){
    if(argc == 2){
        goto exit_err ;
    }

    if(strcmp(argv[1], "archive") == 0 && argc == 4){ // archive인 경우
        strcpy(dst_file, argv[2]) ;
        strcpy(src_path, argv[3]) ;

        if(access(src_path, F_OK) == -1)
            goto exit_err ;
        *opt = 'a' ;
        return ;
    }
    if(strcmp(argv[1], "list") == 0 && argc == 3){
        strcpy(dst_file, argv[2]) ;
        
        if(access(dst_file, F_OK) == -1)
            goto exit_err ;

        *opt = 'l' ;
        return ;
    }
    if(strcmp(argv[1], "extract") == 0 && argc == 3){
        strcpy(dst_file, argv[2]) ;

        if(access(dst_file, F_OK) == -1)
            goto exit_err ;

        *opt = 'e' ;
        return ;
    }

    goto exit_err ;

exit_err:
    perror("wrong option on 'void get_option'") ;
    exit(1) ;
}

int
main(int argc, char **argv){
    char opt ;
    get_option(argc, argv, &opt) ;

    FILE *new_fp = fopen(dst_file, opt == 'a' ? "wb" : "r") ;
    if(new_fp == NULL)
        goto exit_fopen ;

    if(opt == 'a'){
        Header *header ;
        archive(src_path, new_fp, header) ;
    }
    if(opt == 'l'){
        list(dst_file, new_fp) ;
    }
    if(opt == 'e'){
        //extract() ;
    }

    fclose(new_fp) ;
    return 0 ;

exit_fopen:
    perror("fail to open file on 'int main'") ;
    exit(1) ;
}
