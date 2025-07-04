<p align="center">
  <img src="https://github.com/user-attachments/assets/7a7d349b-7354-49fd-bb03-b2619d20f299" width="512" heigth="120">
</p>


<p align="center">
    A Game of Life implementation developed as a project for the Programmazione Concorrente Parallela e su Cloud (Concurrent, Parallel and Cloud Programming) course, part of the Computer Science Master's Degree program at the University of Salerno
</p>


## Table of Contents
- [Overview](#Overview)
- [Preview](#Preview)
- [Requirements](#Requirements)
- [Project structure](#Project-structure)
- [How to replicate](#How-to-replicate)
- [Built with](#Built-with)


## Overview 
<p>
    This Game of Life implementation was developed to gain practical experience in using the Open MPI library. The game initializes a random matrix that is scattered among multiple processes, which then communicate with each other to update the matrix representing the population.
</p>


## Preview
<p>
  <img src="https://github.com/user-attachments/assets/49523be4-ae10-4051-9cd5-f1df816e3928" width="400" heigth="400">
</p>


## Requirements 
- [Docker](https://www.docker.com)
- [colima](https://github.com/abiosoft/colima) (colima usage in start_mpi_container.sh is required for Apple Silicon chipsets, but can be omitted on other architectures)


## Project structure
```
game-of-life/
├── bin                     # Executable
├── build                   # Intermediate build files
├── include                 # Header files
├── scripts                 # Scripts for mpi container setup
├── src                     # C source files
│   ├── main.c              
│   └── *.c
├── .gitignore
├── ...
└── README.md
```          


## How to replicate
1) Start the docker container for Open MPI 
```bash
scripts/start_mpi_container.sh
```
2) Build the project
```bash
make 
```
3) Run the executable
```bash
mpirun --allow-run-as-root -np <number-of-processes> bin/gameoflife
```


## Built with
- [Open MPI](https://www.open-mpi.org) - used for development using the MPI standard
