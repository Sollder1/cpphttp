#include <unistd.h>
#include <cstdio>
#include <sys/socket.h>
#include <cstdlib>
#include <netinet/in.h>
#include <cstring>
#include <fcntl.h>
#include <cstdlib>

#define defaultBufferSize 4096
#define pathLength 1024
#define basePath "/var/cpphttp"

const char *baseHeaders = "\r\nserver: SollderHttp\r\nContent-Type: text/html\r\nConnection: Closed\r\n\r\n";

char *read(int fd);

char *concat(char *a, char *b) {

    char *concatenatedString = (char *) malloc(strlen(a) + strlen(b));

    strcat(concatenatedString, a);
    strcat(concatenatedString, b);

    return concatenatedString;
}

char *baseResponse(char *code) {

    char *result = (char *) malloc(strlen(baseHeaders) + 128);

    strcat(result, "HTTP/1.1 ");
    strcat(result, code);
    strcat(result, " Error");
    strcat(result, baseHeaders);

    return result;
}

char *getStaticNotFoundErrorResponse() {

    char *string = "<h1>404 - Resource not found</h1>"
                   "The requested Resource could not be found.";
    return concat(baseResponse("404"), string);
}


char *getStaticUnsupportedMethodErrorResponse() {

    char *string = "<h1>415 - Unsupported Method</h1>"
                   "The Method provided is not supported. We only support GET at the moment.";
    return concat(baseResponse("415"), string);
}

char *readFileContent(char *path) {
    int fd = open(concat(basePath, path), O_SYNC);

    if (fd < 0) {
        return getStaticNotFoundErrorResponse();
    }
    return concat(baseResponse("200"), read(fd));
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

    printf("1\n");
    char *path = (char *) malloc(pathLength);
    printf("2\n");
    size_t offset = strlen(method) + 1;
    printf("3\n");
    for (int i = 0; i < pathLength; ++i) {
        if (requestRaw[i + offset] == ' ') {
            memmove(path, requestRaw + offset, i);
            break;
        }
    }

    return readFileContent(path);
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
