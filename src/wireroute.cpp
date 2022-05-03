#include "headers/wireroute.h"
#include "headers/mpi.h"
#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <libgen.h>
#include <sys/stat.h>
#include <limits.h>
#include <vector>
#include <algorithm>
#include <math.h>

/* -------------- PRE-PROCESSOR DIRECTIVES -------------- */
#define ROOT_PROC 0
#define COMM_DATA_LEN 3

/* ------------------ GLOBAL VARIABLES ------------------ */
wire_t *wires;
cost_t *costs;
int procID, numProcs;
int numWires, dimX, dimY;

/* --------------------- FUNCTIONS ---------------------- */

/**
 * @brief Compute the number of wires to send depending on the number of processes in the system and the number of wires
 *
 * @param numWires Number of wires
 * @param numProc Number of processes
 */
int inline getNumSend()
{
    return (int)((150.0f * (60000.0f / numWires)) / (int)(fmax(cbrt(numProcs * numProcs / 6), 1)));
}

/**
 * @brief Read the wire data from the input file and also
 *        allocate memory for wires array and cost matrix
 *
 * @param inputFilename Input File Name
 */
void readInputFile(char *inputFilename)
{
    FILE *input = fopen(inputFilename, "r");
    if (!input)
    {
        printf("Unable to open file: %s.\n", inputFilename);
        return;
    }
    fscanf(input, "%d %d\n", &dimY, &dimX);
    fscanf(input, "%d\n", &numWires);

    wires = (wire_t *)calloc(numWires, sizeof(wire_t));
    costs = (cost_t *)calloc(dimX * dimY, sizeof(cost_t));

    for (int i = 0; i < numWires; i++)
    {
        fscanf(input, "%d %d %d %d\n", &wires[i].x1, &wires[i].y1, &wires[i].x2, &wires[i].y2);
        wires[i].hasBend = 0;
    }
}

/**
 * @brief Write the cost matrix to cost file
 *
 * @param inputFilename Input File Name
 */
void writeCostFile(char *inputFilename)
{
    char costFileName[100], fileName[100];
    char *token = NULL, *nextToken = NULL;

    mkdir("./WireRoute", 0777);
    mkdir("./WireRoute/costs", 0777);

    strcpy(fileName, inputFilename);
    nextToken = token = strtok(fileName, "/");
    while ((nextToken = strtok(NULL, "/")))
    {
        token = nextToken;
    }
    token = strtok(token, ".");
    sprintf(costFileName, "./WireRoute/costs/costs_%s_%d.txt", token, numProcs);
    FILE *cost_file = fopen(costFileName, "w");

    // Write to file
    fprintf(cost_file, "%d %d\n", dimY, dimX);
    for (int i = 0; i < dimY; i++)
    {
        for (int j = 0; j < dimX; j++)
        {
            fprintf(cost_file, "%d ", costs[i * dimX + j]);
        }
        fprintf(cost_file, "\n");
    }
    fclose(cost_file);
}

/**
 * @brief Write the segments of wire to output file
 *
 * @param output_file Output file descriptor
 * @param x1 Wire Segment start x-coordinate
 * @param y1 Wire Segment start x-coordinate
 * @param x2 Wire Segment end x-coordinate
 * @param y2 Wire Segment end y-coordinate
 */
void writeWireSegment(FILE *output_file, int x1, int y1, int x2, int y2)
{
    if (x1 != x2)
    {
        // Along x-direction
        int updateX = (x1 < x2) ? 1 : -1;
        for (int i = x1; i != x2; i += updateX)
        {
            fprintf(output_file, "%d %d ", i, y1);
        }
    }
    else
    {
        // Along y-direction
        int updateY = (y1 < y2) ? 1 : -1;
        for (int i = y1; i != y2; i += updateY)
        {
            fprintf(output_file, "%d %d ", x1, i);
        }
    }
}

/**
 * @brief Write the outputs (dimensions, number of wires and the wires' paths) to output file
 *
 * @param inputFilename Input file name
 */
