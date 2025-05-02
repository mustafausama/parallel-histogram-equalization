# Compiler and flags
CXX = g++
MPICXX = mpic++
CXXFLAGS = -O3 -march=native -I/usr/local/include/opencv4
LDFLAGS = -L/usr/local/lib -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc
OMPFLAGS = -fopenmp

# Output binaries
SEQ_BIN = seq.out
OMP_BIN = omp.out
MPI_BIN = mpi.out

# Docker
DOCKER_IMAGE = cppcv-mpi
DOCKER_CONTAINER = cppcv

# Default number of threads
THREADS ?= 4

# Internal quiet flag (set based on QUIET)
QUIET_FLAG := $(if $(filter 1,$(QUIET)),--quiet,)

# Build Docker image
docker-build:
	docker build -f Dockerfile -t $(DOCKER_IMAGE) .

# Run Docker container mounting current directory
docker-run:
	docker run -dit --name $(DOCKER_CONTAINER) -v "$(PWD)":/workspace $(DOCKER_IMAGE)

# Build all binaries inside Docker
build-all: build-seq build-omp build-mpi

# ---- Sequential ----
build-seq:
	docker exec -w /workspace $(DOCKER_CONTAINER) $(CXX) $(CXXFLAGS) seq.cpp utils.cpp -o $(SEQ_BIN) $(LDFLAGS)

run-seq:
	@if [ -z "$(IMAGE)" ]; then \
		echo "Error: You must provide IMAGE (e.g., IMAGE=\"input/einstein.jpg\")"; \
		exit 1; \
	fi
	docker exec -w /workspace $(DOCKER_CONTAINER) sh -c 'LD_LIBRARY_PATH=/usr/local/lib ./$(SEQ_BIN) $(QUIET_FLAG) $(IMAGE)'

build-run-seq: build-seq run-seq

# ---- OpenMP ----
build-omp:
	docker exec -w /workspace $(DOCKER_CONTAINER) $(CXX) $(CXXFLAGS) $(OMPFLAGS) omp.cpp utils.cpp -o $(OMP_BIN) $(LDFLAGS)

run-omp:
	@if [ -z "$(IMAGE)" ]; then \
		echo "Error: You must provide IMAGE (e.g., IMAGE=\"input/einstein.jpg\")"; \
		exit 1; \
	fi
	docker exec -w /workspace $(DOCKER_CONTAINER) sh -c 'OMP_NUM_THREADS=$(THREADS) LD_LIBRARY_PATH=/usr/local/lib ./$(OMP_BIN) $(QUIET_FLAG) $(IMAGE)'

build-run-omp: build-omp run-omp

# ---- MPI ----
build-mpi:
	docker exec -w /workspace $(DOCKER_CONTAINER) $(MPICXX) $(CXXFLAGS) mpi.cpp utils.cpp -o $(MPI_BIN) $(LDFLAGS)

run-mpi:
	@if [ -z "$(IMAGE)" ]; then \
		echo "Error: You must provide IMAGE (e.g., IMAGE=\"input/einstein.jpg\")"; \
		exit 1; \
	fi
	docker exec -w /workspace $(DOCKER_CONTAINER) sh -c 'LD_LIBRARY_PATH=/usr/local/lib mpirun --allow-run-as-root -np $(THREADS) ./$(MPI_BIN) $(QUIET_FLAG) $(IMAGE)'

build-run-mpi: build-mpi run-mpi

# ---- Combine all ----
build-combine:
	docker exec -w /workspace $(DOCKER_CONTAINER) $(CXX) $(CXXFLAGS) combine_all.cpp utils.cpp -o combine_all.out $(LDFLAGS)

run-combine-only:
	docker exec -w /workspace $(DOCKER_CONTAINER) sh -c 'LD_LIBRARY_PATH=/usr/local/lib ./combine_all.out'

combine: build-combine run-combine-only

# Run-combine: run seq, omp, mpi, then combine
run-combine: run-seq run-omp run-mpi combine

# Clean binaries (does NOT remove Docker container)
clean:
	docker exec -w /workspace $(DOCKER_CONTAINER) rm -f $(SEQ_BIN) $(OMP_BIN) $(MPI_BIN)
