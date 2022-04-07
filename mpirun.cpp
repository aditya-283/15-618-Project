#include <cstdio>
#include <cstdlib>
#include "mpi.h"
#include <unistd.h>
// #include <cstring>


int main(int argc, char* argv[]) {
    int opt = 0;
    char *executable = NULL;
    char rankBuffer[10];
    int numProc=0;

    do {
        opt = getopt(argc, argv, "e:n:");
        switch (opt) {
        case 'e':
            executable = optarg;
            break;
        case 'n':
            numProc = atoi(optarg);
            break;
        }
    } while (opt != -1);

    for (int i=0; i<numProc; i++) {
        int pid = fork();
        if (pid == 0) {
            //parent
            snprintf(rankBuffer, sizeof(rankBuffer), "%d", i);
            argv[3] = rankBuffer;
            execve(executable, argv+2, NULL);
            break;
        } else {
            //child
            if (i == numProc - 2) {
                snprintf(rankBuffer, sizeof(rankBuffer), "%d", i + 1);
                argv[3] = rankBuffer;;
                execve(executable, argv+2, NULL);
                break;
            }   
        }
    }
}