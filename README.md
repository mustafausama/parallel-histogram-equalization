# Parallel Histogram Equalization of Gray Scale Images

## 📂 Directory Structure

```
.
├── seq.cpp
├── omp.cpp
├── mpi.cpp
├── combine_all.cpp
├── utils.cpp / utils.hpp
├── makefile
├── Dockerfile
├── devcontainer.json (if used in VSCode Codespaces)
├── output/
│   ├── seq/
│   ├── omp/
│   ├── mpi/
│   └── result_all.png
```

---

## 🐳 Docker Environment

### 🔨 Build Image and Run Docker Container

**Make commands (default):**

```bash
make docker-build-image
make docker-run-container
```

---

## ▶️ Run the Program in Docker

**Make commands (default):**

- Run Sequential:

```bash
make docker-build-seq
make docker-run-seq IMAGE=<image_path>
```

- Run OpenMP:

```bash
make docker-build-omp
make docker-run-omp IMAGE=<image_path> [THREADS=4]
```

- Run MPI:

```bash
make docker-build-mpi
make docker-run-mpi IMAGE=<image_path> THREADS=<num_processes>
```

- Combine All Results:

```bash
make docker-build-combine
make docker-combine
```

\*️⃣ _To suppress console output of histograms, you can add `QUIET=1`. Example:_

```bash
make docker-run-seq IMAGE=<image_path> QUIET=1
```

---

### 🛠 Manual Alternative (Terminal Inside docker)

<details>
<summary>Manual alternative (inside Docker)</summary>

### 🐳 Accessing the Docker Container

To **enter the container:**

```bash
docker exec -it cppcv bash
cd /workspace
```

---

### ✅ Option 1: Use Makefile Commands (Inside Container)

Inside the container, you can **run the same Makefile commands as in the local environment** (drop the `docker-` prefix).

Example:

```bash
make build-seq
make run-seq IMAGE=<image_path>
```

---

### 🛠 Option 2: Run Manual Commands

#### 🔹 Run Sequential

**Build:**

```bash
g++ -O3 -march=native seq.cpp utils.cpp -o seq.out -I/usr/local/include/opencv4 -L/usr/local/lib -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc
```

**Run:**

```bash
LD_LIBRARY_PATH=/usr/local/lib ./seq.out <image_path>
```

\*️⃣ _For quiet mode, add `--quiet`:_

```bash
LD_LIBRARY_PATH=/usr/local/lib ./seq.out --quiet <image_path>
```

---

#### 🔹 Run OpenMP

**Build:**

```bash
g++ -O3 -march=native omp.cpp utils.cpp -o omp.out -fopenmp -I/usr/local/include/opencv4 -L/usr/local/lib -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc
```

**Run:**

```bash
LD_LIBRARY_PATH=/usr/local/lib ./omp.out <image_path>
```

\*️⃣ _For quiet mode, add `--quiet`:_

```bash
LD_LIBRARY_PATH=/usr/local/lib ./omp.out --quiet <image_path>
```

---

#### 🔹 Run MPI

**Build:**

```bash
mpic++ -O3 -march=native mpi.cpp utils.cpp -o mpi.out -I/usr/local/include/opencv4 -L/usr/local/lib -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc
```

**Run:**

```bash
LD_LIBRARY_PATH=/usr/local/lib mpirun --allow-run-as-root -np <num_processes> ./mpi.out <image_path>
```

\*️⃣ _For quiet mode, add `--quiet`:_

```bash
LD_LIBRARY_PATH=/usr/local/lib mpirun --allow-run-as-root -np <num_processes> ./mpi.out --quiet <image_path>
```

---

#### 🔹 Combine All Results

**Build:**

```bash
g++ -std=c++17 -O3 -march=native combine_all.cpp utils.cpp -o combine_all.out -I/usr/local/include/opencv4 -L/usr/local/lib -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc
```

**Run:**

```bash
LD_LIBRARY_PATH=/usr/local/lib ./combine_all.out
```

</details>

---

## 💻 Local Environment

### 🔧 Install C++, OpenCV, and MPI

**Make is default once installed.**

#### Manual installation (Ubuntu example)

```bash
sudo apt update
sudo apt install -y build-essential cmake g++ libopencv-dev openmpi-bin libopenmpi-dev
```

(Or follow [OpenCV build instructions](https://docs.opencv.org/) if compiling OpenCV manually.)

---

### ▶️ Run the Program Locally

#### 🔹 Run Sequential

**Make commands (default):**

```bash
make build-seq
make run-seq IMAGE=<image_path>
```

\*️⃣ _To suppress console output of histograms, add `QUIET=1`. Example:_

```bash
make run-seq IMAGE=<image_path> QUIET=1
```

<details>
<summary>Manual alternative</summary>

**Build:**

```bash
g++ -O3 -march=native seq.cpp utils.cpp -o seq.out -I/usr/include/opencv4 -L/usr/lib -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc
```

**Run:**

```bash
./seq.out <image_path>
```

\*️⃣ _For quiet mode, add `--quiet`:_

```bash
./seq.out --quiet <image_path>
```

</details>

---

#### 🔹 Run OpenMP

**Make commands (default):**

```bash
make build-omp
make run-omp IMAGE=<image_path> [THREADS=4]
```

\*️⃣ _To suppress console output of histograms, add `QUIET=1`:_

```bash
make run-omp IMAGE=<image_path> THREADS=4 QUIET=1
```

<details>
<summary>Manual alternative</summary>

**Build:**

```bash
g++ -O3 -march=native omp.cpp utils.cpp -o omp.out -fopenmp -I/usr/include/opencv4 -L/usr/lib -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc
```

**Run:**

```bash
./omp.out <image_path>
```

\*️⃣ _For quiet mode, add `--quiet`:_

```bash
./omp.out --quiet <image_path>
```

</details>

---

#### 🔹 Run MPI

**Make commands (default):**

```bash
make build-mpi
make run-mpi IMAGE=<image_path> THREADS=<num_processes>
```

\*️⃣ _To suppress console output of histograms, add `QUIET=1`:_

```bash
make run-mpi IMAGE=<image_path> THREADS=4 QUIET=1
```

<details>
<summary>Manual alternative</summary>

**Build:**

```bash
mpic++ mpi.cpp -o mpi.out -I/usr/include/opencv4 -L/usr/lib -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc
```

**Run:**

```bash
mpirun -np <num_processes> ./mpi.out <image_path>
```

\*️⃣ _For quiet mode, add `--quiet`:_

```bash
mpirun -np <num_processes> ./mpi.out --quiet <image_path>
```

</details>

---

#### 🔹 Combine All Results

**Make commands (default):**

```bash
make build-combine
make combine
```

<details>
<summary>Manual alternative</summary>

**Build:**

```bash
g++ -std=c++17 -O3 -march=native combine_all.cpp utils.cpp -o combine_all.out -I/usr/local/include/opencv4 -L/usr/local/lib -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc
```

**Run:**

```bash
./combine_all.out
```

</details>

---

## 🔑 Notes

- **QUIET=1** (Makefile only) suppresses console output of histograms while still saving histogram images.
- **--quiet** (manual command) does the same when added to the program run command.
- **THREADS=4** (Makefile only) sets the number of threads for OpenMP and MPI. Default is 4 if not specified.
- Output images are saved under `output/` with subdirectories for `seq`, `omp`, `mpi`.
- Combined grid saved at: `output/result_all.png`.
