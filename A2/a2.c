#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <limits.h>

#define iterative 50

int n = 0, x, y, maxX = 0, maxY = 0, minX = INT_MAX, minY = INT_MAX;

typedef struct {
    int x;
    int y;
} Point;


int max(int num1, int num2) {
    return (num1 > num2) ? num1 : num2;
}

int min(int num1, int num2) {
    return (num1 > num2) ? num2 : num1;
}

int getLines(char fileName[]) {
    FILE *file = fopen(fileName, "r");
    int chr = 0;
    int lines = 0;
    while (chr != EOF) {
        lines += chr == '\n';
        chr = fgetc(file);
    }
    fclose(file);
    n = lines;
    return lines;
}

Point *readFile(char fileName[]) {
    int lines = getLines(fileName);
    FILE *file = fopen(fileName, "r");
    char line[100];

    Point *points = (Point *) malloc(sizeof(Point *) * lines);
    int i = 0;
    while (fgets(line, 100, file)) {
        sscanf(line, "%d %d", &points[i].x, &points[i].y);
        maxX = max(maxX, points[i].x);
        maxY = max(maxY, points[i].y);
        minX = min(minX, points[i].x);
        minY = min(minY, points[i].y);
        i++;
    }
    fclose(file);
    return points;
}

int Dist(Point a, Point b) {
    int result = sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2) * 1.0);
    return result;
}

int random_number(int min_num, int max_num) {
    int result = 0, low_num = 0, hi_num = 0;
    if (min_num < max_num) {
        low_num = min_num;
        hi_num = max_num + 1; // include max_num in output
    } else {
        low_num = max_num + 1; // include max_num in output
        hi_num = min_num;
    }
    result = (rand() % (hi_num - low_num)) + low_num;
    return result;
}

int main(int argc, char *argv[]) {
    int clusterNumber;
    scanf("%d", &clusterNumber);

    //Reading from file
    int i = 0, j = 0;
    Point *points = readFile("data.txt");
    //Reading from file done
    
    Point centroids[clusterNumber];
    srand(time(NULL));

    for (i = 0; i < clusterNumber; i++) {
        centroids[i].x = random_number(minX, maxX);
        centroids[i].y = random_number(minY, maxY);
    }
    
    int distance[clusterNumber][n];
    int len[clusterNumber];
    Point pointsCluster[clusterNumber][n];


    int t = 0;
    for (; t < iterative; t++) {
        int h, k;
#pragma omp parallel num_threads(clusterNumber) shared (n) private(h, k)
        {
#pragma omp for schedule(static)
            for (h = 0; h < n; h++) {
                for (k = 0; k < clusterNumber; k++) {
                    distance[k][h] = Dist(points[h], centroids[k]);
                }
            }
        }

        for (i = 0; i < clusterNumber; i++) len[i] = 0;
        for (i = 0; i < n; i++) {
            int minDistance = INT_MAX;
            int index = 0;
            int tempDistance;
            for (j = 0; j < clusterNumber; j++) {
                tempDistance = minDistance;
                minDistance = min(distance[j][i], minDistance);
                if (minDistance < tempDistance) index = j;
            }
            pointsCluster[index][len[index]] = points[i];
            len[index]++;
        }

        int stoppingCondition = 0;
#pragma omp parallel num_threads(clusterNumber) shared(pointsCluster,len, stoppingCondition)
        {
            int id = omp_get_thread_num();
            Point centroid;
            int sumX = 0, sumY = 0;
            for (i = 0; i < len[id]; i++) {
                sumX += pointsCluster[id][i].x;
                sumY += pointsCluster[id][i].y;
            }
            if (len[id]) {
                centroid.x = sumX / len[id];
                centroid.y = sumY / len[id];
                if (centroids[id].x == centroid.x && centroids[id].y == centroid.y) {
                    #pragma omp critical
                    {
                        stoppingCondition++;
                    }
                } else {
                    centroids[id].x = centroid.x;
                    centroids[id].y = centroid.y;
                }
            } else {
                #pragma omp critical
                {
                    stoppingCondition++;
                }
            }

        }
        if (stoppingCondition == clusterNumber) {
            break;
        }
    }

    for (i = 0; i < clusterNumber; i++) {
        printf("Cluster %d: \n", i);
        if (len[i] == 0)printf("has no close points \n");
        for (j = 0; j < len[i]; j++) {
            printf("(%d, %d)\n", pointsCluster[i][j].x, pointsCluster[i][j].y);
        }
    }
}
