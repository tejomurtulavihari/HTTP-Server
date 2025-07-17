# Multithreaded HTTP Server in C
This project is a multithreaded HTTP server programmed in C. It is capable of handling multiple clients at the same time and supports both GET and POST requests from a client. Furthermore it handles both text and binary files, includes basic security checks, and logs all requests.


## What is an HTTP Server?
HTTP stands for Hyper-Text-Transfer-Protocol, which is a data communication protocol that is widely used to define the communication between a client and a server. It defines in what manner requests are made and how responses are sent back. 

An HTTP server uses this as its underlying foundation to serve different types of files to a client which submits a request to it. It listens for requests, accepts them, processes them, and then responds with the appropriate response───whether that is a web page, text file, or image. We interact with HTTP Servers everyday when we make a request on the internet and are met with countless responses. An underlying protocol known as Transfer Control Protocol(TCP) is the set of rules HTTP is guided by to securely transmit data. TCP handles the actual data transmission while HTTP handles the message semantics and structure.

HTTP has a strict request and response message structure. A request message consists of the desired method, the resource being requested, HTTP protocol version, and some optional headers with extra information. A response message consists of the status code, status message, content-type, content-length, and a body with the contents of the file requested. HTTP is stateless meaning no information about any previous request is retained. This is often done by cookies or sessions on the internet.

## Components of the server
### Sockets
A socket is a communication endpoint that allows for communication between a client and server. It is usually defined as an IP address or port number and make up one side of a two way link needed for communication. The server's socket listens for incoming connections while the client's socket attempts to connect with the server's socket. After these two sockets are connected, data can be transmitted between them.
### Sending and Recieving
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
### flkjasdf
lsjadflkjsa


