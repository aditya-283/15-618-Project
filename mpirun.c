#include <cstdio>
#include <cstdlib>
#include <mpi.h>

int main(int argc, char* argv[]) {
    int opt = 0;
    char *executable = NULL;
    int numProc;
    do {
        opt = getopt(argc, argv, "exe:np:");
        switch (opt) {
        case "exe":
            executable = optarg;
            break;
        case "np":
            numProc = optarg;
            break;
        }
    } while (opt != -1);

    printf("MPIARGS %d %s\n", executable, numProc)
    for (int i=0; i<argc; i++) {
        printf("i=%i argv=%s \n", i, argv[i]);
    }
    // fork();
}