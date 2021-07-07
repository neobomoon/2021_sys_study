Mission 2. fshare 1.0: Simple File Sharing System

Overview
This mission asks you to construct a pair of a server program and a client program for sharing files. This program makes a directory shared among multiple users via the internet. A user runs the client program to look up what kinds of files that a server has in a shared directory, and downloads a file from the shared directory, and uploads a file in the local directory to the shared directory. The server program receives requests from one or multiple client programs, and serves these requests by sending the list of contained files (for a list request), sending the file data (for a download request), and writing files with the transferred data (for a upload request).

  You need to use TCP for networking between the server and the client. The server program and the client program must use multithreading such that they can transfer multiple files concurrently.

Background study
•	Study the basics of the TCP/IP networking with a simple server-client example
o	https://youtu.be/ruxT8hy0M9I
o	https://github.com/hongshin/OperatingSystem/tree/sysprog/IPC
•	Q1. Change server.c and client.c to use multithreading instead of multiprocessing
•	Q2. Change server.c such that it regards the message received from a client as the name of a text file and transfers the text data of the file in the local directory back to the client.

Program design 


1.	Command-line interface

-  Server: fshared.c


•	As command-line arguments, the server program receives a port number (with option -p) and a directory to share with the client (with option -d)
•	Example
 
$ls
. 	..	files
$ls files
.	..	a.out		hello.txt		main.c
$fshared -p 8080 -d files &
$
 
- Client: fshare.c


•	The client program receives the IP address of a host machine with a port number (separated with :)and a user command as command-line arguments.
•	Example
 
$ls
.	..	hi.txt
$fshare 192.168.0.0.1:8080 list
a.out
hello.txt
main.c
$fshare 192.168.0.0.1:8080 get hello.txt
$ls
.	..	hello.txt	hi.txt
$fshare 192.168.0.0.1:8080 put hi.txt
$fshare 192.168.0.0.1:8080 list
a.out
hello.txt
hi.txt
main.c
$
 


2.	Functionality and Operation

The client program first sends a fixed-size header that indicates the type of a user command and the size of the payload data (i.e., the remaining part of the transmitted data). Following the header, the client transmits data according to the header if necessary, and then shutdowns the writing channel.

  Once the connection with a client is established, the server program reads the fixed amount of bytes to read the header. Based on the command type, the client reads the payload data and sends a response message. The response message may be the list of the contained files (for a list request), the content of a specific file (for a get request), nothing (for a put request), or an error message. A response message consists of a header and payload. Note that the header for a request (i.e., client sent) and the header for a response (i.e., the server sent) would have different structures.

Task Plan
1.	Start implementing fshared.c and fshare.c by constructing their command-line interface parts
•	Use Makefile to manage build process
2.	Design communication protocol (e.g., header structure, message data formats) for each request type
•	Although they are built by different persons, your implementations must be compatible with each other.
3.	Accomplish the list service in both client and server, and test your implementation to see they are compatible with each other.
4.	List up all cases that the server rejects a user request and sends an error message.
5.	Implement the remaining two services (i.e., get and put)



![image](https://user-images.githubusercontent.com/70479118/124770548-62ff5380-df75-11eb-86a2-9e2140aaba37.png)
