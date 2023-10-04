#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

#define SIZE 20    // Definimos el tamaño del vector
#define SCALAR 2.0 // Escalar para la multiplicación

// Prototipos de funciones
void Parallel_scalar_product(double local_x[], double local_z[], int local_n);
void Allocate_vectors(double **local_x_pp, double **local_z_pp, int local_n, MPI_Comm comm);
void Generate_random_vector(double local_a[], int local_n, int n, int my_rank, MPI_Comm comm);
void Print_vector10(double local_b[], int local_n, int n, char title[], int my_rank, MPI_Comm comm);

void Check_for_error(int local_ok, char fname[], char message[], MPI_Comm comm);
void Read_n(int *n_p, int *local_n_p, int my_rank, int comm_sz, MPI_Comm comm);

int main(void)
{
    int n = SIZE;
    int local_n, comm_sz, my_rank;
    double *local_x, *local_z;
    MPI_Comm comm;
    double tstart, tend;

    MPI_Init(NULL, NULL);
    comm = MPI_COMM_WORLD;
    MPI_Comm_size(comm, &comm_sz);
    MPI_Comm_rank(comm, &my_rank);
    local_n = n / comm_sz;

    tstart = MPI_Wtime();
    Allocate_vectors(&local_x, &local_z, local_n, comm);

    Generate_random_vector(local_x, local_n, n, my_rank, comm);
    Print_vector10(local_x, local_n, n, "x is", my_rank, comm);

    Parallel_scalar_product(local_x, local_z, local_n);
    tend = MPI_Wtime();

    Print_vector10(local_z, local_n, n, "The product is", my_rank, comm);

    if (my_rank == 0)
    {
        printf("\nTook %f sec to run\n", (tend - tstart));
    }

    free(local_x);
    free(local_z);

    MPI_Finalize();

    return 0;
}

void Parallel_scalar_product(double local_x[], double local_z[], int local_n)
{
    for (int i = 0; i < local_n; i++)
    {
        local_z[i] = local_x[i] * SCALAR;
    }
}

void Allocate_vectors(double **local_x_pp, double **local_z_pp, int local_n, MPI_Comm comm)
{
    int local_ok = 1;
    char *fname = "Allocate_vectors";

    *local_x_pp = malloc(local_n * sizeof(double));
    *local_z_pp = malloc(local_n * sizeof(double));

    if (*local_x_pp == NULL || *local_z_pp == NULL)
        local_ok = 0;
    Check_for_error(local_ok, fname, "Can't allocate local vector(s)", comm);
}

void Generate_random_vector(double local_a[], int local_n, int n, int my_rank, MPI_Comm comm)
{
    double wtime = MPI_Wtime();
    long long_seed = (long)(1e9 * wtime) + my_rank; // Convierte tiempo en segundos a una especie de nanosegundos
    srand(long_seed);

    for (int i = 0; i < local_n; i++)
        local_a[i] = (double)rand() / RAND_MAX;
}

void Print_vector10(double local_b[], int local_n, int n, char title[], int my_rank, MPI_Comm comm)
{
    double *b = NULL;
    int i;
    int local_ok = 1;
    char *fname = "Print_vector";

    if (my_rank == 0)
    {
        b = malloc(n * sizeof(double));
        if (b == NULL)
            local_ok = 0;
        Check_for_error(local_ok, fname, "Can't allocate temporary vector", comm);
        MPI_Gather(local_b, local_n, MPI_DOUBLE, b, local_n, MPI_DOUBLE, 0, comm);
        printf("%s\n", title);
        // Imprimir los primeros 10 elementos
        for (i = 0; i < 10 && i < n; i++)
            printf("%f ", b[i]);

        printf("\n... \n"); // Indicador de que hay elementos omitidos en medio

        // Imprimir los últimos 10 elementos
        for (i = (n - 10 > 0 ? n - 10 : 0); i < n; i++)
            printf("%f ", b[i]);

        printf("\n");
        free(b);
    }
    else
    {
        Check_for_error(local_ok, fname, "Can't allocate temporary vector", comm);
        MPI_Gather(local_b, local_n, MPI_DOUBLE, b, local_n, MPI_DOUBLE, 0, comm);
    }
}

void Check_for_error(int local_ok, char fname[], char message[], MPI_Comm comm)
{
    int ok;

    MPI_Allreduce(&local_ok, &ok, 1, MPI_INT, MPI_MIN, comm);
    if (ok == 0)
    {
        int my_rank;
        MPI_Comm_rank(comm, &my_rank);
        if (my_rank == 0)
        {
            fprintf(stderr, "Proc %d > In %s, %s\n", my_rank, fname, message);
            fflush(stderr);
        }
        MPI_Finalize();
        exit(-1);
    }
}

void Read_n(int *n_p, int *local_n_p, int my_rank, int comm_sz, MPI_Comm comm)
{
    int local_ok = 1;
    char *fname = "Read_n";

    if (my_rank == 0)
    {
        printf("What's the order of the vectors?\n");
        scanf("%d", n_p);
    }
    MPI_Bcast(n_p, 1, MPI_INT, 0, comm);
    if (*n_p <= 0 || *n_p % comm_sz != 0)
        local_ok = 0;
    Check_for_error(local_ok, fname, "n should be > 0 and evenly divisible by comm_sz", comm);
    *local_n_p = *n_p / comm_sz;
}
