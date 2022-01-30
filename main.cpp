#include <unistd.h>
#include <cstdio>
#include <sys/socket.h>
#include <cstdlib>
#include <netinet/in.h>
#include <cstring>


#define defaultBufferSize 4096
#define pathLength 1024


char *concat(char *a, char *b) {

    char *concatenatedString = (char *) malloc(strlen(a) + strlen(b) - 1);

    strcat(concatenatedString, a);
    strcat(concatenatedString, b);

    return concatenatedString;
}

char *baseResponse() {
    return "HTTP/1.1 200 OK\r\n"
           "Server: SollderHttp\r\n"
           "Content-Type: text/html\r\n"
           "Connection: Closed\r\n"
           "\r\n";
}

char *getStaticUnsupportedMethodErrorResponse() {

    char *string = "<h1>415 - Unsupported Method</h1>"
                   "The Method provided is not supported. We only support GET at the moment.";
    return concat(baseResponse(), string);
}

char *handleRequest(const char *requestRaw) {

    //Read and validade http method:
    char *method = (char *) malloc(16);
    for (int i = 0; i < 16; ++i) {
        if (requestRaw[i] == ' ') {
            memcpy(method, requestRaw, i);
            break;
        }
    }
    if (strcmp(method, "GET") != 0) {
        return getStaticUnsupportedMethodErrorResponse();
    }
    printf("%s \n", method);


    char *path = (char *) malloc(pathLength);
    size_t offset = strlen(method) + 1;
    for (int i = 0; i < pathLength; ++i) {
        if (requestRaw[i + offset] == ' ') {
            memcpy(path, requestRaw + offset, i);
            break;
        }
    }

    printf("%s \n", path);

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
