// 1.The library #include s for all the necessary functions
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     //this contains the close() function to close the connections after the message is sent
#include <arpa/inet.h>  //this has all the inet_pton(), inet_ntop(), htons()... It has the converting IP functions
#include <sys/socket.h> //all the core socket functions like socket(), bind(), listen(), that kind of stuff
#include <netinet/in.h> //all the core socket address info stuff like struct sockaddr_in and constants defining type of sockets...
#include <pthread.h>    //includes pthread functions
#include <string.h>     //includes strcmp function
#include <stdbool.h>    //boolean include
#include <time.h>       //includes time functions

// define Port and maximum read size with constants
#define PORT 3490  // port communication occurs on
#define BACKLOG 10 // how many connections can be qued up: #define BACKLOG 10

void *spawner(void *arg); // function prototype

/*2.define the port that the server will be connected to on the local machine
    then set up address information and stuff in struct sockaddr
    will have to call getaddrinfo() some time to fill in the actual struct sockaddr we'll be connecting with
*/
int main(void)
{
    int sockfd;                              // socket descriptor for server
    int clientfd;                            // socket descriptor for each new connection that will be handled by a child process
    struct sockaddr_in servaddy, clientaddy; // servaddy holds addy parameters of server and clientaddy holds addy parameters for client that is connection

    memset(&servaddy, 0, sizeof(servaddy));
    servaddy.sin_family = AF_INET;
    servaddy.sin_port = htons(PORT);
    servaddy.sin_addr.s_addr = INADDR_ANY;

    // 3.create the socket with socket() and also error check using some perror() function look into that library's documentation
    sockfd = socket(PF_INET, SOCK_STREAM, 0); // will return -1 if unsuccessful
    if (sockfd == -1)
    {
        perror("Socket Creation Failure");
        exit(EXIT_FAILURE);
    }
    // 4.bind socket to proper port and also error check
    if (bind(sockfd, (struct sockaddr *)&servaddy, sizeof(servaddy)) == -1)
    { // will return -1 if unsuccssful
        perror("Binding Failure");
        exit(EXIT_FAILURE);
    }
    // 5.listen() for connections and err check
    if (listen(sockfd, BACKLOG) == -1)
    { // will return -1 if unsuccessful
        perror("Listening Failure");
        exit(EXIT_FAILURE);
    }
    pthread_t t[BACKLOG];
    int a = 0;
    while (1)
    { // infinite loop forever accepting connections and echoing message until program is terminated
        // 6.accept connection
        socklen_t clientaddysize = sizeof(clientaddy);
        clientfd = accept(sockfd, (struct sockaddr *)&clientaddy, &clientaddysize); // will be set to -1 if unsuccessful
        if (clientfd == -1)
        {
            perror("Accepting Failure");
            continue;
        }
        int *fdpointer = malloc(sizeof(int));
        if (fdpointer == NULL)
        {
            perror("Malloc failed");
            continue;
        }
        *fdpointer = clientfd;
        // hand connection off to a thread
        pthread_create(&t[a], NULL, spawner, fdpointer);
        pthread_detach(t[a]);
        a = (a + 1) % BACKLOG;
    }
}

