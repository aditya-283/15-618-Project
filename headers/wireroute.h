#ifndef _WIRE_ROUTE_H
#define _WIRE_ROUTE_H

// Data structure for wire bend
typedef struct {
    int x;
    int y;
} bend_t;

// Data structure for wire
typedef struct { 
    int x1, y1;
    int x2, y2;
    bool hasBend;
    bend_t bend;
} wire_t;

typedef int cost_t;

// Perform computation, including reading/writing output files
void compute(int procID, int nproc, double prob, int numIterations);

// Read input file
void readInputFile(char *inputFilename);

// Write cost array file based on input filename
void writeCostFile(char *inputFilename);

// Write wire output file based on input filename
void writeOutputFile(char *inputFilename);

#endif // _WIRE_ROUTE_H
