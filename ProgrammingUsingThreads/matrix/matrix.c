#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define MAX 20
#define NUM_THREADS 10

int matA[MAX][MAX];
int matB[MAX][MAX];

int matSumResult[MAX][MAX];
int matDiffResult[MAX][MAX];
int matProductResult[MAX][MAX];

typedef struct {
    int start_row;
    int end_row;
} ThreadData;

void fillMatrix(int matrix[MAX][MAX]) {
    for (int i = 0; i < MAX; i++) {
        for (int j = 0; j < MAX; j++) {
            matrix[i][j] = rand() % 10 + 1;
        }
    }
}

void printMatrix(int matrix[MAX][MAX]) {
    for (int i = 0; i < MAX; i++) {
        for (int j = 0; j < MAX; j++) {
            printf("%5d", matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

void* computeSum(void* args) {
    ThreadData* data = (ThreadData*)args;
    for (int i = data->start_row; i < data->end_row; i++) {
        for (int j = 0; j < MAX; j++) {
            matSumResult[i][j] = matA[i][j] + matB[i][j];
        }
    }
    free(data); // Free allocated memory for thread data
    return NULL;
}

void* computeDiff(void* args) {
    ThreadData* data = (ThreadData*)args;
    for (int i = data->start_row; i < data->end_row; i++) {
        for (int j = 0; j < MAX; j++) {
            matDiffResult[i][j] = matA[i][j] - matB[i][j];
        }
    }
    free(data);
    return NULL;
}

void* computeProduct(void* args) {
    ThreadData* data = (ThreadData*)args;
    for (int i = data->start_row; i < data->end_row; i++) {
        for (int j = 0; j < MAX; j++) {
            matProductResult[i][j] = 0;
            for (int k = 0; k < MAX; k++) {
                matProductResult[i][j] += matA[i][k] * matB[k][j];
            }
        }
    }
    free(data);
    return NULL;
}

int main() {
    srand(time(0));

    // 1. Fill the matrices (matA and matB) with random values.
    fillMatrix(matA);
    fillMatrix(matB);

    // 2. Print the initial matrices.
    printf("Matrix A:\n");
    printMatrix(matA);
    printf("Matrix B:\n");
    printMatrix(matB);

    // 3. Create pthread_t objects for our threads.
    pthread_t threads[NUM_THREADS];
    int rows_per_thread = MAX / NUM_THREADS;

    // 4. Perform matrix sum in parallel.
    for (int i = 0; i < NUM_THREADS; i++) {
        ThreadData* data = malloc(sizeof(ThreadData));
        data->start_row = i * rows_per_thread;
        data->end_row = (i + 1) * rows_per_thread;

        pthread_create(&threads[i], NULL, computeSum, data);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // 5. Perform matrix difference in parallel.
    for (int i = 0; i < NUM_THREADS; i++) {
        ThreadData* data = malloc(sizeof(ThreadData));
        data->start_row = i * rows_per_thread;
        data->end_row = (i + 1) * rows_per_thread;

        pthread_create(&threads[i], NULL, computeDiff, data);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // 6. Perform matrix product in parallel.
    for (int i = 0; i < NUM_THREADS; i++) {
        ThreadData* data = malloc(sizeof(ThreadData));
        data->start_row = i * rows_per_thread;
        data->end_row = (i + 1) * rows_per_thread;

        pthread_create(&threads[i], NULL, computeProduct, data);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // 7. Print the results.
    printf("Results:\n");
    printf("Sum:\n");
    printMatrix(matSumResult);
    printf("Difference:\n");
    printMatrix(matDiffResult);
    printf("Product:\n");
    printMatrix(matProductResult);

    return 0;
}
