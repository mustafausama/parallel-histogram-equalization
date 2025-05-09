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
docker-build-image:
	docker build -f Dockerfile -t $(DOCKER_IMAGE) .

# Run Docker container mounting current directory
docker-run-container:
	docker run -dit --name $(DOCKER_CONTAINER) -v "$(PWD)":/workspace $(DOCKER_IMAGE)

# Build all binaries inside Docker
docker-build-all: docker-build-seq docker-build-omp docker-build-mpi

# ---- Sequential ----
docker-build-seq:
	docker exec -w /workspace $(DOCKER_CONTAINER) $(CXX) $(CXXFLAGS) seq.cpp utils.cpp -o $(SEQ_BIN) $(LDFLAGS)

docker-run-seq:
	@if [ -z "$(IMAGE)" ]; then \
		echo "Error: You must provide IMAGE (e.g., IMAGE=\"input/einstein.jpg\")"; \
		exit 1; \
	fi
	docker exec -w /workspace $(DOCKER_CONTAINER) sh -c 'LD_LIBRARY_PATH=/usr/local/lib ./$(SEQ_BIN) $(QUIET_FLAG) $(IMAGE)'

docker-build-run-seq: docker-build-seq docker-run-seq

# Local equivalents
build-seq:
	$(CXX) $(CXXFLAGS) seq.cpp utils.cpp -o $(SEQ_BIN) $(LDFLAGS)

run-seq:
	@if [ -z "$(IMAGE)" ]; then \
		echo "Error: You must provide IMAGE (e.g., IMAGE=\"input/einstein.jpg\")"; \
		exit 1; \
	fi
	LD_LIBRARY_PATH=/usr/local/lib ./$(SEQ_BIN) $(QUIET_FLAG) $(IMAGE)

build-run-seq: build-seq run-seq

# ---- OpenMP ----
docker-build-omp:
	docker exec -w /workspace $(DOCKER_CONTAINER) $(CXX) $(CXXFLAGS) $(OMPFLAGS) omp.cpp utils.cpp -o $(OMP_BIN) $(LDFLAGS)

docker-run-omp:
	@if [ -z "$(IMAGE)" ]; then \
		echo "Error: You must provide IMAGE (e.g., IMAGE=\"input/einstein.jpg\")"; \
		exit 1; \
	fi
	docker exec -w /workspace $(DOCKER_CONTAINER) sh -c 'OMP_NUM_THREADS=$(THREADS) LD_LIBRARY_PATH=/usr/local/lib ./$(OMP_BIN) $(QUIET_FLAG) $(IMAGE)'

docker-build-run-omp: docker-build-omp docker-run-omp

# Local equivalents
build-omp:
	$(CXX) $(CXXFLAGS) $(OMPFLAGS) omp.cpp utils.cpp -o $(OMP_BIN) $(LDFLAGS)

run-omp:
	@if [ -z "$(IMAGE)" ]; then \
		echo "Error: You must provide IMAGE (e.g., IMAGE=\"input/einstein.jpg\")"; \
		exit 1; \
	fi
	OMP_NUM_THREADS=$(THREADS) LD_LIBRARY_PATH=/usr/local/lib ./$(OMP_BIN) $(QUIET_FLAG) $(IMAGE)

build-run-omp: build-omp run-omp

# ---- MPI ----
docker-build-mpi:
	docker exec -w /workspace $(DOCKER_CONTAINER) $(MPICXX) $(CXXFLAGS) mpi.cpp utils.cpp -o $(MPI_BIN) $(LDFLAGS)

docker-run-mpi:
	@if [ -z "$(IMAGE)" ]; then \
		echo "Error: You must provide IMAGE (e.g., IMAGE=\"input/einstein.jpg\")"; \
		exit 1; \
	fi
	docker exec -w /workspace $(DOCKER_CONTAINER) sh -c 'LD_LIBRARY_PATH=/usr/local/lib mpirun --allow-run-as-root -np $(THREADS) ./$(MPI_BIN) $(QUIET_FLAG) $(IMAGE)'

docker-build-run-mpi: docker-build-mpi docker-run-mpi

# Local equivalents
build-mpi:
	$(MPICXX) $(CXXFLAGS) mpi.cpp utils.cpp -o $(MPI_BIN) $(LDFLAGS)

run-mpi:
	@if [ -z "$(IMAGE)" ]; then \
		echo "Error: You must provide IMAGE (e.g., IMAGE=\"input/einstein.jpg\")"; \
		exit 1; \
	fi
	LD_LIBRARY_PATH=/usr/local/lib mpirun -np $(THREADS) ./$(MPI_BIN) $(QUIET_FLAG) $(IMAGE)

build-run-mpi: build-mpi run-mpi

# ---- Combine all ----
docker-build-combine:
	docker exec -w /workspace $(DOCKER_CONTAINER) $(CXX) -std=c++17 $(CXXFLAGS) combine_all.cpp utils.cpp -o combine_all.out $(LDFLAGS)

docker-run-combine:
	docker exec -w /workspace $(DOCKER_CONTAINER) sh -c 'LD_LIBRARY_PATH=/usr/local/lib ./combine_all.out'

docker-build-run-combine: docker-build-combine docker-run-combine

# Local equivalents
build-combine:
	$(CXX) -std=c++17 $(CXXFLAGS) combine_all.cpp utils.cpp -o combine_all.out $(LDFLAGS)

run-combine:
	LD_LIBRARY_PATH=/usr/local/lib ./combine_all.out

build-run-combine: build-combine run-combine

# Run-all-combine: run seq, omp, mpi, then combine
docker-build-run-all-combine: docker-build-all docker-build-combine docker-run-all-combine

docker-run-all-combine: docker-run-seq docker-run-omp docker-run-mpi docker-run-combine

# Local equivalent
build-run-all-combine: build-seq build-omp build-mpi build-combine run-seq run-omp run-mpi run-combine

# Run-all-combine: run seq, omp, mpi, then combine
run-all-combine: run-seq run-omp run-mpi run-combine

# Clean binaries (does NOT remove Docker container)
docker-clean:
	docker exec -w /workspace $(DOCKER_CONTAINER) rm -f $(SEQ_BIN) $(OMP_BIN) $(MPI_BIN)

clean:
	rm -f $(SEQ_BIN) $(OMP_BIN) $(MPI_BIN) combine_all.out
