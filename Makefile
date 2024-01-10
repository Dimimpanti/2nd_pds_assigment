MPICC = mpicc
CC = gcc


BUILD_DIR = make-build-debug-gcc
MPI_BUILD_DIR = make-build-debug-mpicc
DATA_DIR = data
SRC_DIR = src
VALGRIND_DIR = valgrind


SEQUENTIAL_FILES = $(SRC_DIR)/sequential/sequential.c $(SRC_DIR)/partition.c
SEQUENTIAL_FILES := $(SEQUENTIAL_FILES:%=$(BUILD_DIR)/%.o)

MPI_FILES = $(SRC_DIR)/mpi/main.c $(SRC_DIR)/mpi/masterProcess.c $(SRC_DIR)/mpi/slaveProcess.c $(SRC_DIR)/partition.c
MPI_FILES := $(MPI_FILES:%=$(MPI_BUILD_DIR)/%.o)

DATA_FILES = $(DATA_DIR)/data.c
DATA_FILES := $(DATA_FILES:%=$(BUILD_DIR)/%.o)

MPI_INCLUDES = -I$(SRC_DIR)/mpi -I$(SRC_DIR)

MPICC_FLAGS = -Wno-alloc-size-larger-than -O3 -g
CC_FLAGS = -O3 -g


$(BUILD_DIR)/data.out: $(DATA_FILES)
	@echo
	@echo -e "			Linking"
	@echo
	@$(CC) $(CC_FLAGS) -o $(BUILD_DIR)/data.out $(DATA_FILES)
	@echo -e "			Build finished successfully!"

$(BUILD_DIR)/sequential.out: $(SEQUENTIAL_FILES)
	@echo
	@echo -e "		Linking"
	@echo
	@$(CC) $(CC_FLAGS) -o $(BUILD_DIR)/sequential.out $(SEQUENTIAL_FILES)
	@echo -e "		Build finished successfully!"	


$(BUILD_DIR)/%.c.o: %.c
	@echo
	@echo -e "		Compiling $<"
	@echo
	@mkdir -p $(dir $@)
	@$(CC) $(CC_FLAGS) -c $< -o $@


$(MPI_BUILD_DIR)/mpi.out: $(MPI_FILES)
	@echo
	@echo -e "		Linking"
	@echo
	@$(MPICC) $(MPICC_FLAGS) -o $(MPI_BUILD_DIR)/mpi.out $(MPI_FILES) $(MPI_INCLUDES)
	@echo -e "		Build finished successfully!"


$(MPI_BUILD_DIR)/%.c.o: %.c
	@echo
	@echo -e "		Compiling $<"
	@echo
	@mkdir -p $(dir $@)
	@$(MPICC) $(MPICC_FLAGS) $(MPI_INCLUDES) -c $< -o $@

%.c:


DATA_ARGS = data_2.txt 10
SEQUENTIAL_ARGS = data_2.txt 123
MPI_ARGS = data_2.txt 123
MPIRUN_ARGS = -hostfile host
VALGRIND_ARGS = --leak-check=full --track-origins=yes --suppressions=/usr/share/openmpi/openmpi-valgrind.supp --log-file=$(VALGRIND_DIR)/valgrind.%p.log

build_data: $(BUILD_DIR)/data.out
run_data: $(BUILD_DIR)/data.out
	@echo
	@echo
	@$(BUILD_DIR)/data.out ./$(DATA_DIR)/$(DATA_ARGS)
	@echo
	@echo 

build_sequential: $(BUILD_DIR)/sequential.out
run_sequential: $(BUILD_DIR)/sequential.out
	@echo
	@echo
	@$(BUILD_DIR)/sequential.out ./$(DATA_DIR)/$(SEQUENTIAL_ARGS)
	@echo
	@echo

build_mpi: $(MPI_BUILD_DIR)/mpi.out
run_mpi: $(MPI_BUILD_DIR)/mpi.out
	@echo
	@echo
	@mpirun $(MPIRUN_ARGS) $(MPI_BUILD_DIR)/mpi.out ./$(DATA_DIR)/$(MPI_ARGS)
	@echo

valgrind_mpi: $(MPI_BUILD_DIR)/mpi.out
	@echo
	@echo
	@mkdir -p $(VALGRIND_DIR)
	@mpirun $(MPIRUN_ARGS) valgrind $(VALGRIND_ARGS) $(MPI_BUILD_DIR)/mpi.out ./$(DATA_DIR)/$(MPI_ARGS)
	@echo

.PHONY: clean
clean:
	@echo
	@echo -e "			Cleaning"
	@echo
	@rm -rf $(BUILD_DIR)/*
	@rm -rf $(MPI_BUILD_DIR)/*
	@echo -e "			Clean finished successfully!"

.PHONY: clean_valgrind
clean_valgrind:
	@echo
	@echo -e "			Cleaning valgrind"
	@echo
	@rm -rf $(VALGRIND_DIR)/*
	@echo -e "			Clean finished successfully!"