void writeOutputFile(char *inputFilename)
{
    char ouputFileName[100], fileName[100];
    char *token = NULL, *nextToken = NULL;

    mkdir("./WireRoute", 0777);
    mkdir("./WireRoute/outputs", 0777);

    strcpy(fileName, inputFilename);
    nextToken = token = strtok(fileName, "/");
    while ((nextToken = strtok(NULL, "/")))
    {
        token = nextToken;
    }
    token = strtok(token, ".");
    sprintf(ouputFileName, "./WireRoute/outputs/output_%s_%d.txt", token, numProcs);

    FILE *output_file = fopen(ouputFileName, "w");
    fprintf(output_file, "%d %d\n", dimY, dimX);
    fprintf(output_file, "%d\n", numWires);
    for (int i = 0; i < numWires; i++)
    {
        wire_t wire = wires[i];
        if (!wire.hasBend)
        {
            // writeWireSegment(output_file, wire.x1, wire.y1, wire.x2, wire.y2);
            fprintf(output_file, "%d %d ", wire.x1, wire.y1);
        }
        else
        {
            bend_t secondBend = (wire.bend.x == wire.x1) ? (bend_t){wire.x2, wire.bend.y} : (bend_t){wire.bend.x, wire.y2};
            // writeWireSegment(output_file, wire.x1, wire.y1, wire.bend.x, wire.bend.y);
            // writeWireSegment(output_file, wire.bend.x, wire.bend.y, secondBend.x, secondBend.y);
            // writeWireSegment(output_file, secondBend.x, secondBend.y, wire.x2, wire.y2);
            fprintf(output_file, "%d %d ", wire.x1, wire.y1);
            fprintf(output_file, "%d %d ", wire.bend.x, wire.bend.y);
            fprintf(output_file, "%d %d ", secondBend.x, secondBend.y);
        }
        fprintf(output_file, "%d %d \n", wires[i].x2, wires[i].y2);
    }

    fclose(output_file);
}

/**
 * @brief Binary function to compare the starting y coordinates of 2 wires
 *
 * @param wire1
 * @param wire2
 */
struct wireLessThan
{
    inline bool operator()(const wire_t &wire1, const wire_t &wire2)
    {
        int minY1 = (wire1.y1 < wire1.y1) ? wire1.y1 : wire1.y2;
        int minY2 = (wire2.y1 < wire2.y1) ? wire2.y1 : wire2.y2;
        return (minY1 < minY2);
        // return (wire1.y1 < wire2.y1);
    }
};

/**
 * @brief Sort the wires in ascending order
 */
void sortWires()
{
    std::vector<wire_t> v_wires(wires, wires + numWires);
    std::sort(v_wires.begin(), v_wires.end(), wireLessThan());
    for (int i = 0; i < numWires; i++)
    {
        wires[i] = v_wires[i];
    }
}

/**
 * @brief Populates the cost matrix for a wire with an initial placement strategy (1-bend only)
 *
 * @param wire Wire for which the initial placement and cost update is made
 */
void initWireCosts(wire_t wire)
{
    int update_y = (wire.y1 < wire.y2) ? 1 : -1;
    int update_x = (wire.x1 < wire.x2) ? 1 : -1;
    for (int r = wire.y1; r != wire.bend.y; r += update_y)
    {
        costs[r * dimX + wire.bend.x] += 1;
    }

    for (int c = wire.bend.x; c != wire.x2; c += update_x)
    {
        costs[wire.bend.y * dimX + c] += 1;
    }

    for (int r = wire.bend.y; r != wire.y2 + update_y; r += update_y)
    {
        costs[r * dimX + wire.x2] += 1;
    }
}

/**
 * @brief Performs the initial wire placement for all the wires and updates the cost matrix.
 * Uses 1-bend for initial placement
 */
void initWireRoute()
{
    for (int i = 0; i < numWires; i++)
    {
        wire_t wire = wires[i];
        if (wire.x1 == wire.x2 || wire.y1 == wire.y2)
        {
            // Straight wire
            wire.bend = (bend_t){wire.x1, wire.y1};
            wire.hasBend = false;
        }
        else
        {
            wire.bend = (bend_t){wire.x1, wire.y2};
            wire.hasBend = true;
        }
        initWireCosts(wire);
        wires[i] = wire;
    }
}

/**
 * @brief Update the cost matrix for a particular wire based on costUpdate
 *
 * @param wire wire that needs to be updated
 * @param costUpdate The cost update parameter: +1 or -1
 */
void updateWireInCost(wire_t wire, int costUpdate)
{
    int updateY = (wire.y1 < wire.y2) ? 1 : -1;
    int updateX = (wire.x1 < wire.x2) ? 1 : -1;
    if (wire.x1 == wire.bend.x)
    { // Vertical Wires
        for (int r = wire.y1; r != wire.bend.y; r += updateY)
        {
            costs[r * dimX + wire.bend.x] += costUpdate;
        }

        for (int c = wire.bend.x; c != wire.x2; c += updateX)
        {
            costs[wire.bend.y * dimX + c] += costUpdate;
        }

        for (int r = wire.bend.y; r != wire.y2 + updateY; r += updateY)
        {
            costs[r * dimX + wire.x2] += costUpdate;
        }
    }
    else
    { // Horizontal Wires
        for (int c = wire.x1; c != wire.bend.x; c += updateX)
        {
            costs[wire.bend.y * dimX + c] += costUpdate;
        }

        for (int r = wire.bend.y; r != wire.y2; r += updateY)
        {
            costs[r * dimX + wire.bend.x] += costUpdate;
        }

        for (int c = wire.bend.x; c != wire.x2 + updateX; c += updateX)
        {
            costs[wire.y2 * dimX + c] += costUpdate;
        }
    }
}

