#!/bin/bash

colima delete -f
colima start --cpu 2 --disk 10 --arch x86_64

docker build --no-cache -t spagnuolocarmine/docker-mpi:latest .
docker run --rm -it --mount src="$(pwd)",target=/home,type=bind spagnuolocarmine/docker-mpi:latest
