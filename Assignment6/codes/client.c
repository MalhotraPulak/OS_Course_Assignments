#include <stdbool.h>
#include "header.h"


int createClient() {
    // create the socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    // configure the address struct
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = INADDR_ANY;

    printf("Waiting for a server to connect\n");
    if (connect(clientSocket, (struct sockaddr *) &address, sizeof(address)) != 0) {
        fprintf(stderr, "Cannot connect to a server \n");
        _exit(1);
    }
    printf(YEL "Connected to the server\n" RESET);
    return clientSocket;
}

int clientStuff(int argc, char *argv[], int clientSocket) {
    if (argc < 2) {
        fprintf(stderr, "at least one argument required\n");
        _exit(1);
    }
    int fileNum = argc - 1;
    char **filename = argv + 1;
    /* send number of files */
    if (sendInt(argc - 1, clientSocket) != 1) {
        perror("No ack");
    }
    /* send each file name one by one */
    for (int i = 1; i < argc; i++) {
        sendString(argv[i], clientSocket);
    }
    /* get file size and validity of the file */
    long long int fileSize[argc - 1];
    for (int i = 0; i < argc - 1; i++) {
        fileSize[i] = getInt(clientSocket);
        if (fileSize[i] == -1) {
            printf(RED "%s does not exist on server\n" RESET, argv[i + 1]);
        } else {
            //printf("%s has size %lld\n", filename[i], fileSize[i]);
        }
    }
    /* receive the valid files one by one */
    printf(BLU "\nReady to receive files \n\n" RESET);
    for (int i = 0; i < fileNum; i++) {
        if (fileSize[i] >= 0) {
            getFile(argv[i + 1], fileSize[i], clientSocket);
            printf("\n");
        }
    }
}

char *trim_whitespace(char *line) {
    int t = 0;
    for (int i = 0; i < strlen(line); i++) {
        if (line[i] == ' ' || line[i] == '\t' || line[i] == '\n') {
            t++;
        } else {
            break;
        }
    }
    for (int i = 0; i < t; i++) {
        line++;
    }
    for (int i = strlen(line) - 1; i >= 0; i--) {
        if (line[i] == ' ' || line[i] == '\t' || line[i] == '\n') {
            line[i] = '\0';
        } else {
            break;
        }
    }
    return line;
}

int main() {
    int clientSocket = createClient();
    while (true) {
        printf(BLU"client>"RESET);
        char *command = NULL;
        size_t read_size = 0;
        if (getline(&command, &read_size, stdin) == -1) {
            if (feof(stdin)) {
                close(clientSocket);

                break;
            }
        }
        trim_whitespace(command);
        char *args[CHUNK];
        char *token = strtok(command, " \t\n");
        args[0] = token;
        int i = 1;
        if (strcmp(args[0], "get") == 0) {
            while (token != NULL) {
                token = strtok(NULL, " \t\n");
                args[i++] = token;
            }
            clientStuff(i - 1, args, clientSocket);
        } else if(strcmp(args[0], "exit") == 0){
            close(clientSocket);
            printf(RED "\nConnection closed\n"RESET);
            return 0;
        }
    }
    return 0;
}