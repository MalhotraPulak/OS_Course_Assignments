#include "header.h"


int createServer(int *clientSocket) {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    int true = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(int)) != 0) {
        perror("setsockopt");
    }
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = INADDR_ANY;
    fflush(stdout);
    // bind the socket to the IP port
    if (bind(serverSocket, (struct sockaddr *) &address, sizeof(address)) != 0) {
        perror("Cannot bind the server to the IP");
        _exit(1);
    }

    if (listen(serverSocket, 3) != 0) {
        perror("Cannot listen on the socket");
        _exit(1);
    }

    *clientSocket = accept(serverSocket, NULL, NULL);
    if (clientSocket <= 0) {
        perror("Server cannot get client");
        _exit(1);
    }
    return serverSocket;
}


int main() {
    int serverSocket, clientSocket;
    serverSocket = createServer(&clientSocket);
    int fileNum = getInt(clientSocket);
    fprintf(stderr, "Got number %d\n", fileNum);
    fflush(stdout);
    char *filenames[fileNum];
    for (int i = 0; i < fileNum; i++) {
        filenames[i] = getString(clientSocket);
        sendInt(1, clientSocket);
    }
    fprintf(stderr, "Got names \n");
    for (int i = 0; i < fileNum; i++) {
        if (filenames[i] != NULL)
            printf("%s\n", filenames[i]);
        else
            printf("Filename %d is null", i);
    }


    close(clientSocket);
    close(serverSocket);

    return 0;
}