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
    long long int fileNum = getInt(clientSocket);
    printf("Files Requested: %lld\n", fileNum);
    fflush(stdout);
    char *filenames[fileNum];
    for (int i = 0; i < fileNum; i++) {
        filenames[i] = getString(clientSocket);
    }
    for (int i = 0; i < fileNum; i++) {
        if (filenames[i] != NULL) {
//            printf("%s\n", filenames[i]);
        } else
            printf("Filename %d is null\n", i);
    }
    printf(BLU "Received names of the files\n\n" RESET);
    fflush(stdout);
    struct stat fileInfo;
    long long int fileSize[fileNum];
    for (int i = 0; i < fileNum; i++) {
        if (lstat(filenames[i], &fileInfo) != -1 && (S_IFREG & fileInfo.st_mode)) {
            printf("%s exists on server\n", filenames[i]);
            sendInt(fileInfo.st_size, clientSocket);
            fileSize[i] = fileInfo.st_size;
        } else {
            printf(RED "%s does not exist on server\n" RESET, filenames[i]);
            sendInt(-1, clientSocket);
            fileSize[i] = -1;
        }
    }
    printf(BLU "\nReady to send the files to client\n" RESET);
    printf("\n");
    fflush(stdout);
    for (int i = 0; i < fileNum; i++) {
        if (fileSize[i] >= 0)
            sendFile(filenames[i], fileSize[i], clientSocket);
        fflush(stdout);
    }

    close(clientSocket);
    close(serverSocket);


    return 0;
}