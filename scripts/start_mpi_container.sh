#!/bin/bash

# required since the container image is x86_64 (and built-in docker emulation layer doesn't work)
colima delete -f
colima start --cpu 2 --disk 10 --arch x86_64

docker pull spagnuolocarmine/docker-mpi:latest
docker build --no-cache -t spagnuolocarmine/docker-mpi:latest .
docker run --rm -it --mount src="$(pwd)",target=/home,type=bind spagnuolocarmine/docker-mpi:latest