#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/* You'll probably want this at some point */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/* Completely serial algorithm: multiply matrix A with vector B, with the
 * resulting vector being written into vector C. Matrix A has size M rows, N
 * columns, stored indexed by A[row][column] with 0<=row<M and 0<=column<N.
 * Vector B has N rows, and vector C has M rows. */
void matrix_mul_serial(double **A, double *B, double *C, int M, int N) {
	int row, col;

	for(row = 0; row < M; row++) {
		C[row] = 0.0;
		for(col = 0; col < N; col++)
			C[row] += A[row][col] * B[col];
	}
}

struct mtx_mul_params {
	double **A;
	double *B;
	double *C;
	int M;
	int N;
	int row;
};

void *matrix_mul_thread(struct mtx_mul_params *params) {
	/* How might you go about this? Hint: look *inside* the "for" loop above
	 * that iterates over "row".
	 * You can access the parameters through the pointer, e.g., params->A,
	 * params->B, params->M, params->row, etc. */

        /*TODO for students*/
	int M = params->M;
	int N = params->N;
	int row, col;
	for(row = 0; row < M; row++) {
                params->C[row] = 0.0;
                for(col = 0; col < N; col++)
                        params->C[row] += params->A[row][col] * params->B[col];
        }

}

double randElement() {
	return 20.0 * (((double) random()) / RAND_MAX) - 10.0;
}

int main(int argc, char **argv) {
	int M, N, row, col, ok = 1;
	double **A, *B, *Cser, *Cthr;
	pthread_t *threads;
	struct mtx_mul_params *params;

	if(argc < 3) {
		printf("usage: %s <rows> <cols>\n", argv[0]);
		return 1;
	}

	M = atoi(argv[1]);
	N = atoi(argv[2]);
	if(M <= 0 || N <= 0) {
		printf("Bad parameters.\n");
		return 1;
	}

	printf("Matrix A:\n");
	A = malloc(sizeof(double *) * M);
	for(row = 0; row < M; row++) {
		A[row] = malloc(sizeof(double) * N);
		for(col = 0; col < N; col++) {
			A[row][col] = randElement();
			printf("%f\t", A[row][col]);
		}
		printf("\n");
	}

	printf("Vector B:\n");
	B = malloc(sizeof(double) * N);
	for(row = 0; row < N; row++) {
		B[row] = randElement();
		printf("%f\n", B[row]);
	}

	printf("Computing serial multiplication...\n");
	Cser = malloc(sizeof(double) * M);
	matrix_mul_serial(A, B, Cser, M, N);

	printf("Vector Cser:\n");
	for(row = 0; row < M; row++)
		printf("%f\n", Cser[row]);

	printf("Computing parallel multiplication...\n");
	Cthr = malloc(sizeof(double) * M);
	threads = malloc(sizeof(pthread_t) * M);
	params = malloc(sizeof(struct mtx_mul_params) * M);
	for(row = 0; row < M; row++) {
		params[row].A = A;
		params[row].B = B;
		params[row].C = Cthr;
		params[row].M = M;
		params[row].N = N;
		params[row].row = row;
		pthread_create(threads + row, NULL, (void *(*)(void *)) matrix_mul_thread, params + row);
	}

	printf("Waiting for parallel multiplication to complete...\n");
	for(row = 0; row < M; row++)
		pthread_join(threads[row], NULL);

	printf("Vector Cthr:\n");
	for(row = 0; row < M; row++)
		printf("%f\n", Cthr[row]);

	printf("Comparing results...\n");
	for(row = 0; row < M; row++)
		if(Cser[row] != Cthr[row]) {
			printf("Results differ at row %d! Serial, parallel: %f, %f\n", row, Cser[row], Cthr[row]);
			ok = 0;
		}

	if(!ok)
		return 1;
	else
		return 0;
}
