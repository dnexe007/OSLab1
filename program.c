#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

void CreatePipe(int newPipe[2]){
    if(pipe(newPipe) == -1){
        perror("Pipe creating error");
        exit(1);
    }
}

int OpenFile(){
    char outpufFile[256];

    printf("Enter output file name: ");

    if(fgets(outpufFile, sizeof(outpufFile), stdin) != NULL){
        size_t length = strlen(outpufFile);
    
        if (length > 0 && outpufFile[length - 1] == '\n') {
            outpufFile[length - 1] = '\0';
        }
    }
    else if(ferror(stdin)){
        perror("File name reading error");
    }

    if(outpufFile[0] == '\0'){
        fprintf(stderr, "Error: file name is empty\n");
        exit(1);
    }

    int outputFileId = open(
        outpufFile,
        O_WRONLY | O_CREAT | O_TRUNC,
        S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
    );
    if(outputFileId == -1){
        perror("File opening error");
        exit(1);
    }
    return outputFileId;
}


void DuplicateTo(int oldFd, int newFd){
    if(dup2(oldFd, newFd) == -1){
        perror("dup2 error");
        exit(1);
    }
}

int main() {

    int outputFileId = OpenFile();

    int parentToChild[2];
    CreatePipe(parentToChild);


    int childToParent[2];
    CreatePipe(childToParent);

    pid_t processId = fork();

    if(processId == -1) {
        perror("Fork error");
        return 1;
    }

    if(processId == 0) { //child process
        DuplicateTo(parentToChild[0], STDIN_FILENO);
        DuplicateTo(outputFileId, STDOUT_FILENO);


        close(parentToChild[0]);
        close(parentToChild[1]);

        close(childToParent[0]);
        close(outputFileId);

        char fdString[16];
        sprintf(fdString, "%d", childToParent[1]);


        execl("./child", "child", fdString, (char*)NULL);

        perror("Child execution error");
        exit(1);
    }


    close(parentToChild[0]);
    close(childToParent[1]);
    close(outputFileId);

    char inputBuffer[1024];
    char outputBuffer[1024];

    ssize_t bytesRead;

    while(true){
        printf("Enter text: ");
        if(fgets(inputBuffer, sizeof(inputBuffer), stdin) == NULL) break;

        if(strcmp(inputBuffer, "/esc\n") == 0) break;

        write(parentToChild[1], inputBuffer, strlen(inputBuffer));

        bytesRead = read(childToParent[0], outputBuffer, sizeof(outputBuffer) - 1);
        if(bytesRead != 0){
            outputBuffer[bytesRead] = '\0';
            if(strcmp(outputBuffer, "Ok\n") != 0)
                printf("%s", outputBuffer);
        }
    }


    close(parentToChild[1]);
    close(childToParent[0]);

    wait(NULL);
    return 0;
}