#include "header.h"
#include "signal.h"

int t = 0;
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
    printf("\nWaiting for a new client.... Press CTRL-C to exit\n");
    *clientSocket = accept(serverSocket, NULL, NULL);
    if (*clientSocket <= 0) {
        perror("Server cannot get client");
        _exit(1);
    }
    printf(YEL "Client found \n" RESET);
    return serverSocket;
}

int main() {
    int serverSocket, clientSocket;
    while (1) {
        serverSocket = createServer(&clientSocket);
        /* get the number of files */
        long long int fileNum = getInt(clientSocket);
        printf("Files Requested: %lld\n", fileNum);
        fflush(stdout);

        /* get the filenames one by one */
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


        /*check if the file is valid and send the size accordingly */
        struct stat fileInfo;
        long long int fileSize[fileNum];
        for (int i = 0; i < fileNum; i++) {
            if (lstat(filenames[i], &fileInfo) != -1
                && (S_IFREG & fileInfo.st_mode)
                && access(filenames[i], R_OK) == 0) {
                printf("%s exists on server\n", filenames[i]);
                sendInt(fileInfo.st_size, clientSocket);
                fileSize[i] = fileInfo.st_size;
            } else {
                if (errno == ENOENT)
                    fprintf(stderr, RED "%s does not exist on server\n" RESET, filenames[i]);
                else if (errno == EACCES)
                    fprintf(stderr, RED "%s has access error\n" RESET, filenames[i]);
                else {
                    fprintf(stderr, "%s", filenames[i]);
                    perror(RED "Some error in opening file" RESET);
                }
                sendInt(-1, clientSocket);
                fileSize[i] = -1;
            }
        }

        /* send the files one by one */
        printf(BLU "\nReady to send the files to client\n" RESET);
        printf("\n");
        fflush(stdout);
        for (int i = 0; i < fileNum; i++) {
            if (fileSize[i] >= 0)
                sendFile(filenames[i], fileSize[i], clientSocket);
            fflush(stdout);
            free(filenames[i]);
        }

        close(clientSocket);
        close(serverSocket);
        if(t == 1){
            break;
        }
    }
}