/**
 * @brief Return the maximum of 2 costs
 *
 * @param a
 * @param b
 * @return cost_t
 */
cost_t max(cost_t a, cost_t b)
{
    return (a > b) ? a : b;
}

/**
 * @brief Find the best bend for a given wire with the least cost.
 *
 * @param wire array of all the wires
 * @param prob probability for choosing the random wire route
 * @return best bend for the wire
 */
bend_t findBestBend(wire_t &wire, double prob)
{
    int minCost = INT_MAX;
    int minOverlap = INT_MAX;
    int updateY = (wire.y1 < wire.y2) ? 1 : -1;
    int updateX = (wire.x1 < wire.x2) ? 1 : -1;

    double randf = (float)rand() / (float)INT_MAX;
    int yCandidateNum = (wire.y2 - wire.y1 + 1);
    int xCandidateNum = (wire.x2 - wire.x1 + 1);
    float yFraction = prob * yCandidateNum / (yCandidateNum + xCandidateNum);

    bend_t bestCandidate;
    // Random selection based on probability
    if (randf < yFraction)
    { // pick in y
        int randY = wire.y1 + updateY * rand() % (abs(wire.y2 - wire.y1) + 1);
        bestCandidate = (bend_t){wire.x1, randY};
        return bestCandidate;
    }
    else if (randf < prob)
    { // pick in x
        int randX = wire.x1 + updateX * rand() % (abs(wire.x2 - wire.x1) + 1);
        bestCandidate = (bend_t){randX, wire.y1};
        return bestCandidate;
    }

    // Iterate through all the possible wire routes and pick the best
    bend_t candidate = (bend_t){wire.x1, wire.y1};
    // All Vertical wires candidates
    for (candidate.y = wire.y1; candidate.y != wire.y2 + updateY; candidate.y += updateY)
    {
        int curCost = 0;
        int curOverlap = 0;
        int val;
        for (int r = wire.y1; r != candidate.y + updateY; r += updateY)
        {
            val = costs[r * dimX + candidate.x] + 1;
            curCost += val * val;
            curOverlap = max(val, curOverlap);
        }

        for (int c = candidate.x; c != wire.x2 + updateX; c += updateX)
        {
            val = costs[candidate.y * dimX + c] + 1;
            curCost += val * val;
            curOverlap = max(val, curOverlap);
        }

        for (int r = candidate.y; r != wire.y2 + updateY; r += updateY)
        {
            val = costs[r * dimX + wire.x2] + 1;
            curCost += val * val;
            curOverlap = max(val, curOverlap);
        }

        val = costs[candidate.y * dimX + (wire.x1)];
        curCost -= val * val;
        if (candidate.y != wire.y1 && candidate.y != wire.y2)
        {
            val = costs[candidate.y * dimX + wire.x2];
            curCost -= val * val;
        }

        if (curOverlap < minOverlap)
        {
            minOverlap = curOverlap;
            bestCandidate = candidate;
        }
        else if (curOverlap == minOverlap && curCost < minCost)
        {
            bestCandidate = candidate;
            minCost = curCost;
        }
    }

    candidate = (bend_t){wire.x1, wire.y1};
    // All Horizontal wires candidates
    for (candidate.x = wire.x1; candidate.x != wire.x2 + updateX; candidate.x += updateX)
    {
        int curCost = 0;
        int curOverlap = 0;
        int val;
        for (int c = wire.x1; c != candidate.x + updateX; c += updateX)
        {
            val = costs[candidate.y * dimX + c] + 1;
            curCost += val * val;
            curOverlap = max(val, curOverlap);
        }

        for (int r = candidate.y; r != wire.y2 + updateY; r += updateY)
        {
            val = costs[r * dimX + candidate.x] + 1;
            curCost += val * val;
            curOverlap = max(val, curOverlap);
        }

        for (int c = candidate.x; c != wire.x2 + updateX; c += updateX)
        {
            val = costs[wire.y2 * dimX + c] + 1;
            curCost += val * val;
            curOverlap = max(val, curOverlap);
        }

        val = costs[wire.y1 * dimX + candidate.x];
        curCost -= val * val;
        if (candidate.x != wire.x1 && candidate.x != wire.x2)
        {
            val = costs[wire.y2 * dimX + candidate.x];
            curCost -= val * val;
        }

        if (curOverlap < minOverlap)
        {
            minOverlap = curOverlap;
            bestCandidate = candidate;
        }
        else if (curOverlap == minOverlap && curCost < minCost)
        {
            bestCandidate = candidate;
            minCost = curCost;
        }
    }
    return bestCandidate;
}

