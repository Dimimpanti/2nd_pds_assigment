## 2nd_pds_assigment

### Run the code

To run the code, you need to have the mpicc compiler installed and added to the PATH. The makefile provides the following targets:

#### 1. make run_data

Target thats creates the data file filled with random numbers.

```bash
make run_data DATA_ARGS="data.txt 20"
```

`DATA_ARGS`: Is a variable that is passed to the `data` executable. It is used to specify the name of the data file and the number of random numbers to be generated. `WARNING`: The number of random numbers is given as a power of 2. So, in the example above, `2^20` random numbers will be generated.

#### 2. make run_sequential

Target that runs the sequential version of the program.

```bash
make run_sequential SEQUENTIAL_ARGS="data.txt 123"
```

`SEQUENTIAL_ARGS`: Is a variable that is passed to the `sequential` executable. It is used to specify the name of the data file and the kt value.

#### 3. make run_mpi

Target that runs the MPI version of the program.

```bash
make run_mpi MPI_ARGS="data.txt 123"
```

`MPI_ARGS`: Is a variable that is passed to the `mpi` executable. It is used to specify the name of the data file and the kt value.

#### 4. make valgrind_mpi

Target that runs the MPI version of the program with valgrind for debugging. You have to install valgrind and add it to the PATH.

```bash
make valgrind_mpi MPI_ARGS="data.txt 123"
```


### MPI host file

The mpi host file must be present in the same directory as the makefile and must be named `host`. To change the number of nodes, you must change the number of slots in the host file.
