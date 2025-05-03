<!-- @import "[TOC]" {cmd="toc" depthFrom=1 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [Parallel Histogram Equalization of Gray Scale Images](#parallel-histogram-equalization-of-gray-scale-images)
  - [📂 Directory Structure](#-directory-structure)
  - [🐳 Docker Environment](#-docker-environment)
    - [🔨 Build Image and Run Docker Container](#-build-image-and-run-docker-container)
  - [▶️ Run the Program in Docker](#️-run-the-program-in-docker)
    - [🛠 Manual Alternative (Terminal Inside docker)](#-manual-alternative-terminal-inside-docker)
    - [🐳 Accessing the Docker Container](#-accessing-the-docker-container)
    - [✅ Option 1: Use Makefile Commands (Inside Container)](#-option-1-use-makefile-commands-inside-container)
    - [🛠 Option 2: Run Manual Commands](#-option-2-run-manual-commands)
      - [🔹 Run Sequential](#-run-sequential)
      - [🔹 Run OpenMP](#-run-openmp)
      - [🔹 Run MPI](#-run-mpi)
      - [🔹 Combine All Results](#-combine-all-results)
  - [💻 Local Environment](#-local-environment)
    - [🔧 Install C++, OpenCV, and MPI](#-install-c-opencv-and-mpi)
      - [Manual installation (Ubuntu example)](#manual-installation-ubuntu-example)
    - [▶️ Run the Program Locally](#️-run-the-program-locally)
      - [🔹 Run Sequential](#-run-sequential-1)
      - [🔹 Run OpenMP](#-run-openmp-1)
      - [🔹 Run MPI](#-run-mpi-1)
      - [🔹 Combine All Results](#-combine-all-results-1)
  - [🔑 Notes](#-notes)

<!-- /code_chunk_output -->


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
```

Install essential packages:

- Ubuntu 22.04+:

  ```bash
  sudo apt install -y build-essential cmake git pkg-config libgtk-3-dev \
      libavcodec-dev libavformat-dev libswscale-dev libv4l-dev \
      libxvidcore-dev libx264-dev libjpeg-dev libpng-dev libtiff-dev \
      gfortran openexr libatlas-base-dev python3-dev python3-numpy \
      libtbb-dev libdc1394-dev g++ libopencv-dev openmpi-bin libopenmpi-dev
  ```

- Other Ubuntu versions:
  ```bash
  sudo apt install -y build-essential cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev \
    python3-dev python3-numpy libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev libdc1394-22-dev \
    libcanberra-gtk-module libcanberra-gtk3-module openmpi-bin libopenmpi-dev
  ```

Then install OpenCV:

```bash
git clone https://github.com/opencv/opencv.git ~/opencv_build && \
    cd ~/opencv_build && mkdir build && cd build && \
    cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=/usr/local .. && \
    make -j"$(nproc)" && \
    sudo make install && \
    rm -rf ~/opencv_build
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
