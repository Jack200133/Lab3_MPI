## Instrucciones de KOMPILATIONEN

### MPI Vector Add
En este se hace la suma y el producto punto
```bash
mpicc -g -Wall -o mpi_vector_add mpi_vector_add_mod.c 
```

```bash
mpiexec -n 4 ./mpi_vector_add
```

### Vector Add

```bash 
gcc -g -Wall -o vector_add vector_add_mod.c 
./vector_add
```

### MPI Vector Add Escalar
```bash
mpicc -g -Wall -o mpi_vector_add_escalar mpi_vector_add_escalar.c 
```

```bash
mpiexec -n 4 ./mpi_vector_add_escalar
```