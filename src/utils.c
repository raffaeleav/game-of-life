#include <mpi.h>
#include <time.h>  
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "utils.h"

void initMatrix(int matrix[][COLUMNS], int rows, int columns) {
    int i, j; 

    srand(time(NULL));

    for(i = 0; i < ROWS; i++) {
        for(j = 0; j < COLUMNS; j++) {
            matrix[i][j] = rand() % 2;
        }
    }
}

void printMatrix(int matrix[][COLUMNS], int rows, int columns) {
    int i, j; 

    printf("\033[2J\033[H");

    printf("+--");

    for(i = 0; i < columns - 1; i++) {
        printf("--");
    }

    printf("-+\n");

    for(i = 0; i < rows; i++) {
        printf("| ");

        for(j = 0; j < columns; j++) {
            if(matrix[i][j] == 1) {
                printf("* "); 
            } else {
                printf("  ");
            }
        }

        printf("|\n");
    }

    printf("+");

    for(i = 0; i < columns - 1; i++) {
        printf("--");
    }

    printf("---+\n");
}

// load balance with remainder
void loadBalanceMatrix(int numWorkers, int rows, int columns, int *sendCounts, int *displacements) {
    int i, baseRows, extraRows, offset, rowsPerWorker;
    
    offset = 0;
    baseRows = rows / numWorkers;
    extraRows = rows % numWorkers;

    for (i = 0; i < numWorkers; i++) {
        rowsPerWorker = baseRows + (i < extraRows ? 1 : 0);

        sendCounts[i] = rowsPerWorker * columns;
        displacements[i] = offset;
        offset += sendCounts[i];
    }
}

int getMatrixRecvBufferRows(int numWorkers, int rows) {
    int rowsPerWorker, extraRows;

    rowsPerWorker = rows / numWorkers;
    extraRows = rows % numWorkers;

    return rowsPerWorker + (extraRows > 0 ? 1 : 0);
}

void initMatrixRecvBuffer(int recvBuffer[][COLUMNS], int rows, int columns, int value) {
    int i, j; 

    for(i = 0; i < rows; i++) {
        for(j = 0; j < columns; j++) {
            recvBuffer[i][j] = value; 
        }
    }
}

void updateMatrixRecvBuffer(int recvBuffer[][COLUMNS], int mbi, int columns, int neighbours[COLUMNS]) {
    int i; 

    for(i = 0; i < columns; i++) {
        switch(neighbours[i]) {
            case 2: 
                if (recvBuffer[mbi][i] != 1) {
                    recvBuffer[mbi][i] = 0;
                }

                break;
            case 3: 
                recvBuffer[mbi][i] = 1;

                break; 
            default: 
                recvBuffer[mbi][i] = 0;
        }
    }
}

void initRequests(int numWorkers, MPI_Request requests[numWorkers]) {
    int i;

    for(i = 0; i < numWorkers; i++) {
        requests[i] = MPI_REQUEST_NULL;
    }
}

// each process sends the last row in their buffer to the next one
int * getTopRow(int workerRank, int numWorkers, int mbi, int matrixRecvBufferRows, int matrixRecvBuffer[][COLUMNS], 
        int tag, MPI_Comm comm, MPI_Request requests[][numWorkers], int *rowBuffer) {
    int src, dest;
    
    src = workerRank - 1;
    dest = workerRank + 1;
    
    if(workerRank != numWorkers - 1 && (mbi == matrixRecvBufferRows - 1 || matrixRecvBuffer[mbi + 1][0] == NIL)) {
        MPI_Isend(matrixRecvBuffer[mbi], COLUMNS, MPI_INT, dest, tag, comm, &requests[SEND][workerRank]);
    }        

    if(workerRank == 0 && mbi == 0) {
        return NULL;
    }
    
    if(mbi != 0) {
        return matrixRecvBuffer[mbi - 1];
    }

    if (src >= 0) {
        MPI_Irecv(rowBuffer, COLUMNS, MPI_INT, src, tag, comm, &requests[RECEIVE][workerRank]);
        MPI_Wait(&requests[1][workerRank], MPI_STATUS_IGNORE);

        return rowBuffer;
    }

    return NULL;
}

// each process sends the first row in their buffer to the previous one
int * getBottomRow(int workerRank, int numWorkers, int mbi, int matrixRecvBufferRows, int matrixRecvBuffer[][COLUMNS], 
        int tag, MPI_Comm comm, MPI_Request requests[][numWorkers], int *rowBuffer) {
    int src, dest;

    src = workerRank + 1;
    dest = workerRank - 1;

    if (workerRank != 0 && mbi == 0) {
        MPI_Isend(matrixRecvBuffer[mbi], COLUMNS, MPI_INT, dest, tag, comm, &requests[SEND][workerRank]);
    }

    if (mbi + 1 < matrixRecvBufferRows && matrixRecvBuffer[mbi + 1][0] != NIL) {
        return matrixRecvBuffer[mbi + 1];
    }

    if (workerRank != numWorkers - 1) {
        MPI_Irecv(rowBuffer, COLUMNS, MPI_INT, src, tag, comm, &requests[RECEIVE][workerRank]);
        MPI_Wait(&requests[1][workerRank], MPI_STATUS_IGNORE);

        return rowBuffer;
    }

    return NULL;
}

int isAlive(int cell) {
    return (cell == 1) ? 1 : 0;
}

void getNeighbours(int *topRow, int *middleRow, int *bottomRow, int columns, int neighbours[COLUMNS]) {
    int i, count, left, right;

    for (i = 0; i < columns; i++) {
        count = 0;
        left = (i > 0) ? i - 1 : -1;
        right = (i < columns - 1) ? i + 1 : -1;

        if (topRow != NULL) {
            if (left != -1) {
                count += isAlive(topRow[left]);
            }

            if (right != -1) {
                count += isAlive(topRow[right]);
            }

            count += isAlive(topRow[i]);
        }

        if (left  != -1) {
            count += isAlive(middleRow[left]);
        }

        if (right != -1) {
            count += isAlive(middleRow[right]);
        }

        if (bottomRow != NULL) {
            if (left  != -1) {
                count += isAlive(bottomRow[left]);
            }

            if (right != -1) {
                count += isAlive(bottomRow[right]);
            }

            count += isAlive(bottomRow[i]);
        }

        neighbours[i] = count;
    }
}