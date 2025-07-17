# Multithreaded HTTP Server in C
This project is a multithreaded HTTP server programmed in C. It is capable of handling multiple clients at the same time and supports both GET and POST requests from a client. Furthermore it handles both text and binary files, includes basic security checks, and logs all requests.


## What is an HTTP Server?
HTTP stands for Hyper-Text-Transfer-Protocol, which is a data communication protocol that is widely used to define the communication between a client and a server. It defines in what manner requests are made and how responses are sent back. 

An HTTP server uses this as its underlying foundation to serve different types of files to a client which submits a request to it. It listens for requests, accepts them, processes them, and then responds with the appropriate response───whether that is a web page, text file, or image. We interact with HTTP Servers everyday when we make a request on the internet and are met with countless responses. An underlying protocol known as Transfer Control Protocol(TCP) is the set of rules HTTP is guided by to securely transmit data. TCP handles the actual data transmission while HTTP handles the message semantics and structure.

HTTP has a strict request and response message structure. A request message consists of the desired method, the resource being requested, HTTP protocol version, and some optional headers with extra information. A response message consists of the status code, status message, content-type, content-length, and a body with the contents of the file requested. HTTP is stateless meaning no information about any previous request is retained. This is often done by cookies or sessions on the internet.

## Components of the server
### Sockets
A socket is a communication endpoint that allows for communication between a client and server. It is usually defined as an IP address or port number and make up one side of a two way link needed for communication. The server's socket listens for incoming connections while the client's socket attempts to connect with the server's socket. After these two sockets are connected, data can be transmitted between them.

#### Socket Creation + Error Checking
```c

sockfd = socket(PF_INET, SOCK_STREAM, 0);
if (sockfd == -1)
{
  perror("Socket Creation Failure");
  exit(EXIT_FAILURE);
}
```

#### Binding the socket to the local port + Error Checking
```c
if (bind(sockfd, (struct sockaddr *)&servaddy, sizeof(servaddy)) == -1)  // will return -1 if unsuccssful
{ 
  perror("Binding Failure");
  exit(EXIT_FAILURE);
}
```

#### Listening for connection + Error Checking
```c
if (listen(sockfd, BACKLOG) == -1)  // will return -1 if unsuccessful
{ 
  perror("Listening Failure");
  exit(EXIT_FAILURE);
}
```
### Sending and Recieving Methodology
First, the server recieves a request from the client in the form of a text string. This text string is then parsed into the different components:
- method (GET or POST)
- path (eg. index.html, writing.txt, or img.jpg
- version (This server only handles version 1.0, but version 1.1 exists as well)

A "GET" request fetches the contents of the file that is requested through the path, while a "POST" request writes into the file that is requested through the path. After the action is carried out, the server sends a message back to the client. This message contains:
- status code
- status message
- content-type
- content-length
- contents itself

If the method requested is "GET", then the contents returned are what is contained in the file requested through the path. If the method requested is "POST", then the contents returned is a message: "POST SUCCESSFUL". If there is some issue with handling the request then the status code and status message are changed accordingly. For example, an internal server error sets the status code to 500 and the status message to "Internal Server Error". Also, the content returned will simply read "POST UNSUCCESSFUL".
## Multithreading
Creating multiple threads to handle each connection makes this server more efficient. The main server(main thread) constantly listens for incoming connections. When a connection is detected, it is accepted and then passed off to a newly created thread. This thread then handles the incoming request processing, sending the response back to the client, closing the connection, and terminating the thread. This allows the main thread to continue listening even while the request is being handled.
```c
*fdpointer = clientfd;     // hand connection off to a thread
pthread_create(&t[a], NULL, spawner, fdpointer);
pthread_detach(t[a]);
```
spawner() is the function that handles the HTTP requests.
## Other Features
### Partial Sends
Often times when data is sent across a connection, the entirity of the data isn't send. Thus, the name partial send. This is dealt with by checking the amount of bytes sent against buffer size and retrying send() until the full buffer is sent.
```c
int total = 0;  //total number of bytes we've sent
int bytes_left = strlen(buffer_three);
int n;
while(total<bytes_left){
  n = send(clientfd, buffer_three + total, bytes_left - total, 0);
  if (n == -1)
  {                                      
    perror("Sending Message Failure");
    return NULL;                       
  }
  total+=n; 
  }
if(double_send){
  int t_total = 0;  //total number of bytes we've sent
  int t_bytes_left = bytes_read;
  int m;
  while(t_total<t_bytes_left){
    m = send(clientfd, buffer_two + t_total, t_bytes_left - t_total, 0);
    if (m == -1)
    {                                      
      perror("Sending Message Failure");
      return NULL;                       
    }
    t_total+=m; 
  }
}
```
### Logging Requests
Every time a request is sent to the server, the timestamp, method, requested path, and response status code are logged on a file called server_log.txt
```c
//Log file request onto server_log with format [TimeStamp] Method Path => status_code
FILE *log;
log = fopen("server_log.txt", "a");
if(log == NULL){
  perror("Server Logging Error");
}
fprintf(log, "[%s] %s %s => %d\n", TimeStamp, method, fullpath, status_code);
fclose(log);
```
### Safety
Insecure path requests are blocked by the server. The server does this by checking the HTTP request that is sent for characters like "..", "<", "\\', and other unsafe characters.


