#include <stdio.h>
#include <stdlib.h>
#include <chrono>

#define ARRAY_SIZE      1000000

int main(int argc, char* argv[]) {
    using namespace std::chrono;
    typedef std::chrono::high_resolution_clock Clock;
    typedef std::chrono::duration<double> dsec;
    auto initStart = Clock::now();

    int *intArray = (int *) malloc(sizeof(int) * ARRAY_SIZE);

    for (int i = 0; i < ARRAY_SIZE; i ++) {
        intArray[i] = rand() % 100;
    }

    long int sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i ++) {
        sum += intArray[i];
    }    
    double finalAvg = (double) sum / ARRAY_SIZE;
    auto compComplete = Clock::now();
    printf("Final Average is %lf\n", finalAvg);
    printf("Computation took %lf seconds\n", duration_cast<dsec>(compComplete - initStart).count());

    return 0;
}
