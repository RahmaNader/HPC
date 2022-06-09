#include <stdio.h>
#include <mpi.h>
#include <omp.h>
#include <stdlib.h>
#include <limits.h>

void readFileGetMinMax(int data[], int *minValue, int *maxValue) {
    int line;
    *maxValue = INT_MIN;
    *minValue = INT_MAX;
    FILE *ptr;
    ptr = fopen("dataset.txt", "r");

    if (NULL == ptr) {
        printf("file can't be opened \n");
    }
    int n = 0;
    while (fscanf(ptr, "%d", &line) == 1) {
        data[n] = line;
        if (line < *minValue) {
            *minValue = line;
        }
        if (line > *maxValue) {
            *maxValue = line;
        }
        n++;
    }
    fclose(ptr);
}

void initializeBars(int barMaxes[], int minValue, int range, int barsNumber) {
    int start = minValue - 1;
    for (int i = 0; i <= barsNumber; i++) {
        barMaxes[i] = start;
        start += range;
    }
}

int findBar(int dataPoint, int barMaxes[], int barsNumber) {
    int i;
    for (i = 0; i < barsNumber; ++i) {
        if (dataPoint > barMaxes[i] && dataPoint <= barMaxes[i + 1])
            return i;
    }
    return -1;
}

int main(int argc, char *argv[]) {
    int *data;
    int *localData;
    int *barMaxes;
    int *barFreq;
    int *localBarFreq;
    int *threadBarFreq;
    int dataCount = 0;
    int localDataCount = 0;
    int minValue;
    int maxValue;
    int threadNumber;
    int barsNumber;
    int range;
    int bar;
    int i = 0;
    int j = 0;
    int numprocs;
    int rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        scanf("%d", &dataCount);
        scanf("%d", &threadNumber);
        scanf("%d", &barsNumber);
        localDataCount = (dataCount / numprocs) + ((dataCount % numprocs) != 0);
        data = malloc((localDataCount * numprocs) * sizeof(int));
        #pragma omp parallel for num_threads(threadNumber) private (j)
        for (j = dataCount; j < (localDataCount * numprocs); ++j) {
            data[j] = -1;
        }
        readFileGetMinMax(data, &minValue, &maxValue);
        range = ((maxValue - minValue) / barsNumber) + 1;
        barFreq = calloc(barsNumber, sizeof(int));
    }

    MPI_Bcast(&threadNumber, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&barsNumber, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&localDataCount, 1, MPI_INT, 0, MPI_COMM_WORLD);


    barMaxes = malloc((barsNumber + 1) * sizeof(int));
    localBarFreq = calloc(barsNumber, sizeof(int));
    localData = malloc(localDataCount * sizeof(int));

    if (rank == 0) {
        initializeBars(barMaxes, minValue, range, barsNumber);
    }
    MPI_Bcast(barMaxes, barsNumber + 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(data, localDataCount, MPI_INT, localData, localDataCount, MPI_INT, 0, MPI_COMM_WORLD);
#pragma omp parallel num_threads(threadNumber) private (i,j, threadBarFreq)
    {
        threadBarFreq = calloc(barsNumber, sizeof(int));
#pragma omp for
        for (i = 0; i < localDataCount; i++) {
            if (localData[i] != -1) {
                bar = findBar(localData[i], barMaxes, barsNumber);
                threadBarFreq[bar]++;
            }
        }
#pragma omp critical
        {
            for (j = 0; j < barsNumber; ++j) {
                localBarFreq[j] += threadBarFreq[j];
            }
        }
        free(threadBarFreq);
    }
    MPI_Reduce(localBarFreq, barFreq, barsNumber, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    if (rank == 0) {
        for (j = 0; j < barsNumber; ++j) {
            printf("The range start with %d, end with %d with count %d\n", barMaxes[j], barMaxes[j + 1], barFreq[j]);
        }
        free(barFreq);
        free(data);
    }
    free(barMaxes);
    free(localBarFreq);
    free(localData);
    MPI_Finalize();
    return 0;
}