void *spawner(void *arg)
{
    // 7.recv() message and read it
    // RECEIVING MESSAGES PART:

    char buffer_three[64000];
    int clientfd = *(int *)arg;
    free(arg);
    char buffer[64000]; // buffer that msg is copied on to just set to some value


    int total_recieved =0;
    int j;
    while (1){               //to make sure the full header was obtained
        j = recv(clientfd, buffer+total_recieved, sizeof(buffer) - 1 - total_recieved, 0);
        if (j < 0){
            perror("Message Recieving Failure");
            close(clientfd);
            return NULL;
        }
        total_recieved+=j;
        buffer[total_recieved] = '\0';

        char *body_start = strstr(buffer, "\r\n\r\n");
        if (body_start != NULL){
            body_start +=4;            
            break;
        }

    }
    char *loc = strstr(buffer, "Content-Length");
    if(!loc){
        perror("Missing Header");
        close(clientfd);
        return NULL;
    }
    int content_length = atoi(loc+15);
    if (!body_start) {
        perror("Error: No header-body delimiter found.\n");
        close(clientfd);
        return NULL;
    }

    int header_length = body_start - buffer;
    int body_bytes_recieved = total_recieved - header_length;
    int body_bytes_needed = content_length - body_bytes_recieved;
    while(body_bytes_needed > 0){       //trying to make sure full message was gotten
        j = recv(clientfd, buffer+total_recieved, sizeof(buffer) - 1 - total_recieved, 0); 
        if(j<0){
            perror("Recieving Message Failure");
            close(clientfd);
            return NULL;
        }
        total_recieved+=j;
        body_bytes_needed-=j;
    }
    buffer[total_recieved] = '\0';

    char method[10], path[1024], version[10];          // these two lines parce the buffer into
    sscanf(buffer, "%s %s %s", method, path, version); // multiple different pieces so its understandable
    // CHECKING IF THE PATH ENTERED IS SAFE
    if (strstr(path, "..") != NULL || strstr(path, "\\") != NULL || strstr(path, "<") != NULL || strstr(path, ">") != NULL || strstr(path, ";") != NULL)
    {
        close(clientfd);
        return NULL; // exits thread if any unsafe characters are found in received path
    }
    char fullpath[2000];
    snprintf(fullpath, sizeof(fullpath), "./www%s", path);
    FILE *fp;
    char buffer_two[64000]; // buffer that file conents will be read onto

    int status_code;
    char *status_message;
    char *content_type;
    bool binary = false;
    bool double_send = false;
    size_t bytes_read;

    // determining what type of file is being requested
    if (strstr(fullpath, ".html"))
        content_type = "text/html";
    else if (strstr(fullpath, ".css"))
        content_type = "text/css";
    else if (strstr(fullpath, ".js"))
        content_type = "application/javascript";
    else if (strstr(fullpath, ".png"))
    {
        content_type = "image/png";
        binary = true;
    }
    else if (strstr(fullpath, ".pdf"))
    {
        content_type = "application/pdf";
        binary = true;
    }
    else if (strstr(fullpath, ".mp4"))
    {
        content_type = "video/mp4";
        binary = true;
    }
    else
        content_type = "text/plain";
    //getting the TimeStamp
    time_t clock_reading = time(NULL);
    struct tm *local = localtime(&clock_reading);
    char TimeStamp[100];
    strftime(TimeStamp, sizeof(TimeStamp), "%Y-%m-%d %H:%M:%S",local);   //now the loacl time is saved to TimeStamp;


    if (!binary)
    {
        // IF GET:
        if (strcmp(method, "GET") == 0)
        {
            // get method and then access the path provided and return contents of file
            fp = fopen(fullpath, "r"); // open file from path that was sent over
            if (fp != NULL)
            { // making sure file exists

                bytes_read = fread(buffer_two, sizeof(char), sizeof(buffer_two) - 1, fp);
                buffer_two[bytes_read] = '\0';
                if (ferror(fp))
                {
                    status_code = 500;
                    status_message = "Internal Server Error";
                    content_type = "text/plain";
                }
                else
                {
                    status_code = 200;
                    status_message = "OK";
                }
                fclose(fp);
            }
            else
            {
                status_code = 400;
                status_message = "Not Found";
                content_type = "text/plain";
            }

            snprintf(buffer_three, sizeof(buffer_three), "HTTP/1.0 %d %s\r\nContent-type: %s\r\nContent-length: %zu\r\n\r\n%s", status_code, status_message, content_type, strlen(buffer_two), buffer_two);
        }

        // IF POST
        if (strcmp(method, "POST") == 0)
        {
            // post method and then access the path provided and post
            char *response_body;
            char *body = strstr(buffer, "\r\n\r\n");
            if (body != NULL)
            {
                body += 4;
                fp = fopen(fullpath, "w");
                if (fp != NULL)
                { // making sure file exists
                    fwrite(body, sizeof(char), strlen(body), fp);
                    if (ferror(fp))
                    {
                        status_code = 500;
                        status_message = "Internal Server Error";
                        response_body = "POST UNSUCCESSFUL";
                        content_type = "text/plain";
                    }
                    else
                    {
                        status_code = 200;
                        status_message = "OK";
                        response_body = "POST SUCCESSFUL";
                    }
                    fclose(fp);
                }
                else
                {
                    status_code = 400;
                    status_message = "Not Found";
                    response_body = "POST UNSUCCESSFUL";
                    content_type = "text/plain";
                }

                // 200 OK message
                snprintf(buffer_three, sizeof(buffer_three), "HTTP/1.0 %d %s\r\nContent-type: %s\r\nContent-length: %zu\r\n\r\n%s", status_code, status_message, content_type, strlen(response_body), response_body);
            }
            else
            {
                printf("No Content Provided");
                close(clientfd);
                return NULL;
            }
        }
    }
    
    if(binary){
        // IF GET:
        if (strcmp(method, "GET") == 0)
        {
            // get method and then access the path provided and return contents of file
            fp = fopen(fullpath, "rb"); // open file from path that was sent over
            if (fp != NULL)
            { // making sure file exists

                size_t bytes_read = fread(buffer_two, sizeof(char), sizeof(buffer_two) - 1, fp);
                if (ferror(fp))
                {
                    status_code = 500;
                    status_message = "Internal Server Error";
                    content_type = "text/plain";
                }
                else
                {
                    status_code = 200;
                    status_message = "OK";
                }
                fclose(fp);
            }
            else
            {
                status_code = 400;
                status_message = "Not Found";
                content_type = "text/plain";
            }

            snprintf(buffer_three, sizeof(buffer_three), "HTTP/1.0 %d %s\r\nContent-type: %s\r\nContent-length: %zu\r\n\r\n", status_code, status_message, content_type, bytes_read);
            double_send = true;
        }

        // IF POST
        if (strcmp(method, "POST") == 0)
        {
            // post method and then access the path provided and post
            char *response_body;
            char *body = strstr(buffer, "\r\n\r\n");
            if (body != NULL)
            {
                body += 4;
                fp = fopen(fullpath, "wb");
                if (fp != NULL)
                { // making sure file exists
                    int actual_size = total_recieved-(body-buffer);
                    fwrite(body, sizeof(char), actual_size, fp);
                    if (ferror(fp))
                    {
                        status_code = 500;
                        status_message = "Internal Server Error";
                        response_body = "POST UNSUCCESSFUL";
                        content_type = "text/plain";
                    }
                    else
                    {
                        status_code = 200;
                        status_message = "OK";
                        response_body = "POST SUCCESSFUL";
                    }
                    fclose(fp);
                }
                else
                {
                    status_code = 400;
                    status_message = "Not Found";
                    response_body = "POST UNSUCCESSFUL";
                    content_type = "text/plain";
                }
                
                // 200 OK message
                snprintf(buffer_three, sizeof(buffer_three), "HTTP/1.0 %d %s\r\nContent-type: %s\r\nContent-length: %zu\r\n\r\n%s", status_code, status_message, content_type, strlen(response_body), response_body);
            }
            else
            {
                printf("No Content Provided");
                close(clientfd);
                return NULL;
            }
        }
    }
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

    //Log file request onto server_log with format [TimeStamp] Method Path => status_code
    FILE *log;
    log = fopen("server_log.txt", "a");
    if(log == NULL){
        perror("Server Logging Error");
    }
    fprintf(log, "[%s] %s %s => %d\n", TimeStamp, method, fullpath, status_code);
    fclose(log);
    // 9.terminate connection with client
    close(clientfd);
    return NULL;
}
