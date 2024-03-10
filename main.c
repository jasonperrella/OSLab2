#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#define BOARD_SIZE 9
#define SUBGRID_SIZE 3
int sudokuBoard[BOARD_SIZE][BOARD_SIZE];
int valid;
int option;
int pipefd[2];

typedef struct
{
    int row;
    int column;
} parameters;

void isValid()
{
    valid = -1;
    if (option == 3)
    {
        write(pipefd[1], &valid, sizeof(int));
        exit(0);
    }
}

void printBoard()
{
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            printf("%d ", sudokuBoard[i][j]);
        }
        printf("\n");
    }
}
// ADD IF VALID = -1 CONDITION TO ALL FUNCTIONS
// ADD GLOBAL VARIABLE TO TRACK WHICH INDEX THE ERROR IS

// Function to validate a row
void *validateRow(void *param)
{
    parameters *data = (parameters *)param;
    int row = data->row;

    while (row < BOARD_SIZE)
    {
        int seen[9] = {0};
        for (int i = 0; i < BOARD_SIZE; i++)
        {
            if (seen[sudokuBoard[row][i] - 1] == 1)
            {
                isValid();
                pthread_exit(0);
            }
            else
            {
                seen[sudokuBoard[row][i] - 1] = 1;
            }
            // printf("seen[%d] = %d\n", sudokuBoard[row][i], seen[i-1]);
        }
        row++;
    }
    pthread_exit(0);
}

// Function to validate a column
void *validateColumn(void *param)
{
    parameters *data = (parameters *)param;
    int col = data->column;

    while (col < BOARD_SIZE)
    {
        int seen[9] = {0};
        for (int i = 0; i < BOARD_SIZE; i++)
        {
            if (seen[sudokuBoard[i][col] - 1] == 1)
            {
                isValid();
                pthread_exit(0);
            }
            else
            {
                seen[sudokuBoard[i][col] - 1] = 1;
            }
            // printf("seen[%d] = %d\n", sudokuBoard[row][i], seen[i-1]);
        }
        col++;
    }
    pthread_exit(0);
}

// Function to validate a subgrid
void *validateSubgrid(void *param)
{
    parameters *data = (parameters *)param;
    int startRow = data->row;
    int startCol = data->column;

    int seen[BOARD_SIZE] = {0};

    for (int row = startRow; row < startRow + SUBGRID_SIZE; row++)
    {
        for (int col = startCol; col < startCol + SUBGRID_SIZE; col++)
        {
            int num = sudokuBoard[row][col];

            if (seen[num - 1] == 1)
            {
                isValid();
                pthread_exit(0);
            }
            else
            {
                seen[num - 1] = 1;
            }
        }
    }

    pthread_exit(0);
}

// Function to read the Sudoku board from a file
void readBoardFromFile(FILE *sudokufile, int sudokuBoard[BOARD_SIZE][BOARD_SIZE])
{
    sudokufile = fopen("input.txt", "r");
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            if (fscanf(sudokufile, "%d", &sudokuBoard[i][j]) != 1)
            {
                printf("Error reading input file\n");
                return;
            }
            // printf("%d ", sudokuBoard[i][j]);
        }
        // printf("\n");
    }
}

int main(int argc, char *argv[])
{
    clock_t start_time = clock();
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <option>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    option = atoi(argv[1]);
    FILE *fptr;
    readBoardFromFile(fptr, sudokuBoard);
    parameters *checkRows = (parameters *)malloc(sizeof(parameters));
    parameters *checkCols = (parameters *)malloc(sizeof(parameters));
    parameters *check3by3 = (parameters *)malloc(sizeof(parameters));
    valid = 1;
    printBoard();

    // Create threads based on the selected option
    int threadIndex = 0;

    if (option == 1)
    {
        // Create 11 threads to validate rows, columns, and subgrids
        pthread_t threads[11];
        parameters params[11];

        checkRows->row = 0;
        checkRows->column = 0;
        pthread_create(&threads[0], NULL, validateColumn, (void *)checkCols);

        checkRows->row = 0;
        checkRows->column = 0;
        pthread_create(&threads[1], NULL, validateRow, (void *)checkRows);

        for (int threadIndex = 2; threadIndex < 11; threadIndex++)
        {
            for (int i = 0; i < 7; i+=3)
            {
                for (int j = 0; j < 7; j+=3)
                {
                    check3by3->row = i;
                    check3by3->column = j;
                    pthread_create(&threads[threadIndex], NULL, validateSubgrid, (void *)check3by3);
                }
            }
        }
        for (int i = 0; i < 11; i++)
        {
            pthread_join(threads[i], NULL);
        }
    }
    else if (option == 2)
    {
        pthread_t threads[27];
        parameters params[27];
        for (int i = 0; i < 9; i++)
        {
            checkRows->row = 0;
            checkRows->column = i;
            pthread_create(&threads[i], NULL, validateColumn, (void *)checkCols);
        }
        for (int i = 9; i < 18; i++)
        {
            checkRows->row = i - 9;
            checkRows->column = 0;
            pthread_create(&threads[i], NULL, validateRow, (void *)checkRows);
        }
        for (int threadIndex = 18; threadIndex < 27; threadIndex++)
        {
            for (int i = 0; i < 7; i+=3)
            {
                for (int j = 0; j < 7; j+=3)
                {
                    check3by3->row = i;
                    check3by3->column = j;
                    pthread_create(&threads[threadIndex], NULL, validateSubgrid, (void *)check3by3);
                }
            }
        }
        for (int i = 0; i < 27; i++)
        {
            pthread_join(threads[i], NULL);
        }
    }
    else if (option == 3)
    {
        // Create 4 processes - parent, row process, column process, subgrid process

        pid_t pids[3];
        if (pipe(pipefd) == -1)
        {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
        for (int i = 0; i < 3; i++)
        {
            pids[i] = fork();
            if (pids[i] < 0)
            {
                perror("fork() failed");
                exit(EXIT_FAILURE);
            }
            if (pids[i] == 0)
            {
                if (i == 0)
                {
                    /* row process */
                    for (int row = 0; row < BOARD_SIZE; row++)
                    {
                        checkRows->row = row;
                        validateRow((void *)checkRows);
                    }
                    exit(0);
                }
                if (i == 1)
                {
                    /* column process */
                    for (int col = 0; col < BOARD_SIZE; col++)
                    {
                        checkCols->column = col;
                        validateColumn((void *)checkCols);
                    }
                    exit(0);
                }
                if (i == 2)
                {
                    /* subgrid process */
                    for (int row = 0; row < BOARD_SIZE; row += SUBGRID_SIZE)
                    {
                        for (int col = 0; col < BOARD_SIZE; col += SUBGRID_SIZE)
                        {
                            check3by3->row = row;
                            check3by3->column = col;
                            validateSubgrid((void *)check3by3);
                        }
                    }
                    exit(0);
                }
            }
        }
        for (int i = 0; i < 3; i++)
        {
            waitpid(pids[i], NULL, 0);
        }
        //parent has to close the write end
        close(pipefd[1]);
        read(pipefd[0], &valid, sizeof(int));
        close(pipefd[0]);

    }

    clock_t end_time;
    double time_taken;
    if (valid == -1)
    {
        end_time = clock();
        time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
        printf("SOLUTION: NO(%f seconds)\n", time_taken);
    }
    else
    {
        end_time = clock();
        time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
        printf("SOLUTION: YES(%f seconds)\n", time_taken);
    }
    free(checkRows);
    free(checkCols);
    free(check3by3);
    return 0;
}
