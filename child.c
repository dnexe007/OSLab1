#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>

int main(int argc, char* argv[]){
    if(argc < 2){
        fprintf(stderr, "Cild error: missing argument\n");
        return 1;
    }

    char buffer[1024];    
    int childToParent1 = atoi(argv[1]);

    while(fgets(buffer, sizeof(buffer), stdin) != NULL){
        
        size_t length = strlen(buffer);

        if(isupper(buffer[0])){
            fprintf(stdout, "%s", buffer);
            dprintf(childToParent1, "Ok\n");
        }

        else{
            if(buffer[0] == '\0')
                dprintf(childToParent1, "Error: input is empty\n");
            else
                dprintf(childToParent1, "Error: input starts with '%c' - not uppercase\n", buffer[0]);
        }
    }
    close(childToParent1);
    return 0;
}
