# fshare 1.0
### command line
fshared : $./fshared -p 8080 -d {dir_name}

fshare : $./fshare 127.0.0.1:8080 list
         $./fshare 127.0.0.1:8080 get {file_name}
         $./fshare 127.0.0.1:8080 put {file_name}
         
         
### description
"list"으로 fshared에서 지정한 디렉토리 안에 있는 파일들을 볼 수 있다.

"get"으로 fshared에서 지정한 디렉토리 안에 있는 파일을 가져올 수 있다.

"put"으로 fshare에 있는 파일을 fshared에 업로드 할 수 있다.
