#include <mpi.h>
#include <time.h>  
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "utils.h"

#define TICKS 10

int main(int argc, char *argv[]) {
    int i, mbi,
        rank, numTasks, tag, color, src, dest,
        workerRank, numWorkers, workersRoot,
        matrix[ROWS][COLUMNS], 
        matrixRecvBufferRows,
        topRowBuffer[COLUMNS], bottomRowBuffer[COLUMNS], 
        *topRow, *bottomRow,
        neighbours[COLUMNS] = { 0 };
    // master node isn't involved in the computation, hence a different Comm is needed
    MPI_Comm commWorkers;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numTasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    tag = 1;

    // master in CommWorld sends the matrix to the new '''master''' in commWorkers 
    if(rank == 0) {
        dest = 1;

        initMatrix(matrix, ROWS, COLUMNS);

        MPI_Send(matrix, ROWS * COLUMNS, MPI_INT, dest, tag, MPI_COMM_WORLD);
    } else if(rank == 1) {
        src = 0;

        MPI_Recv(matrix, ROWS * COLUMNS, MPI_INT, src, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    workerRank = rank - 1;
    numWorkers = numTasks - 1;

    // excess workers are ignored
    if(numWorkers > ROWS) {
        numWorkers = ROWS;
    }
    
    if (rank == 0) {
        color = 1;
    } else if (workerRank < numWorkers) {
        color = 0;
    } else {
        color = MPI_UNDEFINED;
    }

    MPI_Comm_split(MPI_COMM_WORLD, color, rank, &commWorkers);

    if (commWorkers == MPI_COMM_NULL) {
        MPI_Finalize();

        return 0;
    }
    
    matrixRecvBufferRows = getMatrixRecvBufferRows(numWorkers, ROWS);

    int matrixSendCounts[numWorkers], matrixDisplacements[numWorkers], matrixRecvBuffer[matrixRecvBufferRows][COLUMNS];
    MPI_Request requests[2][numWorkers];

    initRequests(numWorkers, requests[SEND]);
    initRequests(numWorkers, requests[RECEIVE]);
    initMatrixRecvBuffer(matrixRecvBuffer, matrixRecvBufferRows, COLUMNS, NIL);
    loadBalanceMatrix(numWorkers, ROWS, COLUMNS, matrixSendCounts, matrixDisplacements);

    // 0 in commWorkers is 1 in commWorld
    workersRoot = 0;

    // scatter matrix rows
    MPI_Scatterv(
            matrix, 
            matrixSendCounts, 
            matrixDisplacements,
            MPI_INT, 
            matrixRecvBuffer, 
            matrixRecvBufferRows * COLUMNS,
            MPI_INT,
            workersRoot, 
            commWorkers
        );

    if(rank == 0) {
        printMatrix(matrix, ROWS, COLUMNS);
    }

    for(i = 0; i < TICKS; i++) {
        if(rank != 0){
            for(mbi = 0; mbi < matrixRecvBufferRows; mbi++) {
                // load balancing with remainder implies that some rows might be empty
                if(matrixRecvBuffer[mbi][0] == NIL) {
                    break;
                }

                topRow = getTopRow(workerRank, numWorkers, mbi, matrixRecvBufferRows, matrixRecvBuffer, tag, 
                        commWorkers, requests, topRowBuffer);
                bottomRow = getBottomRow(workerRank, numWorkers, mbi, matrixRecvBufferRows, matrixRecvBuffer, tag,
                        commWorkers, requests, bottomRowBuffer);

                MPI_Waitall(numWorkers, requests[SEND], MPI_STATUSES_IGNORE);
                MPI_Waitall(numWorkers, requests[RECEIVE], MPI_STATUSES_IGNORE);

                getNeighbours(topRow, matrixRecvBuffer[mbi], bottomRow, COLUMNS, neighbours);
                updateMatrixRecvBuffer(matrixRecvBuffer, mbi, COLUMNS, neighbours);
            }
        }

        MPI_Gatherv(
                matrixRecvBuffer, 
                matrixSendCounts[workerRank],
                MPI_INT, 
                matrix, 
                matrixSendCounts, 
                matrixDisplacements,
                MPI_INT, 
                workersRoot, 
                commWorkers
            );

        if(rank == 1) {
            dest = 0;

            MPI_Send(matrix, ROWS * COLUMNS, MPI_INT, dest, tag, MPI_COMM_WORLD);
        }

        if(rank == 0) {
            src = 1; 

            MPI_Recv(matrix, ROWS * COLUMNS, MPI_INT, src, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printMatrix(matrix, ROWS, COLUMNS);

            sleep(1);
        }
    }

    MPI_Finalize();

    return 0;
}