/**
 * @brief Communicates the updated wire info with all other processes.
 *        The root gathers all the data from other processes and then
 *        broadcasts the data to all processes.
 *
 * @param sendMsg Data buffer to be sent
 * @param chunkSize Number of Wires' data present in the message buffer
 */
void communicate(int *sendMsg, int bufSize)
{
    int *recvMsg = (int *)malloc(sizeof(int) * (bufSize * numProcs));
    MPI_Allgather(sendMsg, bufSize, MPI_INT, recvMsg, bufSize, MPI_INT, MPI_COMM_WORLD);

    for (int j = 0; j < numProcs; j += 1)
    {
        if (j == procID)
            continue;

        for (int k = 0; k < bufSize; k += COMM_DATA_LEN)
        {
            int wireIndex = recvMsg[bufSize * j + k]; // (bufSize * j + k)/COMM_DATA_LEN;
            if (wireIndex > numWires)
                continue;
            if (!wires[wireIndex].hasBend)
                continue;
            updateWireInCost(wires[wireIndex], -1);
            wires[wireIndex].bend.x = recvMsg[bufSize * j + k + 1];
            wires[wireIndex].bend.y = recvMsg[bufSize * j + k + 2];
            updateWireInCost(wires[wireIndex], 1);
        }
    }
}

/**
 * @brief Re-routes the wires based on the initial placement to minimize the costs
 *
 * @param numIters Number of Simulated Annealing Iterations
 * @param prob Probability for Simulated Annealing random selection
 */
void routeWires(int numIters, double prob)
{
    int chunkSize = (numWires + numProcs - 1) / numProcs;
    int startIndex = chunkSize * procID;
    int endIndex = startIndex + chunkSize;
    int *sendMsg = (int *)malloc(sizeof(int) * COMM_DATA_LEN * chunkSize);

    for (int iter = 0; iter < numIters; iter++)
    {
        int cnt = 0; // Counter to keep track of communication
        for (int i = startIndex; i < endIndex; i++)
        {
            wire_t wire;
            if (i < numWires)
            { // Optimize if the index is valid
                wire = wires[i];
                if (wire.hasBend)
                {
                    updateWireInCost(wire, -1);
                    wire.bend = findBestBend(wire, prob);
                    wire.hasBend = true;
                    wires[i] = wire;
                    updateWireInCost(wire, 1);
                }
            }
            else
            { // Clear any junk values
                wire.bend = (bend_t){0, 0};
            }

            sendMsg[COMM_DATA_LEN * cnt] = i;
            sendMsg[COMM_DATA_LEN * cnt + 1] = wire.bend.x;
            sendMsg[COMM_DATA_LEN * cnt + 2] = wire.bend.y;

            int numSend = getNumSend();
            if (cnt == numSend)
            {
                communicate(sendMsg, COMM_DATA_LEN * numSend);
                cnt = 0;
            }
            cnt++;
        }
        communicate(sendMsg, COMM_DATA_LEN * cnt);
    }
}

// Perform computation, including reading/writing output files
/**
 * @brief Perform Wire Optimization
 *
 * @param processID ID of the current process
 * @param nProcs number of processes running
 * @param prob Probability for Simulated Annealing random selection
 * @param numIterations Number of Simulated Annealing Iterations
 */
void compute(int processID, int nProcs, double prob, int numIterations)
{
    /* Conduct initial wire placement */
    procID = processID;
    numProcs = nProcs;
    sortWires();
    initWireRoute();
    routeWires(numIterations, prob);
}


int main(int argc, char* argv[]) {
    int nProc=0, procId=0;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &procId);
    MPI_Comm_size(MPI_COMM_WORLD, &nProc);

    char fileName[100] = "wireInputs/easy_1024.txt";
    readInputFile(fileName);
    double initStart = MPI_Wtime();
    compute(procId, nProc, 0.1, 5);
    if (procID == 0) {
        printf("Computation took %lf seconds\n", MPI_Wtime() - initStart);
        writeCostFile(fileName);
        writeOutputFile(fileName);
    }

    MPI_Finalize();
    return 0;
}