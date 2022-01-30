#include <unistd.h>
#include <cstdio>
#include <sys/socket.h>
#include <cstdlib>
#include <netinet/in.h>
#include <cstring>
#include <string>

#define defaultBufferSize 4096


char *handleRequest(char *requestRaw) {

    std::string message(requestRaw);

    //TODO...

    return "HTTP/1.1 200 OK\r\n"
           "Server: SollderHttp\r\n"
           "Content-Type: text/html\r\n"
           "Connection: Closed\r\n"
           "\r\n"
           "<h1>Hello World</h1>";
}


//Realloc could be more efficient, but for now it works
char *read(int fd) {

    size_t bufferSize = defaultBufferSize;
    size_t alreadyRead = 0;
    char *buffer = (char *) malloc(defaultBufferSize);

    while (true) {
        size_t actualBytesRead = read(fd, buffer + alreadyRead, defaultBufferSize);
        if (actualBytesRead == defaultBufferSize) {
            bufferSize += defaultBufferSize;
            buffer = (char *) realloc(buffer, bufferSize);
            alreadyRead += actualBytesRead;
            printf("Reaaloc\n");

        }
        if (actualBytesRead < defaultBufferSize) {
            break;
        }
    }

    return buffer;
}

void acceptConnection(int serverFd, sockaddr_in &address) {
    socklen_t clientAddressSize = sizeof(address);
    int incomingConnectionFd = accept(serverFd, (struct sockaddr *) &address, &clientAddressSize);
    if (incomingConnectionFd < 0) {
        perror("listen");
    }
    printf("Connection with client established... \n");

    //Read message
    char *message = read(incomingConnectionFd);
    printf("%s\n", message);

    char *response = handleRequest(message);

    send(incomingConnectionFd, response, strlen(response), 0);
    close(incomingConnectionFd);
}

[[noreturn]] void openTCPConnection(int port) {


    int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;

    if (serverFd <= 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);


    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    if (bind(serverFd, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(serverFd, 256) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (true) {
        acceptConnection(serverFd, address);
    }

}


int main() {
    openTCPConnection(8000);
}
