#define ROWS 24
#define COLUMNS 38
#define NIL 444
#define SEND 0 
#define RECEIVE 1

void initMatrix(int matrix[][COLUMNS], int rows, int columns);
void printMatrix(int matrix[][COLUMNS], int rows, int columns);
void loadBalanceMatrix(int numWorkers, int rows, int columns, int *sendCounts, int *displacements);
int getMatrixRecvBufferRows(int numWorkers, int rows);
void initMatrixRecvBuffer(int recvBuffer[][COLUMNS], int rows, int columns, int value);
void initRequests(int numWorkers, MPI_Request requests[numWorkers]);
int * getTopRow(int workerRank, int numWorkers, int mbi, int matrixRecvBufferRows, int matrixRecvBuffer[][COLUMNS], 
        int tag, MPI_Comm comm, MPI_Request requests[][numWorkers], int *rowBuffer);
int * getBottomRow(int workerRank, int numWorkers, int mbi, int matrixRecvBufferRows, int matrixRecvBuffer[][COLUMNS], 
        int tag, MPI_Comm comm, MPI_Request requests[][numWorkers], int *rowBuffer);
int isAlive(int cell); 
void getNeighbours(int *topRow, int *currentRow, int *bottomRow, int columns, int neighbours[COLUMNS]);
void updateMatrixRecvBuffer(int recvBuffer[][COLUMNS], int mbi, int columns, int neighbours[COLUMNS]);